#define TAROT_SOURCE
#include "tarot.h"

TAROT_INLINE
static bool is_long_option(const char *argument) {
	assert(argument != NULL);
	return argument[0] == '-' and argument[1] == '-';
}

TAROT_INLINE
static bool is_short_option(const char *argument) {
	assert(argument != NULL);
	return argument[0] == '-' and argument[1] != '-';
}

static int match_long_option(
	const char *argument,
	const struct tarot_option *options,
	unsigned int num_options
) {
	int option = -2;
	unsigned int i;
	for (i = 0; i < num_options; i++) {
		if (!strcmp(options[i].long_option, &argument[2])) {
			option = i;
			break;
		}
	}
	return option;
}

static int match_short_option(
	const char *argument,
	const struct tarot_option *options,
	unsigned int num_options
) {
	int option = -2;
	unsigned int i;
	for (i = 0; i < num_options; i++) {
		if (options[i].short_option == argument[1]) {
			option = i;
			break;
		}
	}
	return option;
}

static int match_option(
	const char *argument,
	const struct tarot_option *options,
	unsigned int num_options
) {
	int option = -3;
	if (is_long_option(argument)) {
		option = match_long_option(argument, options, num_options);
	} else if (is_short_option(argument)) {
		option = match_short_option(argument, options, num_options);
	}
	return option;
}

char* tarot_optarg = NULL;
int tarot_optindex = 1;

int tarot_getopt(
	int argc,
	char *argv[],
	const struct tarot_option *options,
	unsigned int num_options
) {
	static char **argptr = NULL;
	int option = -1;

	assert(argc > 0);
	assert(argv != NULL);
	assert(options != NULL);
	assert(num_options > 0);

	/* Resets argc, if argv changes */
	if (argptr != argv) {
		argptr = argv;
		tarot_optindex = 1;
	}
	tarot_optarg = NULL;
	if (tarot_optindex < argc) {
		option = match_option(argv[tarot_optindex], options, num_options);
		if (option >= 0) {
			tarot_optindex++;
			if (options[option].has_argument) {
				tarot_optarg = argv[tarot_optindex++];
			}
		}
	}
	return option;
}
