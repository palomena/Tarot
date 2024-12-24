#define TAROT_SOURCE
#include "tarot.h"

TAROT_INLINE
static void stack_push(struct tarot_stack *stack, union tarot_value value) {
	if (stack->ptr >= stack->size) {
		bool state = tarot_enable_regions(false);
		stack->size = 16 + stack->size * 2;
		stack->base = tarot_realloc(stack->base, sizeof(value) * stack->size);
		tarot_enable_regions(state);
	}
	stack->base[stack->ptr++] = value;
}

TAROT_INLINE
static union tarot_value stack_pop(struct tarot_stack *stack) {
	union tarot_value value = stack->base[--stack->ptr];
	memset(&stack->base[stack->ptr], 0, sizeof(stack->base[stack->ptr]));
	return value;
}

TAROT_INLINE
static union tarot_value stack_top(struct tarot_stack *stack) {
	return stack->base[stack->ptr-1];
}

TAROT_INLINE
static void clear_stack(struct tarot_stack *stack) {
	tarot_free(stack->base);
	memset(stack, 0, sizeof(*stack));
}

TAROT_INLINE
static struct stackframe* current_frame(struct tarot_callstack *callstack) {
	return &callstack->frames[callstack->index-1];
}

TAROT_INLINE
static void push_frame(struct tarot_callstack *callstack) {
	if (callstack->index >= callstack->size) {
		bool state = tarot_enable_regions(false);
		callstack->size = 8 + callstack->size * 2;
		callstack->frames = tarot_realloc(callstack->frames, sizeof(callstack->frames[0]) * callstack->size);
		tarot_enable_regions(state);
	}
	callstack->index++;
}

TAROT_INLINE
static void pop_frame(struct tarot_callstack *callstack) {
	assert(callstack->index > 0);
	callstack->index--;
}

TAROT_INLINE
static void clear_callstack(struct tarot_callstack *stack) {
	tarot_free(stack->frames);
	memset(stack, 0, sizeof(*stack));
}

void tarot_push(struct tarot_thread *thread, union tarot_value value) {
	stack_push(&thread->stack, value);
}

union tarot_value tarot_pop(struct tarot_thread *thread) {
	return stack_pop(&thread->stack);
}

union tarot_value tarot_top(struct tarot_thread *thread) {
	return stack_top(&thread->stack);
}

union tarot_value tarot_argument(struct tarot_thread *thread, uint8_t index) {
	uint8_t num_variables = tarot_num_variables(current_frame(&thread->callstack)->function);
	return thread->stack.base[thread->stack.baseptr - num_variables - index - 1];
}

union tarot_value* tarot_variable(struct tarot_thread *thread, uint8_t index) {
	return &thread->stack.base[thread->stack.baseptr - index - 1];
}

/* frame: [arguments] [baseptr|ptr] [space for variables] */
/* in call: [arguments] [space for variables] [baseptr|ptr] */
/* return: [baseptr|ptr] [arguments] [space for variables] */
uint16_t tarot_call(struct tarot_thread *thread, struct tarot_function *function) {
	push_frame(&thread->callstack);
	current_frame(&thread->callstack)->return_address = thread->instruction_pointer;
	current_frame(&thread->callstack)->function = function;
	current_frame(&thread->callstack)->baseptr = thread->stack.baseptr;
	current_frame(&thread->callstack)->ptr = thread->stack.ptr;
	thread->stack.ptr += tarot_num_variables(function);
	thread->stack.baseptr = thread->stack.ptr;
	return function->address;
}

void* tarot_return(struct tarot_thread *thread) {
	union tarot_value return_value;
	bool returns_value = tarot_returns(current_frame(&thread->callstack)->function);
	void *address = current_frame(&thread->callstack)->return_address;
	if (returns_value) {
		return_value = tarot_pop(thread);
	}
	thread->stack.baseptr = current_frame(&thread->callstack)->baseptr;
	thread->stack.ptr = current_frame(&thread->callstack)->ptr - tarot_num_parameters(current_frame(&thread->callstack)->function); /* discard call args from stack */
	pop_frame(&thread->callstack);
	if (returns_value) {
		tarot_push(thread, return_value);
	}
	return address;
}

struct tarot_thread* create_thread(uint8_t *instruction_pointer) {
	struct tarot_thread *thread = tarot_malloc(sizeof(*thread));
	thread->instruction_pointer = instruction_pointer;
	return thread;
}

void free_thread(struct tarot_thread *thread) {
	clear_stack(&thread->stack);
	clear_callstack(&thread->callstack);
	tarot_free(thread);
}

void print_thread(struct tarot_thread *thread) {
	size_t i;
	tarot_printf("Printout of Thread(%p)\n", thread);
	tarot_printf(".callstack (index: %zu)\n", thread->callstack.index);
	for (i = 0; i < thread->callstack.index; i++) {
		size_t k;
		struct tarot_function *function = thread->callstack.frames[i].function;
		tarot_printf("Function [%zu] (address:%d)\n", i, function->address);
		tarot_println("  .arguments:");
		for (k = 0; k < tarot_num_parameters(function); k++) {
			tarot_printf("\t[%zu] %p\n", k, tarot_argument(thread, k).Pointer);
		}
		tarot_println("  .variables:");
		for (k = 0; k < tarot_num_variables(function); k++) {
			tarot_printf("\t[%zu] %p\n", k, tarot_variable(thread, k));
		}
	}
	tarot_printf("Stack (ptr: %zu, baseptr: %zu)\n", thread->stack.ptr, thread->stack.baseptr);
	if (thread->stack.base != NULL) {
		for (i = 0; i < thread->stack.size; i++) {
			tarot_printf("[%zu] %p", i, thread->stack.base[i].Pointer);
			if (i == thread->stack.baseptr) {
				tarot_print(" < baseptr");
			}
			if (i == thread->stack.ptr) {
				tarot_print(" < ptr");
			}
			tarot_newline(tarot_stdout);
		}
	}
}
