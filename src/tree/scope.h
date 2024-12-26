#ifndef TAROT_SCOPE_H
#define TAROT_SCOPE_H
#ifdef TAROT_SOURCE

#include "defines.h"

/* Forward declaration */
struct tarot_node;

/**
 * The scope stack tracks the nesting of scopes. Scopes aggregate symbols
 * within a logical block in a list for further processing.
 */
struct scope_stack {
	struct tarot_node *root;
	struct tarot_node *class;
	struct tarot_node *function;
	struct tarot_node *loop;
	struct tarot_list **scopes[10];
	bool type_checking;
	uint8_t nesting;
};

/**
 * Creates a new scope.
 */
extern struct tarot_list* create_scope(void);

/**
 * Destroys a scope.
 */
extern void destroy_scope(struct tarot_list *scope);

/**
 * Enters the scope inside of the node by pushing it to the top of
 * the scope stack.
 */
extern void enter_scope(struct scope_stack *stack, struct tarot_list **scope);

/**
 * Leaves the scope on top of the scope stack.
 */
extern void leave_scope(struct scope_stack *stack, struct tarot_list *scope);

/**
 *
 */
extern void enter_node(struct scope_stack *stack, struct tarot_node *node);

/**
 *
 */
extern void leave_node(struct scope_stack *stack, struct tarot_node *node);

/**
 * Looks for the identifier @p name inside scope @p scope
 */
extern struct tarot_node* lookup_symbol_in_scope(
	struct tarot_list *scope,
	struct tarot_string *name
);

/**
 * Looks for the identifier @p name within all scopes of
 * the stack of scopes @p stack
 */
extern struct tarot_node* lookup_symbol(
	struct scope_stack *stack,
	struct tarot_string *name
);

/**
 * Adds the symbol to the given scope.
 */
extern void add_symbol_to_scope(
	struct tarot_list **scope,
	struct tarot_node *node
);

/**
 * Adds the symbol to the current scope of the stack.
 * If a node with the same name already exists, an error is raised.
 */
extern void add_symbol(
	struct scope_stack *stack,
	struct tarot_node *node
);

/**
 * Adds the symbol to the current scope of the stack.
 * Does not raise an error on redefinition.
 */
extern void add_symbol_unsafe(
	struct scope_stack *stack,
	struct tarot_node *node
);

/**
 * Removes the given symbol from the current scope of the stack.
 * If the symbol is not contained, an assertion error is raised.
 */
extern void remove_symbol(
	struct scope_stack *stack,
	struct tarot_node *node
);

#endif /* TAROT_SOURCE */
#endif /* TAROT_SCOPE_H */
