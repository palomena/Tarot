#define TAROT_SOURCE
#include "tarot.h"

static const struct tarot_option command_line_options[] = {
	{"ast",      'a', 0},
	{"bytecode", 'b', 0},
	{"colored",  'c', 0},
	{"debug",    'd', 0},
	{"export",   'e', 0},
	{"format",   'f', 0},
	{"help",     'h', 0},
	{"input",    'i', 1},
	{"verbose",  'l', 0},
	{"output",   'o', 1},
	{"path",     'p', 1},
	{"run",      'r', 0},
	{"scan",     's', 0},
	{"test",     't', 0},
	{"version",  'v', 0}
};

enum option_name {
	OPTION_PRINT_AST,
	OPTION_PRINT_BYTECODE,
	OPTION_ENABLE_COLORED_OUTPUT,
	OPTION_DEBUG_BUILD,
	OPTION_EXPORT,
	OPTION_FORMAT_AST,
	OPTION_PRINT_HELP,
	OPTION_SET_INPUT,
	OPTION_ENABLE_LOGGING,
	OPTION_SET_OUTPUT,
	OPTION_SET_PATH,
	OPTION_RUN_FILE,
	OPTION_SCAN_FILE,
	OPTION_RUN_TEST,
	OPTION_PRINT_VERSION
};

static const unsigned int num_options = lengthof(command_line_options);

static struct {
	struct tarot_node *ast;
	struct tarot_bytecode *bytecode;
	const char *name;
	const char *input;
	const char *output;
	const char *path;
	bool print_help;
	bool print_version;
	bool format_sourcecode;
	bool export_bytecode;
	bool print_tokens;
	bool print_ast;
	bool print_bytecode;
	bool run_file;
	bool run_test;
} program_state;

/* ISO C90 compilers are required to support strings of up to 509 bytes. */
static const char* help_text[] = {
	"Options:\n",
	"  -a, --ast\n"
	"      Prints the Abstract Syntax Tree for the file at input to stdout.\n",
	"  -b, --bytecode\n"
	"      Prints disassembled bytecode for the file at input to stdout.\n",
	"  -c, --colored\n"
	"      Enables colored output. Requires an ECMA-48 compliant terminal.\n",
	"  -d, --debug\n"
	"      Include debug symbols in the bytecode. Will result in increased\n"
	"      bytecode size and slightly worse performance.\n",
	"  -f, --format\n"
	"      Formats the sourcecode.\n",
	"  -h, --help\n"
	"      Displays this help message.\n",
	"  -i, --input\n"
	"      Reads the input file at <path>.\n",
	"  -l, --verbose\n"
	"      Enables verbose output aka logging.\n",
	"  -o, --output\n"
	"      Writes the output to the file at <path>.\n",
	"  -p, --path  <value>\n"
	"      Sets the include path for tarot modules.\n",
	"  -r, --run\n"
	"      Executes the file at input on the Virtual Machine.\n",
	"  -s, --scan\n"
	"      Scans the input file for tokens and prints them to stdout.\n",
	"  -t, --test\n"
	"      Run unittests.\n",
	"  -v, --version\n"
	"      Displays program information and version.\n"
};

static void print_help(void) {
	unsigned int i;
	tarot_printf("Usage: %s [options]...\n", program_state.name);
	for (i = 0; i < lengthof(help_text); i++) {
		tarot_println(help_text[i]);
	}
}

static void print_version(void) {
	static const char version_text[] = {
		"Penta Programming Language, Version "TAROT_VERSION"\n"
		"Built "__DATE__" "__TIME__" in "TAROT_BUILD_MODE" mode.\n"
		"Copyright (c) 2019-2024 Niklas Benfer <https://github.com/palomena>\n"
		"Licensed under the terms of the MIT License."
	};
	tarot_println(version_text);
}

TAROT_INLINE
static int get_option(int argc, char *argv[]) {
	return tarot_getopt(argc, argv, command_line_options, num_options);
}

