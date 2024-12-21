#ifndef TAROT_VM_H
#define TAROT_VM_H

#include "defines.h"

/* Forward declaration */
struct tarot_thread;

/**
 *
 */
struct tarot_virtual_machine;

/**
 *
 */
typedef void (*tarot_foreign_function)(struct tarot_thread *thread);

/**
 *
 */
extern void tarot_register_foreign_function(
	struct tarot_virtual_machine *vm,
	const char *name,
	tarot_foreign_function func
);

/**
 *
 */
extern struct tarot_virtual_machine* tarot_create_virtual_machine(struct tarot_bytecode *bytecode);

/**
 *
 */
extern void tarot_free_virtual_machine(struct tarot_virtual_machine *vm);

/**
 *
 */
extern void tarot_attach_executor(struct tarot_virtual_machine *vm);

/**
 * Executes tarot bytecode on a virtual machine.
 */
extern void tarot_execute_bytecode(struct tarot_bytecode *bytecode);

/**
 * The arguments must be passed as instances of union tarot_value!
 */
extern void tarot_call_function(
	struct tarot_virtual_machine *vm,
	const char *function_name, ...
);

#endif /* TAROT_VM_H */
