#ifndef TAROT_THREAD_H
#define TAROT_THREAD_H

#include "defines.h"

/* Forward declaration */
struct tarot_thread;
union tarot_value;

/**
 * Pushes the value to the top of the thread's stack.
 */
extern void tarot_push(struct tarot_thread *thread, union tarot_value value);

/**
 * Pops the top value off the thread's stack.
 */
extern union tarot_value tarot_pop(struct tarot_thread *thread);

/**
 * Returns the top value of the thread's stack, without popping it off.
 */
extern union tarot_value tarot_top(struct tarot_thread *thread);

/**
 * Returns the function argument at the specified index.
 */
extern union tarot_value tarot_argument(struct tarot_thread *thread, uint8_t index);

/**
 * Provides access to the variable at the specfied index.
 */
extern union tarot_value* tarot_variable(struct tarot_thread *thread, uint8_t index);

/**
 *
 */
extern uint16_t tarot_call(struct tarot_thread *thread, struct tarot_function *function);

/**
 *
 */
extern void* tarot_return(struct tarot_thread *thread);

/* Only available to tarot source files */
#ifdef TAROT_SOURCE

struct tarot_stack {
	union tarot_value *base;
	size_t baseptr;
	size_t ptr;
	size_t size;
};

struct stackframe {
	void *return_address;
	struct tarot_function *function;
	size_t baseptr;
	size_t ptr;
};

struct tarot_callstack {
	struct stackframe *frames;
	size_t index;
	size_t size;
};

struct tarot_thread {
	uint8_t *instruction_pointer;
	struct tarot_thread *next_thread;
	struct tarot_thread *previous_thread;
	struct tarot_stack stack;
	struct tarot_callstack callstack;
};

/**
 * Creates a new thread by allocating and initializing (stack) memory.
 */
extern struct tarot_thread* create_thread(uint8_t *instruction_pointer);

/**
 * Frees a stack and it's associated memory.
 */
extern void free_thread(struct tarot_thread *thread);

extern void print_thread(struct tarot_thread *thread);

#endif /* TAROT_SOURCE */

#endif /* TAROT_THREAD_H */
