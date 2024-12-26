#define TAROT_SOURCE
#include "tarot.h"
#include "bytecode/opcodes.h"

static struct tarot_bytecode* construct_bytecode_interface(struct tarot_bytecode_header *header) {
	struct tarot_bytecode *bytecode = tarot_malloc(sizeof(*bytecode));
	bytecode->header = header;
	bytecode->instructions = tarot_bytecode_instructions(header);
	bytecode->data = tarot_bytecode_data(header);
	bytecode->functions = tarot_bytecode_functions(header);
	bytecode->foreign_functions = tarot_bytecode_foreign_functions(header);
	bytecode->num_functions = header->size.functions / sizeof(*bytecode->functions);
	bytecode->num_foreign_functions = header->size.foreign_functions / sizeof(*bytecode->foreign_functions);
	return bytecode;
}

/******************************************************************************
 * MARK: Sections
 *****************************************************************************/

static size_t sizeof_bytecode(struct tarot_bytecode_header *bytecode) {
	assert(bytecode != NULL);
	return (
		tarot_align(sizeof(*bytecode))
		+ tarot_align(bytecode->size.instructions)
		+ tarot_align(bytecode->size.functions)
		+ tarot_align(bytecode->size.foreign_functions)
		+ tarot_align(bytecode->size.data)
	);
}

uint8_t* tarot_bytecode_instructions(struct tarot_bytecode_header *bytecode) {
	return end_of_struct(bytecode);
}

uint8_t* tarot_bytecode_functions(struct tarot_bytecode_header *bytecode) {
	return tarot_bytecode_instructions(bytecode) + tarot_align(bytecode->size.instructions);
}

uint8_t* tarot_bytecode_foreign_functions(struct tarot_bytecode_header *bytecode) {
	return tarot_bytecode_functions(bytecode) + tarot_align(bytecode->size.functions);
}

uint8_t* tarot_bytecode_data(struct tarot_bytecode_header *bytecode) {
	return tarot_bytecode_foreign_functions(bytecode) + tarot_align(bytecode->size.foreign_functions);
}

static struct tarot_function* function_index(
	struct tarot_bytecode_header *bytecode,
	size_t index
) {
	return (struct tarot_function*)&tarot_bytecode_functions(bytecode)[index * sizeof(struct tarot_function)];
}

static struct tarot_function* foreign_function_index(
	struct tarot_bytecode_header *bytecode,
	size_t index
) {
	return (struct tarot_function*)&tarot_bytecode_foreign_functions(bytecode)[index * sizeof(struct tarot_function)];
}

static const char* read_string(struct tarot_bytecode *bytecode, uint16_t address) {
	return (const char*)&bytecode->data[address];
}

static uint16_t read_argument(uint8_t **ip) {
	return tarot_read16bit(*ip, ip);
}

static bool invalid_bytecode(struct tarot_bytecode_header *bytecode) {
	if (bytecode == NULL) {
		return false;
	}
	if (!strcmp(bytecode->magic, "TAROT")) {
		return false;
	}
	return true;
}

struct tarot_bytecode* tarot_import_bytecode(const char *path) {
	struct tarot_bytecode *bytecode = NULL;
	size_t size;
	struct tarot_bytecode_header *header = tarot_read_file(path, &size);
	if (invalid_bytecode(header)) {
		tarot_error("Invalid Bytecode!");
		tarot_free(header);
	} else {
		bytecode = construct_bytecode_interface(header);
	}
	return bytecode;
}

int tarot_export_bytecode(const char *path, struct tarot_bytecode *bytecode) {
	return tarot_write_to_file(path, bytecode->header, sizeof_bytecode(bytecode->header));
}

/******************************************************************************
 * MARK: Functions
 *****************************************************************************/

void tarot_setup_function(
	struct tarot_function *function,
	uint16_t address,
	uint8_t num_parameters,
	uint8_t num_variables,
	bool returns_value
) {
	function->address = address;
	function->info |= num_parameters << (16-4);
	function->info |= num_variables << (16-4-7);
	function->info |= returns_value << (16-4-7-1);
}

size_t tarot_num_parameters(struct tarot_function *function) {
	static const int mask = 0xF000; /* 0b1111000000000000 */
	return (function->info & mask) >> (16-4);
}

size_t tarot_num_variables(struct tarot_function *function) {
	static const int mask = 0xFE0; /* 0b0000111111100000 */
	return (function->info & mask) >> (16-4-7);
}

bool tarot_returns(struct tarot_function *function) {
	static const int mask = 0x10; /* 0b0000000000010000 */
	return (function->info & mask) >> (16-4-7-1);
}

const char* tarot_get_function_name(
	struct tarot_bytecode *bytecode,
	struct tarot_function *function
) {
	uint8_t *ip = &bytecode->instructions[function->address];
	if (*ip++ == OP_Debug) {
		return read_string(bytecode, read_argument(&ip));
	}
	return NULL;
}

struct tarot_function* tarot_lookup_function(
	struct tarot_bytecode *bytecode,
	const char *function_name
) {
	size_t i;
	for (i = 0; i < bytecode->num_functions; i++) {
		struct tarot_function *function = &bytecode->functions[i];
		if (!strcmp(function_name, tarot_get_function_name(bytecode, function))) {
			return function;
		}
	}
	return NULL;
}

/******************************************************************************
 * MARK: Disassembler
 *****************************************************************************/

static void print_offset(
	struct tarot_iostream *stream,
	uint16_t offset
) {
	tarot_fprintf(stream, "[%*u] ", 5u, offset);
}

static void print_opcode(
	struct tarot_iostream *stream,
	enum tarot_opcode opcode
) {
	tarot_format(stream, TAROT_COLOR_YELLOW);
	tarot_fprintf(stream, "%s ", opcode_string(opcode));
	tarot_format(stream, TAROT_COLOR_RESET);
}

static void print_argument(
	struct tarot_iostream *stream,
	uint16_t argument
) {
	tarot_fprintf(stream, "%d ", argument);
}

