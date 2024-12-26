#include "tarot.h"

void tarot_vmain(const char *path) {
	struct tarot_bytecode *bytecode = NULL;
	assert(tarot_is_initialized());
	bytecode = tarot_import_bytecode(path);
	if (bytecode != NULL) {
		tarot_execute_bytecode(bytecode);
		tarot_free_bytecode(bytecode);
	}
	return 0;
}
