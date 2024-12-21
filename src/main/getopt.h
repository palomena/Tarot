#ifndef TAROT_GETOPT_H
#define TAROT_GETOPT_H

#include "defines.h"

struct tarot_option {
	char* long_option;
	char short_option;
	bool has_argument;
};

extern char* tarot_optarg;
extern int tarot_optindex;

extern int tarot_getopt(
	int argc,
	char *argv[],
	const struct tarot_option *options,
	unsigned int num_options
);

#endif /* TAROT_GETOPT_H */