static void print_name(
	struct tarot_iostream *stream,
	const char *name
) {
	tarot_format(stream, TAROT_COLOR_GREEN);
	tarot_fputs(stream, name);
	tarot_format(stream, TAROT_COLOR_RESET);
	tarot_fputc(stream, ' ');
}

static void print_float(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode,
	uint16_t index
) {
	double value = tarot_read_float(&bytecode->data[index]);
	tarot_fprintf(stream, "%f", value);
}

static void print_integer(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode,
	uint16_t index
) {
	tarot_integer *value = tarot_import_integer(&bytecode->data[index], NULL);
	tarot_print_integer(stream, value);
	tarot_free_integer(value);
}

static void print_rational(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode,
	uint16_t index
) {
	tarot_rational *value = tarot_import_rational(&bytecode->data[index]);
	tarot_print_rational(stream, value);
	tarot_free_rational(value);
}

static void print_string(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode,
	uint16_t index
) {
	tarot_format(stream, TAROT_COLOR_BLUE);
	tarot_fprintf(stream, "\"%s\"", read_string(bytecode, index));
	tarot_format(stream, TAROT_COLOR_RESET);
}

static void print_function(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode,
	uint16_t index
) {
	struct tarot_function *function = &bytecode->functions[index];
	uint8_t *ip = &bytecode->instructions[function->address];
	if (*ip++ == OP_Debug) {
		print_name(stream, read_string(bytecode, read_argument(&ip)));
	} else {
		print_argument(stream, index);
	}
}

static void print_foreign_function(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode,
	uint16_t index
) {
	struct tarot_function *function = &bytecode->foreign_functions[index];
	print_name(stream, read_string(bytecode, function->address));
}

static void print_type(
	struct tarot_iostream *stream,
	enum tarot_datatype kind
) {
	tarot_format(stream, TAROT_COLOR_GREEN);
	tarot_fputs(stream, datatype_string(kind));
	tarot_format(stream, TAROT_COLOR_RESET);
}

static void disassemble(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode
) {
	uint8_t *ip = bytecode->instructions;
	uint8_t *base = ip;
	tarot_fputs(stream, "[offset] OPCODE *args\n");
	do {
		print_offset(stream, ip - base);
		print_opcode(stream, *ip);

		switch (*ip++) {
		default:
			break;
		case OP_Debug:
			print_name(stream, read_string(bytecode, read_argument(&ip)));
			break;
		case OP_LoadVariablePointer:
			print_argument(stream, tarot_read8bit(ip, &ip));
			break;
		case OP_LoadValue:
		case OP_LoadArgument:
		case OP_Goto:
		case OP_GotoIfFalse:
		case OP_PushList:
		case OP_PushDict:
			print_argument(stream, read_argument(&ip));
			break;
		case OP_CastToFloat:
		case OP_CastToInteger:
		case OP_CastToRational:
		case OP_CastToString:
		case OP_ReturnValue:
		case OP_FreeList:
			print_type(stream, read_argument(&ip));
			break;
		case OP_CallForeignFunction:
			print_foreign_function(stream, bytecode, read_argument(&ip));
			break;
		case OP_CallFunction:
			print_function(stream, bytecode, read_argument(&ip));
			break;
		case OP_PushFloat:
			print_float(stream, bytecode, read_argument(&ip));
			break;
		case OP_PushInteger:
			print_integer(stream, bytecode, read_argument(&ip));
			break;
		case OP_PushRational:
			print_rational(stream, bytecode, read_argument(&ip));
			break;
		case OP_Assert:
		case OP_PushString:
			print_string(stream, bytecode, read_argument(&ip));
			break;
		}

		tarot_newline(stream);
	} while (ip - base < bytecode->header->size.instructions);
}


/******************************************************************************
 * MARK: Printer
 *****************************************************************************/

/**
 * Prints a bold title.
 */
static void print_title(
	struct tarot_iostream *stream,
	const char *title
) {
	tarot_fputs(stream, tarot_color_string(TAROT_COLOR_BOLD));
	tarot_fputs(stream, title);
	tarot_fputs(stream, tarot_color_string(TAROT_COLOR_RESET));
	tarot_fputc(stream, '\n');
}

static void print_section_size(
	struct tarot_iostream *stream,
	const char *section_name,
	size_t size
) {
	tarot_fprintf(
		stream, "  %s %*zu (%*zu) Bytes\n",
		section_name, 5, size, 5, tarot_align(size)
	);
}

static void print_header_section(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode
) {
	print_title(stream, ".header");
	tarot_indent(stream, 1);
	tarot_fprintf(
		stream,
		"magic: %s\n"
		"flags: %x\n"
		"size: %zu Bytes\n",
		bytecode->header->magic,
		bytecode->header->flags,
		sizeof_bytecode(bytecode->header)
	);
	print_section_size(stream,"header:      ", sizeof(*bytecode));
	print_section_size(stream,"instructions:", bytecode->header->size.instructions);
	print_section_size(stream,"functions:   ", bytecode->header->size.functions);
	print_section_size(stream,"foreign funs:", bytecode->header->size.foreign_functions);
	print_section_size(stream,"data:        ", bytecode->header->size.data);
	tarot_indent(stream, -1);
}

static void strdump(
	struct tarot_iostream *stream,
	uint8_t *data,
	size_t size
) {
	size_t i;
	for (i = 0; i < size; i++) {
		char ch = isprint(data[i]) ? data[i] : '.';
		tarot_fputc(stream, ch);
		if (i > 0 and (i % 40 == 0)) {
			tarot_fputc(stream, '\n');
		}
	}
	tarot_fputc(stream, '\n');
}

static void print_data_section(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode
) {
	print_title(stream, ".data (hexadecimal)");
	tarot_indent(stream, 1);
	tarot_hexdump(stream, ',', bytecode->data, bytecode->header->size.data);
	tarot_indent(stream, -1);
	print_title(stream, ".data (ASCII)");
	tarot_indent(stream, 1);
	strdump(stream, bytecode->data, bytecode->header->size.data);
	tarot_indent(stream, -1);
}

