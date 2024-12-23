#ifndef TAROT_TYPE_INTEGER_H
#define TAROT_TYPE_INTEGER_H

#include "defines.h"

/* Forward declarations */
struct tarot_string;
struct tarot_iostream;

typedef void tarot_integer;

extern tarot_integer* tarot_create_integer(void);
extern void tarot_free_integer(tarot_integer *integer);
extern void tarot_initialize_integer(tarot_integer *integer);
extern tarot_integer* tarot_create_integer_from_short(long number);
extern tarot_integer* tarot_create_integer_from_float(double number);
extern tarot_integer* tarot_create_integer_from_rational(void *number);
extern tarot_integer* tarot_create_integer_from_string(
	struct tarot_string *string,
	int base
);
extern void tarot_transfer_integer(tarot_integer *integer);
extern void tarot_release_integer(tarot_integer *integer);
extern tarot_integer* tarot_copy_integer(tarot_integer *integer);
extern void tarot_print_integer(
	struct tarot_iostream *stream,
	tarot_integer *integer
);

extern struct tarot_string* tarot_integer_to_string(
	tarot_integer *integer
);

extern double tarot_integer_to_float(tarot_integer *integer);
extern bool tarot_integer_fits_short(tarot_integer *integer);
extern int32_t tarot_integer_to_short(tarot_integer *integer);
extern tarot_integer* tarot_add_integers(
	tarot_integer *a,
	tarot_integer *b
);
extern tarot_integer* tarot_subtract_integers(
	tarot_integer *a,
	tarot_integer *b
);
extern tarot_integer* tarot_multiply_integers(
	tarot_integer *a,
	tarot_integer *b
);
extern tarot_integer* tarot_divide_integers(
	tarot_integer *a,
	tarot_integer *b
);
extern tarot_integer* tarot_modulo_integers(
	tarot_integer *a,
	tarot_integer *b
);
extern tarot_integer* tarot_exponentiate_integers(
	tarot_integer *a,
	tarot_integer *b
);
extern int tarot_compare_integers(
	tarot_integer *a,
	tarot_integer *b
);

extern tarot_integer* tarot_integer_negate(tarot_integer *integer);
extern size_t tarot_sizeof_integer(tarot_integer *integer);
extern tarot_integer* tarot_import_integer(
	void *buffer,
	size_t *size
);
extern size_t tarot_export_integer(
	void *buffer,
	tarot_integer *integer
);
extern tarot_integer* tarot_integer_abs(tarot_integer *value);

extern tarot_integer* tarot_integer_neg(tarot_integer *value);

#endif /* TAROT_TYPE_INTEGER_H */
