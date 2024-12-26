#define TAROT_SOURCE
#include "tarot.h"
#include "bytecode/opcodes.h"

struct tarot_virtual_machine {
	struct tarot_bytecode *bytecode;
	struct tarot_thread *ready_threads;
	tarot_foreign_function *functions;
	uint16_t num_functions;
	uint16_t num_ready_threads;
};

/**
 * Enqueues a thread into a thread queue.
 */
static void add_ready_thread(
	struct tarot_virtual_machine *vm,
	struct tarot_thread *thread
) {
	assert(vm != NULL);
	assert(thread != NULL);
	if(vm->num_ready_threads == 0) {
		vm->ready_threads = thread;
	} else {
		vm->ready_threads->previous_thread->next_thread = thread;
		thread->next_thread = vm->ready_threads;
		thread->previous_thread = vm->ready_threads->previous_thread;
		vm->ready_threads->previous_thread = thread;
	}
}

/**
 * Removes a thread from a thread queue.
 */
static struct tarot_thread* get_ready_thread(struct tarot_virtual_machine *vm) {
	struct tarot_thread *thread = vm->ready_threads;
	if(vm->num_ready_threads > 1) {
		thread->previous_thread->next_thread = thread->next_thread;
		thread->next_thread->previous_thread = thread->previous_thread;
	} else {
		vm->ready_threads = NULL;
	}
	return thread;
}

/**
 * Spawns a new thread and enqueues it to the ready-queue.
 */
static struct tarot_thread* spawn_thread(
	struct tarot_virtual_machine *vm,
	uint8_t *ip
) {
	struct tarot_thread *thread = create_thread(ip);
	add_ready_thread(vm, thread);
	vm->num_ready_threads++;
	return thread;
}

void tarot_register_foreign_function(
	struct tarot_virtual_machine *vm,
	const char *name,
	tarot_foreign_function func
) {
	unsigned int index;
	assert(name != NULL);
	assert(func != NULL);

	for (index = 0; index < vm->bytecode->num_foreign_functions; index++) {
		struct tarot_function *function = &vm->bytecode->foreign_functions[index];
		const char *function_name = (const char*)&vm->bytecode->data[function->address];
		if (!strcmp(function_name, name)) {
			tarot_debug("bound function '%s' to index %d\n", name, index);
			vm->functions[index] = func;
			break;
		}
	}
}

struct tarot_virtual_machine* tarot_create_virtual_machine(struct tarot_bytecode *bytecode) {
	struct tarot_virtual_machine *vm = tarot_malloc(sizeof(*vm));
	spawn_thread(vm, bytecode->instructions);
	vm->bytecode = bytecode;
	return vm;
}

void tarot_free_virtual_machine(struct tarot_virtual_machine *vm) {
	struct tarot_thread *thread = NULL;
	while ((thread = get_ready_thread(vm))) {
		free_thread(thread);
	}
	tarot_free(vm);
}

void tarot_execute_bytecode(struct tarot_bytecode *bytecode) {
	struct tarot_virtual_machine *vm = tarot_create_virtual_machine(bytecode);
	tarot_attach_executor(vm);
	tarot_free_virtual_machine(vm);
}