/*const char* get_function_name(struct tarot_function *function)*/

static void print_function_section(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode
) {
	uint8_t *instructions = bytecode->instructions;
	size_t i;
	print_title(stream, ".functions");
	tarot_indent(stream, 1);
	for(i = 0; i < bytecode->num_functions; i++) {
		struct tarot_function *function = &bytecode->functions[i];
		tarot_printf("[%zu] ", i);
		if (instructions[function->address] == OP_Debug) {
			uint16_t index = tarot_read16bit(&instructions[function->address+1], NULL);
			const char *function_name = read_string(bytecode, index);
			print_name(stream, function_name);
		}
		tarot_printf(
			"address: %u (parameters: %u, returns: %s, variables: %u)\n",
			function->address,
			tarot_num_parameters(function),
			tarot_bool_string(tarot_returns(function)),
			tarot_num_variables(function)
		);
	}
	tarot_indent(stream, -1);
}

static void print_foreign_function_section(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode
) {
	size_t i;
	print_title(stream, ".foreign_functions");
	tarot_indent(stream, 1);
	for (i = 0; i < bytecode->num_foreign_functions; i++) {
		struct tarot_function *function = &bytecode->foreign_functions[i];
		const char *function_name = read_string(bytecode, function->address);
		tarot_printf(
			"[%zu] %s%s%s (parameters: %d, returns: %s)\n",
			i,
			tarot_color_string(TAROT_COLOR_YELLOW),
			function_name,
			tarot_color_string(TAROT_COLOR_RESET),
			tarot_num_parameters(function),
			tarot_bool_string(tarot_returns(function))
		);
	}
	tarot_indent(stream, -1);
}

static void print_instructions(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode
) {
	print_title(stream, ".instructions");
	tarot_indent(stream, 1);
	disassemble(stream, bytecode);
	tarot_indent(stream, -1);
}

void tarot_print_bytecode(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode
) {
	assert(bytecode != NULL);
	print_header_section(stream, bytecode);
	print_data_section(stream, bytecode);
	print_function_section(stream, bytecode);
	print_foreign_function_section(stream, bytecode);
	print_instructions(stream, bytecode);
}

/******************************************************************************
 * MARK: Generator
 *****************************************************************************/

struct tarot_generator {
	struct tarot_bytecode_header *bytecode;
	struct bytecode_offset {
		uint16_t instructions;
		uint16_t functions;
		uint16_t foreign_functions;
		uint16_t data;
	} offset;
	uint8_t *instructions;
	uint8_t *functions;
	uint8_t *foreign_functions;
	uint8_t *data;
	bool must_copy;
};

static void initialize_generator(struct tarot_generator *generator) {
	memset(generator, 0, sizeof(*generator));
}

static bool read_only(struct tarot_generator *generator) {
	return generator->bytecode == NULL;
}

/**
 * Writes an instruction to the instruction segment.
 */
static void write_instruction(
	struct tarot_generator *generator,
	enum tarot_opcode opcode
) {
	if (not read_only(generator)) {
		generator->instructions[generator->offset.instructions] = opcode;
	}
	generator->offset.instructions++;
}

/**
 * Writes an argument for a preceding instruction to the instruction segment.
 */
static void write_instruction_argument_8bit(
	struct tarot_generator *generator,
	uint8_t value
) {
	if (not read_only(generator)) {
		tarot_write8bit(&generator->instructions[generator->offset.instructions], value);
	}
	generator->offset.instructions += sizeof(value);
}

/**
 * Writes an argument for a preceding instruction to the instruction segment.
 */
static void write_instruction_argument_16bit(
	struct tarot_generator *generator,
	uint16_t value
) {
	if (not read_only(generator)) {
		tarot_write16bit(&generator->instructions[generator->offset.instructions], value);
	}
	generator->offset.instructions += sizeof(value);
}

/**
 * Writes an argument for a preceding instruction to the instruction segment.
 */
static void write_instruction_argument_24bit(
	struct tarot_generator *generator,
	uint32_t value
) {
	if (not read_only(generator)) {
		tarot_write24bit(&generator->instructions[generator->offset.instructions], value);
	}
	generator->offset.instructions += sizeof(value);
}

/**
 * Writes an argument for a previous instruction to the instruction segment.
 */
static void write_argument(
	struct tarot_generator *generator,
	uint16_t value
) {
	if (not read_only(generator)) {
		tarot_write16bit(&generator->instructions[generator->offset.instructions], value);
	}
	generator->offset.instructions += sizeof(value);
}

/**
 * Writes a boolean value to the instruction segment.
 */
static void write_boolean(struct tarot_generator *generator, bool value) {
	write_instruction(generator, value ? OP_PushTrue : OP_PushFalse);
}

/**
 * Writes a floating-point value to the data segment.
 */
static void write_float(struct tarot_generator *generator, double value) {
	write_instruction(generator, OP_PushFloat);
	write_argument(generator, generator->offset.data);
	if (not read_only(generator)) {
		tarot_write_float(&generator->data[generator->offset.data], value);
	}
	generator->offset.data += sizeof(value);
}

/**
 * Writes an integer value to the data segment.
 */
static void write_integer(struct tarot_generator *generator, tarot_integer *value) {
	write_instruction(generator, OP_PushInteger);
	write_argument(generator, generator->offset.data);
	if (not read_only(generator)) {
		tarot_export_integer(&generator->data[generator->offset.data], value);
	}
	generator->offset.data += tarot_sizeof_integer(value);
}

/**
 * Writes a rational value to the data segment.
 */
static void write_rational(struct tarot_generator *generator, tarot_rational *value) {
	write_instruction(generator, OP_PushRational);
	write_argument(generator, generator->offset.data);
	if (not read_only(generator)) {
		tarot_export_rational(&generator->data[generator->offset.data], value);
	}
	generator->offset.data += tarot_sizeof_rational(value);
}

/**
 * Writes a string value to the data segment.
 */
