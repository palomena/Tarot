#define TAROT_SOURCE
#include "tarot.h"

struct tarot_list* create_scope(void) {
	return tarot_create_list(sizeof(struct tarot_node*), 5, NULL);
}

void destroy_scope(struct tarot_list *scope) {
	tarot_free_list(scope);
}

static struct tarot_list** current_scope(struct scope_stack *stack) {
	assert(stack->nesting > 0); /* "Accessing inactive scope stack? */
	return stack->scopes[stack->nesting-1];
}

static void set_current_scope(
	struct scope_stack *stack,
	struct tarot_list **scope
) {
	assert(stack->nesting > 0); /* "Accessing inactive scope stack? */
	stack->scopes[stack->nesting-1] = scope;
}

void enter_scope(struct scope_stack *stack, struct tarot_list **scope) {
	assert(stack->nesting < lengthof(stack->scopes)); /* Scope stack overflow */
	stack->nesting++;
	set_current_scope(stack, scope);
}

void leave_scope(struct scope_stack *stack, struct tarot_list *scope) {
	assert(stack->nesting > 0);
	assert(*current_scope(stack) == scope);
	set_current_scope(stack, NULL);
	stack->nesting--;
}

void enter_node(struct scope_stack *stack, struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			break;
		case NODE_Module:
			enter_scope(stack, &Module(node)->scope);
			break;
		case NODE_Namespace:
			enter_scope(stack, &Namespace(node)->scope);
			break;
		case NODE_Class:
			enter_scope(stack, &ClassDefinition(node)->scope);
			stack->class = node;
			break;
		case NODE_Function:
			enter_scope(stack, &FunctionDefinition(node)->scope);
			stack->function = node;
			break;
		case NODE_Method:
			enter_scope(stack, &MethodDefinition(node)->scope);
			stack->function = node;
			break;
		case NODE_Import:
		case NODE_Type:
			stack->type_checking++;
			break;
	}
}

void leave_node(struct scope_stack *stack, struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			break;
		case NODE_Module:
			leave_scope(stack, Module(node)->scope);
			break;
		case NODE_Namespace:
			leave_scope(stack, Namespace(node)->scope);
			break;
		case NODE_Class:
			leave_scope(stack, ClassDefinition(node)->scope);
			stack->class = NULL;
			break;
		case NODE_Function:
			leave_scope(stack, FunctionDefinition(node)->scope);
			stack->function = NULL;
			break;
		case NODE_Method:
			leave_scope(stack, MethodDefinition(node)->scope);
			stack->function = NULL;
			break;
		case NODE_Import:
		case NODE_Type:
			stack->type_checking--;
			break;
	}
}

struct tarot_node* lookup_symbol_in_scope(
	struct tarot_list *scope,
	struct tarot_string *name
) {
	struct tarot_node *result = NULL;
	size_t i;
	for (i = 0; i < tarot_list_length(scope); i++) {
		struct tarot_node *symbol = *(struct tarot_node**)tarot_list_element(scope, i);
		if (tarot_compare_strings(name_of(symbol), name)) {
			/* If we were to do definition_of(symbol) here, identifiers to
			 * enumerators would get linked to the enumeration and not the
			 * enumerator. This loses information such as index.
			 */
			if (kind_of(symbol) == NODE_Import and link_of(symbol) != NULL) {
				result = link_of(symbol);
			} else {
				result = symbol;
			}
			break;
		}
	}
	return result;
}

struct tarot_node* lookup_symbol(
	struct scope_stack *stack,
	struct tarot_string *name
) {
	struct tarot_node *result = NULL;
	unsigned int i;
	for (i = 0; i < stack->nesting; i++) {
		result = lookup_symbol_in_scope(*(stack->scopes[i]), name);
		if (result != NULL) {
			break;
		}
	}
	return result;
}

static void raise_redefinition_error(
	struct tarot_node *symbol,
	struct tarot_node *node
) {
	tarot_error_at(
		position_of(node),
		"Redefinition of symbol %s%s%s!\n"
		"First defined in file %s on line %d!",
		tarot_color_string(TAROT_COLOR_BLUE),
		tarot_string_text(name_of(node)),
		tarot_color_string(TAROT_COLOR_RESET),
		position_of(symbol)->path,
		position_of(symbol)->line
	);
}

void add_symbol_to_scope(
	struct tarot_list **scope,
	struct tarot_node *node
) {
	struct tarot_node *symbol = lookup_symbol_in_scope(*scope, name_of(node));
	if (symbol != NULL) {
		raise_redefinition_error(symbol, node);
	} else {
		tarot_list_append(scope, &node);
	}
}

void add_symbol_unsafe(
	struct scope_stack *stack,
	struct tarot_node *node
) {
	tarot_list_append(current_scope(stack), &node);
}

void add_symbol(
	struct scope_stack *stack,
	struct tarot_node *node
) {
	struct tarot_node *symbol = lookup_symbol(stack, name_of(node));
	if (symbol != NULL) {
		raise_redefinition_error(symbol, node);
	} else {
		add_symbol_unsafe(stack, node);
	}
}

void remove_symbol(
	struct scope_stack *stack,
	struct tarot_node *node
) {
	tarot_list_remove(
		current_scope(stack),
		tarot_list_lookup(*current_scope(stack), &node)
	);
}