TAROT_INLINE
static void parse_command_line_arguments(int argc, char *argv[]) {
	int option;
	do switch ((option = get_option(argc, argv))) {
		case -1:
			break;
		case -2:
			tarot_error("Invalid option: \"%s\"", argv[tarot_optindex]);
			break;
		case -3:
			tarot_error(
				"Expected an option: Argument \"%s\" is not an option! "
				"Options are preceded by \"--\" or \"-\" "
				"like \"--help\" or \"-h\".",
				tarot_optarg
			);
			break;
		case OPTION_PRINT_AST:
			program_state.print_ast = true;
			break;
		case OPTION_PRINT_BYTECODE:
			program_state.print_bytecode = true;
			break;
		case OPTION_FORMAT_AST:
			program_state.format_sourcecode = true;
			break;
		case OPTION_ENABLE_COLORED_OUTPUT:
			tarot_enable_colored_output(true);
			break;
		case OPTION_DEBUG_BUILD:
			tarot_enable_debugging(true);
			break;
		case OPTION_EXPORT:
			program_state.export_bytecode = true;
			break;
		case OPTION_PRINT_HELP:
			program_state.name = argv[0];
			program_state.print_help = true;
			break;
		case OPTION_SET_INPUT:
			program_state.input = tarot_optarg;
			break;
		case OPTION_ENABLE_LOGGING:
			tarot_enable_logging(true);
			break;
		case OPTION_SET_OUTPUT:
			program_state.output = tarot_optarg;
			break;
		case OPTION_SET_PATH:
			program_state.path = tarot_optarg;
			break;
		case OPTION_RUN_FILE:
			program_state.run_file = true;
			break;
		case OPTION_RUN_TEST:
			program_state.run_test = true;
			break;
		case OPTION_SCAN_FILE:
			program_state.print_tokens = true;
			break;
		case OPTION_PRINT_VERSION:
			program_state.print_version = true;
			break;
	} while (option >= 0);
}

static void print_runtime_information(void) {
	tarot_newline(tarot_stdout);
	tarot_printf(
		"%sRuntime statistics:%s\n"
		"Encountered %d Errors\n"
		"Encountered %d Warnings\n"
		"Used %d Bytes of memory\n"
		"%zu Allocations | %zu Reallocations | %zu Frees\n"
		"%zu Nodes\n",
		tarot_color_string(TAROT_COLOR_BOLD),
		tarot_color_string(TAROT_COLOR_RESET),
		tarot_num_errors(),
		tarot_num_warnings(),
		tarot_total_memory(),
		tarot_num_allocations(),
		tarot_num_reallocations(),
		tarot_num_frees(),
		tarot_num_nodes()
	);
}

static bool match_filetype(const char *path, const char *type) {
	return path != NULL and !strcmp(strrchr(path, '.'), type);
}

static void run_program(void) {
	if (program_state.print_help) {
		print_help();
		return;
	} else if (program_state.print_version) {
		print_version();
		return;
	} else if (program_state.run_test) {
		tarot_run_tests();
		print_runtime_information();
		return;
	}

	if (match_filetype(program_state.input, ".rot")) {
		program_state.ast = tarot_import(program_state.input);
		program_state.bytecode = tarot_create_bytecode(program_state.ast);
	} else if (match_filetype(program_state.input, ".bin")) {
		program_state.bytecode = tarot_import_bytecode(program_state.input);
	} else {
		tarot_error("Unknown input filetype: %s", program_state.input);
		return;
	}

	if (program_state.print_ast) {
		tarot_print_node(tarot_stdout, program_state.ast);
	}

	if (program_state.format_sourcecode) {
		if (program_state.ast) {
			tarot_serialize_node(tarot_stdout, program_state.ast);
		} else {
			tarot_error("Cannot format abstract syntax tree!");
		}
	}

	if (program_state.print_bytecode) {
		if (program_state.bytecode) {
			tarot_print_bytecode(tarot_stdout, program_state.bytecode);
		} else {
			tarot_error("Cannot print bytecode!");
		}
	}

	if (program_state.run_file) {
		if (program_state.bytecode) {
			tarot_execute_bytecode(program_state.bytecode);
		} else {
			tarot_error("Cannot execute bytecode!");
		}
	}

	tarot_free_node(program_state.ast);
	tarot_free_bytecode(program_state.bytecode);

	print_runtime_information();
}

void tarot_main(int argc, char *argv[]) {
	assert(tarot_is_initialized());
	parse_command_line_arguments(argc, argv);
	run_program();
}