static void write_string(struct tarot_generator *generator, struct tarot_string *value) {
	if (not read_only(generator)) {
		tarot_export_string(&generator->data[generator->offset.data], value);
	}
	generator->offset.data += tarot_string_length(value) + 1;
}

/**
 * Writes a string value to the data segment and pushes the address to
 * the stack via the PushString instruction.
 */
static void push_string(struct tarot_generator *generator, struct tarot_string *value) {
	write_instruction(generator, OP_PushString);
	write_argument(generator, generator->offset.data);
	write_string(generator, value);
}

/**
 * Writes debug info to the data segment.
 */
static void write_debug(struct tarot_generator *generator, struct tarot_string *value) {
	write_instruction(generator, OP_Debug);
	write_argument(generator, generator->offset.data);
	if (not read_only(generator)) {
		tarot_export_string(&generator->data[generator->offset.data], value);
	}
	generator->offset.data += tarot_string_length(value) + 1;
}

/**
 * Registers the given function in the function segment
 */
static void register_function(struct tarot_generator *generator, struct tarot_node *node) {
	struct tarot_function *function;
	if (not read_only(generator)) {
		function = function_index(generator->bytecode, FunctionDefinition(node)->index);
		tarot_setup_function(
			function,
			generator->offset.instructions,
			Block(FunctionDefinition(node)->parameters)->num_elements,
			tarot_list_length(FunctionDefinition(node)->scope)
			- Block(FunctionDefinition(node)->parameters)->num_elements,
			FunctionDefinition(node)->return_value != NULL
		);
	}
	generator->offset.functions += sizeof(*function);
}

/**
 * Registers the given foreign function in the function segment
 */
static void register_foreign_function(struct tarot_generator *generator, struct tarot_node *node) {
	struct tarot_function *function;
	if (not read_only(generator)) {
		function = (struct tarot_function*)&generator->foreign_functions[generator->offset.foreign_functions];
		tarot_setup_function(
			function,
			generator->offset.data,
			Block(ForeignFunction(node)->parameters)->num_elements,
			0,
			ForeignFunction(node)->return_value != NULL
		);
	}
	write_string(generator, name_of(node));
	generator->offset.foreign_functions += sizeof(*function);
}

/**
 * Allocates bytecode based on the offsets determined in
 * an initial read-only generator pass over the abstract syntax tree.
 */
static struct tarot_bytecode_header* allocate_bytecode(struct tarot_generator *generator) {
	struct tarot_bytecode_header *bytecode = NULL;
	size_t total_size = (
		tarot_align(sizeof(struct tarot_bytecode_header))
		+ tarot_align(generator->offset.instructions)
		+ tarot_align(generator->offset.functions)
		+ tarot_align(generator->offset.foreign_functions)
		+ tarot_align(generator->offset.data)
	);
	bytecode = tarot_malloc(total_size);
	strcpy(bytecode->magic, "TAROT");
	bytecode->flags = 0x00;
	bytecode->size.instructions = generator->offset.instructions;
	bytecode->size.functions    = generator->offset.functions;
	bytecode->size.foreign_functions = generator->offset.foreign_functions;
	bytecode->size.data         = generator->offset.data;
	return bytecode;
}

/**
 * Sets the bytecode attribute of the given generator and thereby
 * grants it write privileges to said bytecode.
 */
static void set_bytecode(
	struct tarot_generator *generator,
	struct tarot_bytecode_header *bytecode
) {
	initialize_generator(generator); /* rewind */
	generator->bytecode = bytecode;
	generator->instructions = tarot_bytecode_instructions(bytecode);
	generator->functions = tarot_bytecode_functions(bytecode);
	generator->foreign_functions = tarot_bytecode_foreign_functions(bytecode);
	generator->data = tarot_bytecode_data(bytecode);
}

/* Forward declaration */
static void generate(struct tarot_generator *generator, struct tarot_node *node);

struct tarot_bytecode* tarot_create_bytecode(struct tarot_node *ast) {
	struct tarot_bytecode *bytecode = NULL;
	struct tarot_generator generator;
	if (tarot_validate(ast)) {
		struct tarot_bytecode_header *header = NULL;
		initialize_generator(&generator);
		generate(&generator, ast);
		header = allocate_bytecode(&generator);
		set_bytecode(&generator, header);
		generate(&generator, ast);
		bytecode = construct_bytecode_interface(header);
	}
	return bytecode;
}

void tarot_free_bytecode(struct tarot_bytecode *bytecode) {
	if (bytecode != NULL) {
		tarot_free(bytecode->header);
		tarot_free(bytecode);
	}
}

/******************************************************************************
 * MARK: Generate
 *****************************************************************************/

static void generate_module(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	if (Module(node)->is_root) {
		write_instruction(generator, OP_CallFunction);
		write_argument(generator, 0); /* main function is always at index 0 */ /* UNLESS ITS NOT CONTAINED IN THE SOURCECODE LMAO FIXME! */
		write_instruction(generator, OP_Halt);
	}
	while (node != NULL) {
		generate(generator, Module(node)->block);
		node = Module(node)->next_module;
	}
}

static void generate_block(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	size_t i;
	for (i = 0; i < Block(node)->num_elements; i++) {
		generate(generator, Block(node)->elements[i]);
	}
}

static void generate_logical_expression(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, LogicalExpression(node)->left_operand);
	generate(generator, LogicalExpression(node)->right_operand);
	switch (LogicalExpression(node)->operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case EXPR_AND:
			write_instruction(generator, OP_LogicalAnd);
			break;
		case EXPR_OR:
			write_instruction(generator, OP_LogicalOr);
			break;
		case EXPR_XOR:
			write_instruction(generator, OP_LogicalXor);
			break;
	}
}

