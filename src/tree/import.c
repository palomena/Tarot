#define TAROT_SOURCE
#include "tarot.h"

/* Looks for an already imported file with the given name */
static struct tarot_node* lookup_module(
	struct tarot_node *modules,
	struct tarot_string *path
) {
	struct tarot_node *result = NULL;
	struct tarot_node *it;
	for (it = modules; it != NULL; it = Module(it)->next_module) {
		if (tarot_match_string(path, Module(it)->path)) {
			result = it;
			break;
		}
	}
	return result;
}

static void append_module(
	struct tarot_node *modules,
	struct tarot_node *module
) {
	while (modules != NULL) {
		if (Module(modules)->next_module == NULL) {
			Module(modules)->next_module = module;
			break;
		}
		modules = Module(modules)->next_module;
	}
}

static void import_module(
	struct tarot_node *modules,
	struct tarot_node *statement
) {
	struct tarot_node *module;
	struct tarot_string *path = ImportStatement(statement)->path;
	if ((module = lookup_module(modules, path))) {
		ImportStatement(statement)->module_link = module;
	} else if ((module = tarot_parse_file(tarot_string_text(path)))) {
		append_module(modules, module);
		ImportStatement(statement)->module_link = module;
	} else {
		tarot_begin_error(position_of(statement));
		tarot_fputs(tarot_stderr, "Failed to import module ");
		tarot_format(tarot_stderr, TAROT_COLOR_CYAN);
		tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
		tarot_print_string(tarot_stderr, ImportStatement(statement)->path);
		tarot_format(tarot_stderr, TAROT_COLOR_RESET);
		tarot_fputc(tarot_stderr, '!');
		tarot_end_error();
	}
}

static void resolve_import(
	struct tarot_node **nodeptr,
	struct scope_stack *stack,
	void *data
) {
	struct tarot_node *node = *nodeptr;
	struct tarot_node *modules = data;
	unused(stack);
	if (kind_of(node) == NODE_Import) {
		import_module(modules, node);
	}
}

void tarot_resolve_imports(struct tarot_node *modules) {
	struct tarot_node *module;
	for (
		module = modules;
		module != NULL;
		module = Module(module)->next_module
	) {
		tarot_traverse_postorder(Module(module)->block, resolve_import, modules);
	}
}

struct tarot_node* tarot_import(const char *path) {
	struct tarot_node *ast = tarot_parse_file(path);
	if (ast != NULL) {
		Module(ast)->is_root = true;
		tarot_resolve_imports(ast);
		if (Module(ast)->num_errors == 0) {
			tarot_analyze_ast(ast);
		}
	}
	return ast;
}
