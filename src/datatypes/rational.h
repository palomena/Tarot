#ifndef TAROT_TYPE_RATIONAL_H
#define TAROT_TYPE_RATIONAL_H

#include "defines.h"

/* Forward declarations */
struct tarot_iostream;
struct tarot_string;

typedef void tarot_rational;

extern tarot_rational* tarot_create_rational(void);
extern void tarot_free_rational(tarot_rational *rational);
extern void tarot_release_rational(tarot_rational *rational);
extern void tarot_initialize_rational(tarot_rational *rational);
extern tarot_rational* tarot_copy_rational(tarot_rational *value);
extern tarot_rational* tarot_create_rational_from_integers(
	tarot_integer *numerator,
	tarot_integer *denominator
);
extern tarot_rational* tarot_create_rational_from_float(double value);
extern tarot_rational* tarot_create_rational_from_short(int32_t value);
extern tarot_rational* tarot_create_rational_from_integer(tarot_integer *value);
extern tarot_rational* tarot_create_rational_from_string(struct tarot_string *string);
extern void tarot_rational_from_stream(
	tarot_rational *rational,
	struct tarot_iostream *stream
);
extern void tarot_print_rational(
	struct tarot_iostream *stream,
	tarot_rational *rational
);
extern void tarot_print_rational_as_decimal(
	struct tarot_iostream *stream,
	tarot_rational *rational
);

extern struct tarot_string* tarot_rational_to_string(
	tarot_rational *rational
);

extern double tarot_rational_to_float(tarot_rational *rational);
extern int32_t tarot_rational_to_short(tarot_rational *rational);
extern tarot_rational* tarot_add_rationals(
	tarot_rational *a,
	tarot_rational *b
);

extern tarot_rational* tarot_subtract_rationals(
	tarot_rational *a,
	tarot_rational *b
);

extern tarot_rational* tarot_multiply_rationals(
	tarot_rational *a,
	tarot_rational *b
);

extern tarot_rational* tarot_divide_rationals(
	tarot_rational *a,
	tarot_rational *b
);

extern tarot_rational* tarot_exponentiate_rationals(
	tarot_rational *a,
	tarot_rational *b
);

extern int tarot_compare_rationals(
	tarot_rational *a,
	tarot_rational *b
);

extern tarot_rational* tarot_rational_negate(tarot_rational *z);
extern size_t tarot_sizeof_rational(tarot_rational *z);
extern tarot_rational* tarot_import_rational(uint8_t *buffer);

extern void tarot_export_rational(
	void *buffer,
	tarot_rational *z
);
extern tarot_rational* tarot_rational_abs(tarot_rational *a);

extern tarot_rational* tarot_rational_neg(tarot_rational *value);

#endif /* TAROT_TYPE_RATIONAL_H */