static void generate_float_relational_expression(
	struct tarot_generator *generator,
	enum RelationalExpressionOperator operator
) {
	switch (operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case EXPR_LESS:
			write_instruction(generator, OP_FloatLessThan);
			break;
		case EXPR_LESS_EQUAL:
			write_instruction(generator, OP_FloatLessEqual);
			break;
		case EXPR_GREATER:
			write_instruction(generator, OP_FloatGreaterThan);
			break;
		case EXPR_GREATER_EQUAL:
			write_instruction(generator, OP_FloatGreaterEqual);
			break;
		case EXPR_EQUAL:
			write_instruction(generator, OP_FloatEquality);
			break;
		case EXPR_NOT_EQUAL:
			write_instruction(generator, OP_FloatEquality);
			write_instruction(generator, OP_LogicalNot);
			break;
	}
}

static void generate_integer_relational_expression(
	struct tarot_generator *generator,
	enum RelationalExpressionOperator operator
) {
	switch (operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case EXPR_LESS:
			write_instruction(generator, OP_IntegerLessThan);
			break;
		case EXPR_LESS_EQUAL:
			write_instruction(generator, OP_IntegerLessEqual);
			break;
		case EXPR_GREATER:
			write_instruction(generator, OP_IntegerGreaterThan);
			break;
		case EXPR_GREATER_EQUAL:
			write_instruction(generator, OP_IntegerGreaterEqual);
			break;
		case EXPR_EQUAL:
			write_instruction(generator, OP_IntegerEquality);
			break;
		case EXPR_NOT_EQUAL:
			write_instruction(generator, OP_IntegerEquality);
			write_instruction(generator, OP_LogicalNot);
			break;
	}
}

static void generate_rational_relational_expression(
	struct tarot_generator *generator,
	enum RelationalExpressionOperator operator
) {
	switch (operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case EXPR_LESS:
			write_instruction(generator, OP_RationalLessThan);
			break;
		case EXPR_LESS_EQUAL:
			write_instruction(generator, OP_RationalLessEqual);
			break;
		case EXPR_GREATER:
			write_instruction(generator, OP_RationalGreaterThan);
			break;
		case EXPR_GREATER_EQUAL:
			write_instruction(generator, OP_RationalGreaterEqual);
			break;
		case EXPR_EQUAL:
			write_instruction(generator, OP_RationalEquality);
			break;
		case EXPR_NOT_EQUAL:
			write_instruction(generator, OP_RationalEquality);
			write_instruction(generator, OP_LogicalNot);
			break;
	}
}

static void generate_string_relational_expression(
	struct tarot_generator *generator,
	enum RelationalExpressionOperator operator
) {
	switch (operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case EXPR_LESS:
		case EXPR_LESS_EQUAL:
		case EXPR_GREATER:
		case EXPR_GREATER_EQUAL:
			break;
		case EXPR_EQUAL:
			write_instruction(generator, OP_StringEquality);
			break;
		case EXPR_NOT_EQUAL:
			write_instruction(generator, OP_StringEquality);
			write_instruction(generator, OP_LogicalNot);
			break;
		case EXPR_IN:
			write_instruction(generator, OP_StringContains);
			break;
	}
}

static void generate_relational_expression(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, RelationalExpression(node)->left_operand);
	generate(generator, RelationalExpression(node)->right_operand);
	switch (Type(type_of(RelationalExpression(node)->left_operand))->type) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case TYPE_FLOAT:
			generate_float_relational_expression(generator, RelationalExpression(node)->operator);
			break;
		case TYPE_INTEGER:
			generate_integer_relational_expression(generator, RelationalExpression(node)->operator);
			break;
		case TYPE_RATIONAL:
			generate_rational_relational_expression(generator, RelationalExpression(node)->operator);
			break;
		case TYPE_STRING:
			generate_string_relational_expression(generator, RelationalExpression(node)->operator);
			break;
	}
}

static void generate_float_arithmetic_expression(
	struct tarot_generator *generator,
	enum ArithmeticExpressionOperator operator
) {
	switch (operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case EXPR_ADD:
			write_instruction(generator, OP_FloatAddition);
			break;
		case EXPR_SUBTRACT:
			write_instruction(generator, OP_FloatSubtraction);
			break;
		case EXPR_MULTIPLY:
			write_instruction(generator, OP_FloatMultiplication);
			break;
		case EXPR_DIVIDE:
			write_instruction(generator, OP_FloatDivision);
			break;
		case EXPR_MODULO:
			write_instruction(generator, OP_FloatModulo);
			break;
		case EXPR_POWER:
			write_instruction(generator, OP_FloatPower);
			break;
	}
}

static void generate_integer_arithmetic_expression(
	struct tarot_generator *generator,
	enum ArithmeticExpressionOperator operator
) {
	switch (operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case EXPR_ADD:
			write_instruction(generator, OP_IntegerAddition);
			break;
		case EXPR_SUBTRACT:
			write_instruction(generator, OP_IntegerSubtraction);
			break;
		case EXPR_MULTIPLY:
			write_instruction(generator, OP_IntegerMultiplication);
			break;
		case EXPR_DIVIDE:
			write_instruction(generator, OP_IntegerDivision);
			break;
		case EXPR_MODULO:
			write_instruction(generator, OP_IntegerModulo);
			break;
		case EXPR_POWER:
			write_instruction(generator, OP_IntegerPower);
			break;
	}
}

static void generate_rational_arithmetic_expression(
	struct tarot_generator *generator,
	enum ArithmeticExpressionOperator operator
) {
	switch (operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case EXPR_ADD:
			write_instruction(generator, OP_RationalAddition);
			break;
		case EXPR_SUBTRACT:
			write_instruction(generator, OP_RationalSubtraction);
			break;
		case EXPR_MULTIPLY:
			write_instruction(generator, OP_RationalMultiplication);
			break;
		case EXPR_DIVIDE:
			write_instruction(generator, OP_RationalDivision);
			break;
		case EXPR_MODULO:
			write_instruction(generator, OP_RationalModulo);
			break;
		case EXPR_POWER:
			write_instruction(generator, OP_RationalPower);
			break;
	}
}