void tarot_attach_executor(struct tarot_virtual_machine *vm) {
	struct tarot_thread *thread = get_ready_thread(vm);
	uint8_t *ip = thread->instruction_pointer;

	for (;;) {
		enum tarot_opcode opcode;
		tarot_debug("[%d] %s", ip - vm->bytecode->instructions, opcode_string(*ip));
		opcode = *ip++;

		switch (opcode) {
			union tarot_value a, b, z, *var;
			size_t i, length;
			bool state;

		halt:
		case OP_Halt:
			free_thread(thread);
			return;

		case OP_NoOperation:
			break;

		case OP_Debug:
			tarot_debug(&vm->bytecode->data[tarot_read16bit(ip, &ip)]);
			break;

		case OP_Assert: {
			const char *text = (const char*)&vm->bytecode->data[tarot_read16bit(ip, &ip)];
			if (not tarot_pop(thread).Boolean) {
				tarot_error(text);
				/* TODO: Pop all regions | get region index at start and pop until index */
				goto halt;
			}
			break;
		}

		/*
		 * MARK: Memory OPs
		 */

		case OP_PushRegion:
			tarot_push_region();
			break;

		case OP_PopRegion:
			tarot_pop_region();
			break;

		case OP_StoreValue:
			*var = tarot_pop(thread);
			break;

		/* TODO: */
		/* SIngle store suffices if we make FreeInteger take a local var and dereference everything before a free. Then have FreeType() Store() */
		/* We'd need to overhaul the regions though, either with metadata for regional objects or type-specific instance accounting */
		/* Also what about non-allocated objects? -> how would release work on them -> it wouldn't. So not possible! (a float could have same value as a pointer) */
		/* Could introduce a "transfer ownership" opcode that releases topstack from region */

		case OP_StoreInteger:
			z = tarot_pop(thread);
			tarot_release_integer(z.Integer);
			state = tarot_enable_regions(false);
			tarot_free_integer(var->Integer);
			tarot_enable_regions(state);
			*var = z;
			break;

		case OP_StoreRational:
			z = tarot_pop(thread);
			tarot_release_rational(z.Rational);
			state = tarot_enable_regions(false);
			tarot_free_rational(var->Rational);
			tarot_enable_regions(state);
			*var = z;
			break;

		case OP_StoreString:
			z = tarot_pop(thread);
			tarot_remove_from_region(z.String);
			state = tarot_enable_regions(false);
			tarot_free_string(var->String);
			tarot_enable_regions(state);
			*var = z;
			break;

		case OP_LoadValue:
			tarot_push(thread, *tarot_variable(thread, tarot_read16bit(ip, &ip)));
			break;

		case OP_LoadArgument:
			tarot_push(thread, tarot_argument(thread, tarot_read16bit(ip, &ip)));
			break;

		case OP_LoadVariablePointer:
			var = tarot_variable(thread, tarot_read8bit(ip, &ip));
			break;

		case OP_LoadListIndex:
			a = tarot_pop(thread);
			z = tarot_pop(thread);
			i = tarot_integer_to_short(a.Integer);
			var = (union tarot_value*)tarot_list_element(z.List, i);
			break;

		case OP_Track:
			tarot_enable_regions(true);
			break;

		case OP_UnTrack:
			tarot_enable_regions(false);
			break;

		/*
		 * MARK: Branch OPs
		 */

		case OP_CallFunction:
			thread->instruction_pointer = ip+2;
			ip = &vm->bytecode->instructions[tarot_call(thread, &vm->bytecode->functions[tarot_read16bit(ip, &ip)])];
			tarot_push_region();
			break;

		case OP_Return:
			tarot_pop_region();
			ip = tarot_return(thread);
			break;

		case OP_ReturnValue:
			switch (tarot_read16bit(ip, &ip)) {
				default:
					break;
				case TYPE_INTEGER:
					tarot_activate_relative_region(-1);
					z.Integer = tarot_copy_integer(tarot_pop(thread).Integer);
					tarot_activate_relative_region(+1);
					tarot_push(thread, z);
					break;
				case TYPE_RATIONAL:
					tarot_activate_relative_region(-1);
					z.Integer = tarot_copy_rational(tarot_pop(thread).Rational);
					tarot_activate_relative_region(+1);
					tarot_push(thread, z);
					break;
				case TYPE_STRING:
					tarot_activate_relative_region(-1);
					z.Integer = tarot_copy_string(tarot_pop(thread).String);
					tarot_activate_relative_region(+1);
					tarot_push(thread, z);
					break;
			}
			tarot_pop_region();
			ip = tarot_return(thread);
			break;

		case OP_Goto:
			ip = &vm->bytecode->instructions[tarot_read16bit(ip, &ip)];
			break;

		case OP_GotoIfFalse:
			if (not tarot_pop(thread).Boolean) {
				ip = &vm->bytecode->instructions[tarot_read16bit(ip, &ip)];
			} else {
				tarot_read16bit(ip, &ip);
			}
			break;

		/*
		 * MARK: Logical OPs
		 */

		case OP_PushTrue:
			z.Boolean = true;
			tarot_push(thread, z);
			break;

		case OP_PushFalse:
			z.Boolean = false;
			tarot_push(thread, z);
			break;

		case OP_LogicalAnd:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = a.Boolean and b.Boolean;
			tarot_push(thread, z);
			break;

		case OP_LogicalEquality:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = a.Boolean == b.Boolean;
			tarot_push(thread, z);
			break;

		case OP_LogicalNot:
			z = tarot_pop(thread);
			z.Boolean = not z.Boolean;
			tarot_push(thread, z);
			break;

		case OP_LogicalOr:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = a.Boolean or b.Boolean;
			tarot_push(thread, z);
			break;

		case OP_LogicalXor:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = (a.Boolean or b.Boolean) and not (a.Boolean and b.Boolean);
			tarot_push(thread, z);
			break;

		/*
		 * MARK: Integer OPs
		 */

		case OP_PushInteger:
			z.Integer = tarot_import_integer(&vm->bytecode->data[tarot_read16bit(ip, &ip)], NULL);
			tarot_push(thread, z);
			break;

		case OP_CopyInteger:
			z.Integer = tarot_copy_integer(tarot_pop(thread).Integer);
			tarot_push(thread, z);
			break;

		case OP_FreeInteger:
			state = tarot_enable_regions(false);
			tarot_free_integer(var->Integer);
			tarot_enable_regions(state);
			break;

		case OP_CastToInteger:
			switch (tarot_read16bit(ip, &ip)) {
				default:
					break;
				case TYPE_FLOAT:
					z.Integer = tarot_create_integer_from_float(tarot_pop(thread).Float);
					tarot_push(thread, z);
					break;
				case TYPE_INTEGER:
					break;
				case TYPE_RATIONAL:
					z.Integer = tarot_create_integer_from_rational(tarot_pop(thread).Rational);
					tarot_push(thread, z);
					break;
				case TYPE_STRING:
					z.Integer = tarot_create_integer_from_string(tarot_pop(thread).String, 10);
					tarot_push(thread, z);
					break;
			}
			break;

		case OP_IntegerAbs:
			z.Integer = tarot_integer_abs(tarot_pop(thread).Integer);
			tarot_push(thread, z);
			break;

		case OP_IntegerNeg:
			z.Integer = tarot_integer_neg(tarot_pop(thread).Integer);
			tarot_push(thread, z);
			break;

		case OP_IntegerAddition:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Integer = tarot_add_integers(a.Integer, b.Integer);
			tarot_push(thread, z);
			break;

		case OP_IntegerSubtraction:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Integer = tarot_subtract_integers(a.Integer, b.Integer);
			tarot_push(thread, z);
			break;

		case OP_IntegerMultiplication:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Integer = tarot_multiply_integers(a.Integer, b.Integer);
			tarot_push(thread, z);
			break;

		case OP_IntegerDivision:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Integer = tarot_divide_integers(a.Integer, b.Integer);
			tarot_push(thread, z);
			break;

		case OP_IntegerModulo:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Integer = tarot_modulo_integers(a.Integer, b.Integer);
			tarot_push(thread, z);
			break;

		case OP_IntegerPower:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Integer = tarot_exponentiate_integers(a.Integer, b.Integer);
			tarot_push(thread, z);
			break;

		case OP_IntegerLessThan:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_integers(a.Integer, b.Integer) < 0;
			tarot_push(thread, z);
			break;

		case OP_IntegerLessEqual:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_integers(a.Integer, b.Integer) <= 0;
			tarot_push(thread, z);
			break;

		case OP_IntegerGreaterThan:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_integers(a.Integer, b.Integer) > 0;
			tarot_push(thread, z);
			break;

		case OP_IntegerGreaterEqual:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_integers(a.Integer, b.Integer) >= 0;
			tarot_push(thread, z);
			break;

		case OP_IntegerEquality:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_integers(a.Integer, b.Integer) == 0;
			tarot_push(thread, z);
			break;

		/*
		 * MARK: Float
		 */

		case OP_PushFloat:
			z.Float = tarot_read_float(&vm->bytecode->data[tarot_read16bit(ip, &ip)]);
			tarot_push(thread, z);
			break;

		case OP_CastToFloat:
			switch (tarot_read16bit(ip, &ip)) {
				default:
					break;
				case TYPE_FLOAT:
					break;
				case TYPE_INTEGER:
					z.Float = tarot_integer_to_float(tarot_pop(thread).Integer);
					tarot_push(thread, z);
					break;
				case TYPE_RATIONAL:
					z.Float = tarot_rational_to_float(tarot_pop(thread).Rational);
					tarot_push(thread, z);
					break;
				case TYPE_STRING:
					z.Float = strtod(tarot_string_text(tarot_pop(thread).String), NULL);
					tarot_push(thread, z);
					break;
			}
			break;

		case OP_FloatAbs:
			z.Float = fabs(tarot_pop(thread).Float);
			tarot_push(thread, z);
			break;

		case OP_FloatNeg:
			z.Float = -tarot_pop(thread).Float;
			tarot_push(thread, z);
			break;

		case OP_FloatAddition:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Float = a.Float + b.Float;
			tarot_push(thread, z);
			break;

		case OP_FloatSubtraction:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Float = a.Float - b.Float;
			tarot_push(thread, z);
			break;

		case OP_FloatMultiplication:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Float = a.Float * b.Float;
			tarot_push(thread, z);
			break;

		case OP_FloatDivision:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Float = a.Float / b.Float;
			tarot_push(thread, z);
			break;

		case OP_FloatModulo:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Float = fmod(a.Float, b.Float);
			tarot_push(thread, z);
			break;

		case OP_FloatPower:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Float = pow(a.Float, b.Float);
			tarot_push(thread, z);
			break;

		case OP_FloatLessThan:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = a.Float < b.Float;
			tarot_push(thread, z);
			break;

		case OP_FloatLessEqual:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = a.Float <= b.Float;
			tarot_push(thread, z);
			break;

		case OP_FloatGreaterThan:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = a.Float > b.Float;
			tarot_push(thread, z);
			break;

		case OP_FloatGreaterEqual:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = a.Float >= b.Float;
			tarot_push(thread, z);
			break;

		case OP_FloatEquality:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = a.Float == b.Float;
			tarot_push(thread, z);
			break;

		/*
		 * MARK: Rational
		 */

		case OP_PushRational:
			z.Rational = tarot_import_rational(&vm->bytecode->data[tarot_read16bit(ip, &ip)]);
			tarot_push(thread, z);
			break;

		case OP_CopyRational:
			z.Rational = tarot_copy_rational(tarot_pop(thread).Rational);
			tarot_push(thread, z);
			break;

		case OP_CastToRational:
			switch (tarot_read16bit(ip, &ip)) {
				default:
					break;
				case TYPE_FLOAT:
					z.Rational = tarot_create_rational_from_float(tarot_pop(thread).Float);
					tarot_push(thread, z);
					break;
				case TYPE_INTEGER:
					z.Rational = tarot_create_rational_from_integer(tarot_pop(thread).Integer);
					tarot_push(thread, z);
					break;
				case TYPE_RATIONAL:
					break;
				case TYPE_STRING:
					z.Rational = tarot_create_rational_from_string(tarot_pop(thread).String);
					tarot_push(thread, z);
					break;
			}
			break;

		case OP_RationalAbs:
			z.Rational = tarot_rational_abs(tarot_pop(thread).Rational);
			tarot_push(thread, z);
			break;

		case OP_RationalNeg:
			z.Rational = tarot_rational_neg(tarot_pop(thread).Rational);
			tarot_push(thread, z);
			break;

		case OP_RationalAddition:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Rational = tarot_add_rationals(a.Rational, b.Rational);
			tarot_push(thread, z);
			break;

		case OP_RationalSubtraction:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Rational = tarot_subtract_rationals(a.Rational, b.Rational);
			tarot_push(thread, z);
			break;

		case OP_RationalMultiplication:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Rational = tarot_multiply_rationals(a.Rational, b.Rational);
			tarot_push(thread, z);
			break;

		case OP_RationalDivision:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Rational = tarot_divide_rationals(a.Rational, b.Rational);
			tarot_push(thread, z);
			break;

		case OP_RationalModulo:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			/* TODO! */
			tarot_push(thread, z);
			break;

		case OP_RationalPower:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Rational = tarot_exponentiate_rationals(a.Rational, b.Rational);
			tarot_push(thread, z);
			break;

		case OP_RationalLessThan:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_rationals(a.Rational, b.Rational) < 0;
			tarot_push(thread, z);
			break;

		case OP_RationalLessEqual:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_rationals(a.Rational, b.Rational) <= 0;
			tarot_push(thread, z);
			break;

		case OP_RationalGreaterThan:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_rationals(a.Rational, b.Rational) > 0;
			tarot_push(thread, z);
			break;

		case OP_RationalGreaterEqual:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_rationals(a.Rational, b.Rational) >= 0;
			tarot_push(thread, z);
			break;

		case OP_RationalEquality:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_rationals(a.Rational, b.Rational) == 0;
			tarot_push(thread, z);
			break;

		/*
		 * MARK: String
		 */

		case OP_PushString:
			z.String = tarot_import_string(&vm->bytecode->data[tarot_read16bit(ip, &ip)]);
			tarot_push(thread, z);
			break;

		case OP_CopyString:
			z.String = tarot_copy_string(tarot_pop(thread).String);
			tarot_push(thread, z);
			break;

		case OP_FreeString:
			state = tarot_enable_regions(false);
			tarot_free_string(var->String);
			tarot_enable_regions(state);
			break;

		case OP_CastToString:
			switch (tarot_read16bit(ip, &ip)) {
				default:
					break;
				case TYPE_BOOLEAN:
					z.String = tarot_create_string(tarot_bool_string(tarot_pop(thread).Boolean));
					tarot_push(thread, z);
					break;
				case TYPE_FLOAT:
					z.String = tarot_create_string("%f", tarot_pop(thread).Float);
					tarot_push(thread, z);
					break;
				case TYPE_INTEGER:
					z.String = tarot_integer_to_string(tarot_pop(thread).Integer);
					tarot_push(thread, z);
					break;
				case TYPE_RATIONAL:
					z.String = tarot_rational_to_string(tarot_pop(thread).Rational);
					tarot_push(thread, z);
					break;
				case TYPE_STRING:
					break;
			}
			break;

		case OP_StringEquality:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_compare_strings(a.String, b.String);
			tarot_push(thread, z);
			break;

		case OP_StringContains:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.Boolean = tarot_string_contains(a.String, b.String);
			tarot_push(thread, z);
			break;

		case OP_StringConcat:
			b = tarot_pop(thread);
			a = tarot_pop(thread);
			z.String = tarot_concat_strings(a.String, b.String);
			tarot_push(thread, z);
			break;

		case OP_StringLength:
			z = tarot_pop(thread);
			z.Integer = tarot_create_integer_from_short(tarot_string_length(z.String));
			tarot_push(thread, z);
			break;

		/*
		 * MARK: List & Dict
		 */

		case OP_PushList:
			length = tarot_read16bit(ip, &ip);
			z.List = tarot_create_list(sizeof(z), length, NULL);
			for (i = 0; i < length; i++) {
				a = tarot_pop(thread);
				tarot_list_append(&z.List, &a);
			}
			tarot_push(thread, z);
			break;

		case OP_ListIndex:
			a = tarot_pop(thread);
			z = tarot_pop(thread);
			i = tarot_integer_to_short(a.Integer);
			b = *(union tarot_value*)tarot_list_element(z.List, i);
			tarot_push(thread, b);
			break;

		case OP_FreeList: {
			enum tarot_datatype type = tarot_read16bit(ip, &ip);
			bool state = tarot_enable_regions(false);
			for (i = 0; i < tarot_list_length(var->List); i++) {
				a = *(union tarot_value*)tarot_list_element(var->List, i);
				if (type == TYPE_INTEGER) tarot_free_integer(a.Integer);
				else if (type == TYPE_STRING) tarot_free_string(a.String);
				else if (type == TYPE_RATIONAL) tarot_free_rational(a.Rational);
			}
			/*tarot_free_list(z.List);*/  /* currently still resides within region, would need a StoreList opcode */
			tarot_enable_regions(state);
			break;
		}

		case OP_ListLength:
			z = tarot_pop(thread);
			a.Integer = tarot_create_integer_from_short(tarot_list_length(z.List));
			tarot_push(thread, a);
			break;

		case OP_PushDict:
		case OP_DictIndex:
			break;

		/*
		 * MARK: Print
		 */

		case OP_PrintBoolean:
			tarot_fputs(tarot_stdout, tarot_bool_string(tarot_pop(thread).Boolean));
			break;

		case OP_PrintInteger:
			tarot_print_integer(tarot_stdout, tarot_pop(thread).Integer);
			break;

		case OP_PrintFloat:
			tarot_printf("%f", tarot_pop(thread).Float);
			break;

		case OP_PrintRational:
			tarot_print_rational(tarot_stdout, tarot_pop(thread).Rational);
			break;

		case OP_PrintString:
			tarot_print_string(tarot_stdout, tarot_pop(thread).String);
			break;

		case OP_NewLine:
			tarot_newline(tarot_stdout);
			break;

		case OP_Input: {
			tarot_print_string(tarot_stdout, tarot_pop(thread).String);
			z.String = tarot_input_string(tarot_stdin);
			tarot_push(thread, z);
			break;
		}

		} /* switch */

	} /* for */
}
