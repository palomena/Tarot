#ifndef TAROT_BYTECODE_H
#define TAROT_BYTECODE_H

#include "defines.h"

/******************************************************************************
 * MARK: Function
 *****************************************************************************/

/* Forward declaration */
struct tarot_bytecode_header;
struct tarot_iostream;
struct tarot_node;

struct tarot_function {
	uint16_t address;
	uint16_t info;
};

extern void tarot_setup_function(
	struct tarot_function *function,
	uint16_t address,
	uint8_t num_parameters,
	uint8_t num_variables,
	bool returns_value
);

extern size_t tarot_num_parameters(struct tarot_function *function);

extern size_t tarot_num_variables(struct tarot_function *function);

extern bool tarot_returns(struct tarot_function *function);

/******************************************************************************
 * MARK: Bytecode
 *****************************************************************************/

struct tarot_bytecode_header {
	char magic[6];
	uint8_t flags;
	struct {
		uint32_t instructions;
		uint32_t functions;
		uint32_t foreign_functions;
		uint32_t data;
	} size;
};

struct tarot_bytecode {
	struct tarot_bytecode_header *header;
	uint8_t *instructions;
	struct tarot_function *functions;
	struct tarot_function *foreign_functions;
	uint8_t *data;
	uint16_t num_functions;
	uint16_t num_foreign_functions;
	size_t size;
};

/**
 *
 */
extern struct tarot_function* tarot_lookup_function(
	struct tarot_bytecode *bytecode,
	const char *function_name
);

/**
 * Generates bytecode from an abstract syntax tree.
 */
extern struct tarot_bytecode* tarot_create_bytecode(struct tarot_node *ast);

/**
 *
 */
extern void tarot_free_bytecode(struct tarot_bytecode *bytecode);

/**
 * Imports bytecode from the given binary file.
 */
extern struct tarot_bytecode* tarot_import_bytecode(const char *path);

/**
 * Exports bytecode to the given binary file.
 */
extern int tarot_export_bytecode(
	const char *path,
	struct tarot_bytecode *bytecode
);

/**
 * Prints disassembled bytecode to the iostream.
 */
extern void tarot_print_bytecode(
	struct tarot_iostream *stream,
	struct tarot_bytecode *bytecode
);

extern uint8_t* tarot_bytecode_instructions(struct tarot_bytecode_header *bytecode);
extern uint8_t* tarot_bytecode_functions(struct tarot_bytecode_header *bytecode);
extern uint8_t* tarot_bytecode_foreign_functions(struct tarot_bytecode_header *bytecode);
extern uint8_t* tarot_bytecode_data(struct tarot_bytecode_header *bytecode);

#ifdef TAROT_SOURCE
extern const char* read_string(struct tarot_bytecode *bytecode, uint16_t offset);
#endif /* TAROT_SOURCE */

#endif /* TAROT_BYTECODE_H */