static void generate_arithmetic_expression(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, ArithmeticExpression(node)->left_operand);
	generate(generator, ArithmeticExpression(node)->right_operand);
	switch (Type(type_of(node))->type) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case TYPE_FLOAT:
			generate_float_arithmetic_expression(generator, ArithmeticExpression(node)->operator);
			break;
		case TYPE_INTEGER:
			generate_integer_arithmetic_expression(generator, ArithmeticExpression(node)->operator);
			break;
		case TYPE_RATIONAL:
			generate_rational_arithmetic_expression(generator, ArithmeticExpression(node)->operator);
			break;
	}
}

static void generate_infix_expression(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, InfixExpression(node)->expression);
}

static void generate_not(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, NotExpression(node)->expression);
	write_instruction(generator, OP_LogicalNot);
}

static void generate_neg(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, NegExpression(node)->expression);
	switch (Type(type_of(NegExpression(node)->expression))->type) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case TYPE_FLOAT:
			write_instruction(generator, OP_FloatNeg);
			break;
		case TYPE_INTEGER:
			write_instruction(generator, OP_IntegerNeg);
			break;
		case TYPE_RATIONAL:
			write_instruction(generator, OP_RationalNeg);
			break;
	}
}

static void generate_abs(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, AbsExpression(node)->expression);
	switch (Type(type_of(AbsExpression(node)->expression))->type) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case TYPE_FLOAT:
			write_instruction(generator, OP_FloatAbs);
			break;
		case TYPE_INTEGER:
			write_instruction(generator, OP_IntegerAbs);
			break;
		case TYPE_RATIONAL:
			write_instruction(generator, OP_RationalAbs);
			break;
	}
}

static void generate_function_call(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	if (kind_of(definition_of(node)) == NODE_Function) {
		generate(generator, FunctionCall(node)->arguments);
		write_instruction(generator, OP_CallFunction);
		write_argument(generator, index_of(definition_of(node)));
	} else if (kind_of(definition_of(node)) == NODE_ForeignFunction) {
		generate(generator, FunctionCall(node)->arguments);
		write_instruction(generator, OP_CallForeignFunction);
		write_argument(generator, index_of(definition_of(node)));
	}
}

static void generate_relation(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	if (kind_of(Relation(node)->link) == NODE_Builtin) {
		struct tarot_node *builtin = Relation(node)->link;
		if (Builtin(builtin)->builtin_type == TYPE_LIST) {
			generate(generator, Relation(node)->parent);
			write_instruction(generator, OP_ListLength);
		} else  if (Builtin(builtin)->builtin_type == TYPE_STRING) {
			generate(generator, Relation(node)->parent);
			write_instruction(generator, OP_StringLength);
		}
	}
}

static void generate_subscript(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, Subscript(node)->identifier);
	generate(generator, Subscript(node)->index);
	switch (Type(type_of(Subscript(node)->identifier))->type) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case TYPE_LIST:
			write_instruction(generator, OP_ListIndex);
			break;
		case TYPE_DICT:
			write_instruction(generator, OP_DictIndex);
			break;
	}
}

static void generate_pair(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, Pair(node)->key);
	generate(generator, Pair(node)->value);
}

static void generate_typecast(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, CastExpression(node)->operand);
	switch (CastExpression(node)->kind) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case TYPE_FLOAT:
			write_instruction(generator, OP_CastToFloat);
			break;
		case TYPE_INTEGER:
			write_instruction(generator, OP_CastToInteger);
			break;
		case TYPE_RATIONAL:
			write_instruction(generator, OP_CastToRational);
			break;
		case TYPE_STRING:
			write_instruction(generator, OP_CastToString);
			break;
	}
	write_argument(generator, Type(type_of(CastExpression(node)->operand))->type);
}

static void generate_literal(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	switch (Literal(node)->kind) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case VALUE_BOOL:
			write_boolean(generator, Literal(node)->value.Boolean);
			break;
		case VALUE_FLOAT:
			write_float(generator, Literal(node)->value.Float);
			break;
		case VALUE_INTEGER:
			write_integer(generator, Literal(node)->value.Integer);
			break;
		case VALUE_RATIONAL:
			write_rational(generator, Literal(node)->value.Rational);
			break;
		case VALUE_RAW_STRING:
		case VALUE_STRING:
			push_string(generator, Literal(node)->value.String);
			break;
	}
}

static void generate_list(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	size_t i;
	bool must_copy = generator->must_copy; /* TODO: Or even better determine this in analyze and write it to Identifier Node struct */
	write_instruction(generator, OP_UnTrack);
	generator->must_copy = true;
	for (i = List(node)->num_elements; i > 0; i--) {
		generate(generator, List(node)->elements[i-1]);
		/* FIXME: If variable, we need a copy! */
	}
	generator->must_copy = must_copy;
	write_instruction(generator, OP_Track);
	write_instruction(generator, OP_PushList);
	write_argument(generator, List(node)->num_elements);
}

static void generate_dict(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	size_t i;
	for (i = 0; i < Dict(node)->num_elements; i++) {
		generate(generator, Dict(node)->elements[i]);
	}
	write_instruction(generator, OP_PushDict);
	write_argument(generator, Dict(node)->num_elements);
}

static void generate_fstring(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	size_t i;
	for (i = 0; i < FString(node)->num_elements; i++) {
		generate(generator, FString(node)->elements[i]);
		if (i > 0) {
			write_instruction(generator, OP_StringConcat);
		}
	}
}

static void generate_fstring_string(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	push_string(generator, FStringString(node)->value);
}

static void generate_fstring_expression(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, FStringExpression(node)->expression);
	switch (Type(type_of(node))->type) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case TYPE_FLOAT:
			write_instruction(generator, OP_CastToString);
			write_argument(generator, TYPE_FLOAT);
			break;
		case TYPE_INTEGER:
			write_instruction(generator, OP_CastToString);
			write_argument(generator, TYPE_INTEGER);
			break;
		case TYPE_RATIONAL:
			write_instruction(generator, OP_CastToString);
			write_argument(generator, TYPE_RATIONAL);
			break;
		case TYPE_STRING:
			break;
	}
}

static void generate_raise(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	write_instruction(generator, OP_RaiseException);
}

