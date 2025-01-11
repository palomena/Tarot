#define TAROT_SOURCE
#include "tarot.h"

/******************************************************************************
 * MARK: Linker
 *****************************************************************************/

static struct tarot_node* block_of(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
		case NODE_ERROR:
			return NULL;
		case NODE_Class:
			return ClassDefinition(node)->block;
		case NODE_Enum:
			return EnumDefinition(node)->block;
		case NODE_Namespace:
			return Namespace(node)->block;
		case NODE_Module:
			return Module(node)->block;
	}
}

static void resolve_builtin_list_relation(struct tarot_node *node) {
	struct tarot_node *owner = Relation(node)->parent;
	struct tarot_string *child = Relation(node)->child;
	if (tarot_match_string(child, "length")) {
		struct tarot_node *link = tarot_create_node(NODE_Builtin, position_of(node));
		Builtin(link)->return_type = tarot_create_node(NODE_Type, position_of(node));
		Type(Builtin(link)->return_type)->type = TYPE_INTEGER;
		Builtin(link)->builtin_type = TYPE_LIST;
		Relation(node)->link = link;
	} else if (tarot_match_string(child, "append")) {
		struct tarot_node *link = tarot_create_node(NODE_Builtin, position_of(node));
		Builtin(link)->return_type = tarot_create_node(NODE_Type, position_of(node));
		Type(Builtin(link)->return_type)->type = TYPE_VOID;
		Builtin(link)->builtin_type = TYPE_LIST;
		Relation(node)->link = link;
	}
}

static void resolve_builtin_string_relation(struct tarot_node *node) {
	struct tarot_node *owner = Relation(node)->parent;
	struct tarot_string *child = Relation(node)->child;
	if (tarot_match_string(child, "length")) {
		struct tarot_node *link = tarot_create_node(NODE_Builtin, position_of(node)); /* FIXME: Not free'd cuz link is not traversed. Solution: new node kind BuiltinRelation */
		Builtin(link)->return_type = tarot_create_node(NODE_Type, position_of(node));
		Builtin(link)->builtin_type = TYPE_STRING;
		Type(Builtin(link)->return_type)->type = TYPE_INTEGER;
		Relation(node)->link = link;
	}
}

static void resolve_builtin_relation(struct tarot_node *node) {
	struct tarot_node *owner = Relation(node)->parent;
	struct tarot_string *child = Relation(node)->child;
	switch (Type(type_of(owner))->type) {
		default:
			break;
		case TYPE_LIST:
			resolve_builtin_list_relation(node);
			break;
		case TYPE_STRING:
			resolve_builtin_string_relation(node);
			break;
		case TYPE_DICT:
			break;
	}
}

static void resolve_relation(struct tarot_node *node) {
	struct tarot_node *owner = definition_of(type_of(Relation(node)->parent));
	struct tarot_node *block = block_of(owner);
	size_t i;
	/************** */
	/* if builtin type such as list or dict */
	if (owner == NULL) {
		resolve_builtin_relation(node);
		return;
	}
	/************* */
	if (block == NULL) { /* if its NODE_ERROR that is */
		return;
	}
	for (i = 0; i < Block(block)->num_elements; i++) {
		if (
			tarot_compare_strings(
				name_of(Block(block)->elements[i]),
				Relation(node)->child
			)
		) {
			Relation(node)->link = Block(block)->elements[i];
			break;
		}
	}
}

static void link_identifier(struct scope_stack *stack, struct tarot_node *node) {
	if (tarot_match_string(name_of(node), "self")) {
		Identifier(node)->link = stack->class;
	} else if (Identifier(node)->link == NULL) { /* if not already linked (see import relation) */
		Identifier(node)->link = lookup_symbol(stack, name_of(node));
	}
}

void link_nodeptr(
	struct tarot_node *node,
	struct scope_stack *stack
) {
	switch (kind_of(node)) {
		default:
			break;
		case NODE_Identifier:
			link_identifier(stack, node);
			break;
		/* May also have to do the following in pass 1 in addition to pass 2 in case of datatypes with relations as identifier*/
		case NODE_Relation:
			resolve_relation(node);
			break;
	}
}

static void set_link(struct tarot_node *node, struct tarot_node *link) {
	switch (kind_of(node)) {
		case NODE_Relation:
			set_link(Relation(node)->parent, link);
			break;
		case NODE_Identifier:
			Identifier(node)->link = link;
			break;
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase!");
			break;
	}
}

void link_import_nodeptr(
	struct tarot_node **nodeptr,
	struct scope_stack *stack
) {
	struct tarot_node *node = *nodeptr;
	unused(stack);
	if (kind_of(node) == NODE_Import) {
		/* At this point import statements are linked to
		 * the imported module, but the symbol is not linked yet.
		 * Linking the symbol is only possible after parsing and after
		 * loading all modules into the AST. It must occur before linking
		 * references to the symbol, which - as import statements are
		 * position sensitive - is ok to do here.
		 */
		set_link(
			ImportStatement(node)->identifier,
			lookup_symbol_in_scope(
				Module(ImportStatement(node)->module_link)->scope,
				name_of(ImportStatement(node)->identifier)
			)
		);
	}
}

