#include <stdio.h>
#include <stdlib.h>
#include "tarot.h"

#define TAROT_VMAIN

int main(int argc, char *argv[]) {
	int exit_code = EXIT_FAILURE;
	const struct tarot_platform_config config = {
		(tarot_abort_function)   abort,
		(tarot_malloc_function)  malloc,
		(tarot_realloc_function) realloc,
		(tarot_free_function)    free,
		(tarot_fopen_function)   fopen,
		(tarot_fclose_function)  fclose,
		(tarot_fgetc_function)   fgetc,
		(tarot_fputc_function)   fputc,
		stdin, stdout, stderr
	};
	tarot_initialize(&config);
	if (tarot_is_initialized()) {
#ifdef TAROT_VMAIN
		tarot_vmain(argv[1]);
#else
		tarot_main(argc, argv);
#endif
		exit_code = tarot_exit();
	}
	return exit_code;
}