static void generate_return(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, ReturnStatement(node)->expression);
	write_instruction(generator, OP_ReturnValue);
	write_argument(generator, Type(type_of(ReturnStatement(node)->expression))->type);
}

static void generate_assert(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	struct tarot_string *source = tarot_stringize_node(AssertStatement(node)->condition);
	struct tarot_string *message = tarot_create_string(
		"In %s:%d: Function %s: Assertion %s failed.",
		position_of(node)->path, position_of(node)->line,
		tarot_string_text(name_of(AssertStatement(node)->function)),
		tarot_string_text(source)
	);
	generate(generator, AssertStatement(node)->condition);
	write_instruction(generator, OP_Assert);
	write_argument(generator, generator->offset.data);
	write_string(generator, message);
	tarot_free(source);
	tarot_free(message);
}

static void generate_function(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	register_function(generator, node);
	write_debug(generator, name_of(node));
	generate(generator, FunctionDefinition(node)->block);
	{
		size_t i;
		struct tarot_list *scope = FunctionDefinition(node)->scope;
		for (i = 0; i < tarot_list_length(scope); i++) {
			struct tarot_node *symbol = *(struct tarot_node**)tarot_list_element(scope, i);
			if (kind_of(symbol) != NODE_Variable) {
				continue;
			}
			switch (Type(type_of(symbol))->type) {
				default:
					break;
				case TYPE_INTEGER:
					write_instruction(generator, OP_LoadVariablePointer);
					write_instruction_argument_8bit(generator, tarot_cast8bit(index_of(symbol)));
					write_instruction(generator, OP_FreeInteger);
					break;
				case TYPE_RATIONAL:
					write_instruction(generator, OP_LoadVariablePointer);
					write_instruction_argument_8bit(generator, index_of(symbol));
					write_instruction(generator, OP_FreeRational);
					break;
				case TYPE_STRING:
					write_instruction(generator, OP_LoadVariablePointer);
					write_instruction_argument_8bit(generator, index_of(symbol));
					write_instruction(generator, OP_FreeString);
					break;
				case TYPE_LIST:
					write_instruction(generator, OP_LoadVariablePointer);
					write_instruction_argument_8bit(generator, index_of(symbol));
					write_instruction(generator, OP_FreeList);
					write_argument(generator, Type(Type(type_of(symbol))->subtype)->type);
					break;
			}
		}
	}
	write_instruction(generator, OP_Return);
}

static void generate_foreign_function(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	register_foreign_function(generator, node);
}

static void generate_namespace(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, Namespace(node)->block);
}

static void generate_copy(struct tarot_generator *generator, struct tarot_node *node) {
	switch (Type(type_of(node))->type) {
		default:
			break;
		case TYPE_INTEGER:
			write_instruction(generator, OP_CopyInteger);
			break;
		case TYPE_RATIONAL:
			write_instruction(generator, OP_CopyRational);
			break;
		case TYPE_STRING:
			write_instruction(generator, OP_CopyString);
			break;
	}
}

static void generate_identifier(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	switch (kind_of(link_of(node))) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case NODE_Variable:
		case NODE_Constant:
			write_instruction(generator, OP_LoadValue);
			write_argument(generator, index_of(link_of(node)));
			if (generator->must_copy) {
				generate_copy(generator, link_of(node));
			}
			break;
		case NODE_Parameter:
			write_instruction(generator, OP_LoadArgument);
			write_argument(generator, index_of(link_of(node)));
			break;
		case NODE_Enumerator:
			tarot_sourcecode_error(__FILE__, __LINE__, "Not Implemented!");
			break;
	}
}

static void generate_enumerator(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	write_instruction(generator, OP_PushFloat); /* FIXME TODO */
	write_argument(generator, index_of(node));
}

static void generate_if(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, IfStatement(node)->condition);
	write_instruction(generator, OP_GotoIfFalse);
	write_argument(generator, IfStatement(node)->middle);
	generate(generator, IfStatement(node)->block);
	if (IfStatement(node)->elseif != NULL) {
		write_instruction(generator, OP_Goto);
		write_argument(generator, IfStatement(node)->end);
	}
	IfStatement(node)->middle = generator->offset.instructions;
	generate(generator, IfStatement(node)->elseif);
	IfStatement(node)->end = generator->offset.instructions;
}

static void generate_while(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	WhileLoop(node)->start = generator->offset.instructions;
	write_instruction(generator, OP_PushRegion);
	generate(generator, WhileLoop(node)->condition);
	write_instruction(generator, OP_GotoIfFalse);
	write_argument(generator, WhileLoop(node)->end);
	write_instruction(generator, OP_PushRegion);
	generate(generator, WhileLoop(node)->block);
	write_instruction(generator, OP_PopRegion);
	write_instruction(generator, OP_PopRegion);
	write_instruction(generator, OP_Goto);
	write_argument(generator, WhileLoop(node)->start);
	WhileLoop(node)->end = generator->offset.instructions;
	write_instruction(generator, OP_PopRegion);
}

static void generate_assignment(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	struct tarot_node *definition = definition_of(Assignment(node)->identifier);
	bool must_copy = false;
	generate(generator, Assignment(node)->value);
	if (kind_of(Assignment(node)->value) == NODE_Identifier) {
		must_copy = true;
	}



	if (kind_of(Assignment(node)->identifier) == NODE_Subscript) {
		struct tarot_node *subscript = Assignment(node)->identifier;
		switch (Type(type_of(definition))->type) {
			default:
				write_instruction(generator, OP_LoadVariablePointer);
				write_instruction_argument_8bit(generator, index_of(node));
				break;
			case TYPE_LIST: /* FIXME: Not quite right when assigning list to list (not index element) */
				generate(generator, Subscript(subscript)->identifier);
				generate(generator, Subscript(subscript)->index);
				write_instruction(generator, OP_LoadListIndex);
				/* list index could be variable!! */
				break;
		}
	} else {
		write_instruction(generator, OP_LoadVariablePointer);
		write_instruction_argument_8bit(generator, index_of(Assignment(node)->identifier));
	}




	switch (Type(type_of(Assignment(node)->value))->type) {
		default:
			write_instruction(generator, OP_StoreValue);
			break;
		case TYPE_INTEGER:
			if (must_copy) {
				write_instruction(generator, OP_CopyInteger);
			}
			write_instruction(generator, OP_StoreInteger);
			break;
		case TYPE_RATIONAL:
			if (must_copy) {
				write_instruction(generator, OP_CopyRational);
			}
			write_instruction(generator, OP_StoreRational);
			break;
		case TYPE_STRING:
			if (must_copy) {
				write_instruction(generator, OP_CopyString);
			}
			write_instruction(generator, OP_StoreString);
			break;
	}
	/*write_argument(generator, index_of(Assignment(node)->identifier));*/
}