void link_datatype_nodeptr(
	struct tarot_node *node,
	struct scope_stack *stack
) {
	if (stack->type_checking) {
		link_nodeptr(node, stack);
	}
}

/******************************************************************************
 * MARK: Type Inference
 *****************************************************************************/

static void infer_types(struct tarot_node *node, struct scope_stack *stack) {
	if (kind_of(node) == NODE_Variable) {
		if (Variable(node)->type == NULL and Variable(node)->value != NULL) { /* TODO: Re-introduce TYPE_CUSTOM to differentiate from VOID */
			Variable(node)->type = tarot_copy_node(type_of(Variable(node)->value));
		}
	} else if (kind_of(node) == NODE_Constant) {
		if (Constant(node)->type == NULL and Constant(node)->value != NULL) { /* TODO: Re-introduce TYPE_CUSTOM to differentiate from VOID */
			Constant(node)->type = tarot_copy_node(type_of(Constant(node)->value));
		}
	}
}

/******************************************************************************
 * MARK: Analyzer
 *****************************************************************************/

struct tarot_analyzer {
	struct tarot_node *root;
	size_t function_index;
	size_t foreign_function_index;
	size_t variable_index;
	bool has_main_function;
	bool panic;
};

/******************************************************************************
 * MARK: Indexing
 *****************************************************************************/

static void index_node(
	struct tarot_node *node,
	struct tarot_analyzer *analyzer
) {
	switch (kind_of(node)) {
		default:
			break;
		case NODE_Function:
			if (tarot_match_string(name_of(node), "main")) {
				FunctionDefinition(node)->index = 0;
				analyzer->has_main_function = true;
			} else {
				FunctionDefinition(node)->index = ++analyzer->function_index;
			}
			analyzer->variable_index = 0;
			break;
		case NODE_ForeignFunction:
			ForeignFunction(node)->index = analyzer->foreign_function_index++;
			break;
		case NODE_Variable:
			Variable(node)->index = analyzer->variable_index++;
			break;
		case NODE_Constant:
			Constant(node)->index = analyzer->variable_index++;
			break;
	}
}

/******************************************************************************
 * MARK: PASS 1
 *
 * | Links import statements to their symbols
 * | Links datatypes to their definitions
 * We must do this initial pass seperate from and before the other pass.
 * Consider the case:
 * - 2 modules
 * - import function from module 2 into module 1
 * - the function returns a custom datatype defined after said function
 *   in module 2
 * - the function has been added to the scope of module 1
 * - when accessing that functions (return) type, it has not been linked yet
 * - error (in validate and in link in case of overloaded functions)
 *****************************************************************************/

static void enter1(struct tarot_node **nodeptr, struct scope_stack *stack, void *data) {
	struct tarot_node *node = *nodeptr;
	struct tarot_analyzer *analyzer = data;
	if (kind_of(node) == NODE_Module) {
		analyzer->root = node;
	}
	link_import_nodeptr(nodeptr, stack);
}

static void leave1(struct tarot_node **nodeptr, struct scope_stack *stack, void *data) {
	unused(data);
	link_datatype_nodeptr(*nodeptr, stack);
}

static void run_pass_1(struct tarot_node *ast) {
	struct tarot_analyzer analyzer;
	memset(&analyzer, 0, sizeof(analyzer));
	tarot_traverse(&ast, enter1, leave1, &analyzer);
}

/******************************************************************************
 * MARK: PASS 2
 *****************************************************************************/

static void enter2(struct tarot_node **nodeptr, struct scope_stack *stack, void *data) {
	struct tarot_node *node = *nodeptr;
	struct tarot_analyzer *analyzer = data;
	unused(stack);
	if (kind_of(node) == NODE_Module) {
		analyzer->root = node;
	}
}

static void leave2(struct tarot_node **nodeptr, struct scope_stack *stack, void *data) {
	struct tarot_node *node = *nodeptr;
	struct tarot_analyzer *analyzer = data;
	if (not analyzer->panic) {
		infer_types(node, stack);
		link_nodeptr(node, stack);
		if (not validate_nodeptr(nodeptr, stack)) {
			Module(analyzer->root)->num_errors++;
			analyzer->panic = true;
		} else {
			simplify_nodeptr(nodeptr);
			index_node(*nodeptr, analyzer);
		}
	} else if (class_of(node) == CLASS_STATEMENT) {
		analyzer->panic = false;
	}
}

static void run_pass_2(struct tarot_node *ast) {
	struct tarot_analyzer analyzer;
	memset(&analyzer, 0, sizeof(analyzer));
	tarot_traverse(&ast, enter2, leave2, &analyzer);
	if (not analyzer.has_main_function) {
		tarot_error("Module does not expose a main function!");
		Module(ast)->num_errors++;
	}
}

void tarot_analyze_ast(struct tarot_node *root) {
	run_pass_1(root);
	run_pass_2(root);
}