static void generate_expression_statement(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, ExprStatement(node)->expression);
}

static void generate_print(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, PrintStatement(node)->arguments);
	switch (Type(type_of(PrintStatement(node)->arguments))->type) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
		case TYPE_BOOLEAN:
			write_instruction(generator, OP_PrintBoolean);
			break;
		case TYPE_FLOAT:
			write_instruction(generator, OP_PrintFloat);
			break;
		case TYPE_INTEGER:
			write_instruction(generator, OP_PrintInteger);
			break;
		case TYPE_RATIONAL:
			write_instruction(generator, OP_PrintRational);
			break;
		case TYPE_STRING:
			write_instruction(generator, OP_PrintString);
			break;
	}
	if (PrintStatement(node)->newline) {
		write_instruction(generator, OP_NewLine);
	}
}

static void generate_input(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, InputExpression(node)->prompt);
	write_instruction(generator, OP_Input);
}

static void generate_variable(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	bool must_copy = false;
	generate(generator, Variable(node)->value);
	if (kind_of(Variable(node)->value) == NODE_Identifier) {
		must_copy = true;
	}

	write_instruction(generator, OP_LoadVariablePointer);
	write_instruction_argument_8bit(generator, index_of(node));

	switch (Type(type_of(Variable(node)->value))->type) {
		default:
			write_instruction(generator, OP_StoreValue);
			break;
		case TYPE_INTEGER:
			if (must_copy) {
				write_instruction(generator, OP_CopyInteger);
			}
			write_instruction(generator, OP_StoreInteger);
			break;
		case TYPE_STRING:
			if (must_copy) {
				write_instruction(generator, OP_CopyString);
			}
			write_instruction(generator, OP_StoreString);
			break;
	}
}

static void generate_constant(
	struct tarot_generator *generator,
	struct tarot_node *node
) {
	generate(generator, Constant(node)->value);
	write_instruction(generator, OP_LoadVariablePointer);
	write_instruction_argument_8bit(generator, index_of(node));
	write_instruction(generator, OP_StoreValue);
}

static void generate(struct tarot_generator *generator, struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			tarot_error_at(position_of(node), "Unhandled node kind: %s", node_string(kind_of(node)));
			break;
		case NODE_NULL:
			break;
		case NODE_Module:
			generate_module(generator, node);
			break;
		case NODE_Block:
			generate_block(generator, node);
			break;
		case NODE_LogicalExpression:
			generate_logical_expression(generator, node);
			break;
		case NODE_RelationalExpression:
			generate_relational_expression(generator, node);
			break;
		case NODE_ArithmeticExpression:
			generate_arithmetic_expression(generator, node);
			break;
		case NODE_InfixExpression:
			generate_infix_expression(generator, node);
			break;
		case NODE_Not:
			generate_not(generator, node);
			break;
		case NODE_Neg:
			generate_neg(generator, node);
			break;
		case NODE_Abs:
			generate_abs(generator, node);
			break;
		case NODE_FunctionCall:
			generate_function_call(generator, node);
			break;
		case NODE_Relation:
			generate_relation(generator, node);
			break;
		case NODE_Subscript:
			generate_subscript(generator, node);
			break;
		case NODE_Pair:
			generate_pair(generator, node);
			break;
		case NODE_Typecast:
			generate_typecast(generator, node);
			break;
		case NODE_Literal:
			generate_literal(generator, node);
			break;
		case NODE_List:
			generate_list(generator, node);
			break;
		case NODE_Dict:
			generate_dict(generator, node);
			break;
		case NODE_FString:
			generate_fstring(generator, node);
			break;
		case NODE_FStringString:
			generate_fstring_string(generator, node);
			break;
		case NODE_FStringExpression:
			generate_fstring_expression(generator, node);
			break;
		case NODE_Identifier:
			generate_identifier(generator, node);
			break;
		case NODE_Enumerator:
			generate_enumerator(generator, node);
			break;
		case NODE_Type:
		case NODE_Import:
			break;
		case NODE_If:
			generate_if(generator, node);
			break;
		case NODE_While:
			generate_while(generator, node);
			break;
		case NODE_Match:
		case NODE_Case:
			break;
		case NODE_Assignment:
			generate_assignment(generator, node);
			break;
		case NODE_ExpressionStatement:
			generate_expression_statement(generator, node);
			break;
		case NODE_Print:
			generate_print(generator, node);
			break;
		case NODE_Input:
			generate_input(generator, node);
			break;
		case NODE_Try:
		case NODE_Catch:
			break;
		case NODE_Raise:
			generate_raise(generator, node);
			break;
		case NODE_Return:
			generate_return(generator, node);
			break;
		case NODE_Assert:
			generate_assert(generator, node);
			break;
		case NODE_Class:
		case NODE_Enum:
			break;
		case NODE_Function:
			generate_function(generator, node);
			break;
		case NODE_ForeignFunction:
			generate_foreign_function(generator, node);
			break;
		case NODE_Namespace:
			generate_namespace(generator, node);
			break;
		case NODE_TypeDefinition:
		case NODE_Union:
			break;
		case NODE_Variable:
			generate_variable(generator, node);
			break;
		case NODE_Constant:
			generate_constant(generator, node);
			break;
		case NODE_Parameter:
			break;
	}
}
