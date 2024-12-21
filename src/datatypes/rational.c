#define TAROT_SOURCE
#include "tarot.h"

TAROT_INLINE
tarot_rational* tarot_create_rational(void) {
	tarot_rational *rational = tarot_malloc(sizeof(mpq_t));
	tarot_initialize_rational(rational);
	return rational;
}

TAROT_INLINE
void tarot_free_rational(tarot_rational *rational) {
	if (rational != NULL) {
		mpq_clear(rational);
	}
	tarot_free(rational);
}

TAROT_INLINE
void tarot_initialize_rational(tarot_rational *rational) {
	mpq_init(rational);
}

TAROT_INLINE
void tarot_transfer_rational(tarot_rational *rational) {
	mpq_ptr ptr = rational;
	tarot_move_to_parent_region(ptr);
	tarot_move_to_parent_region(mpq_denref(ptr)->_mp_d);
	tarot_move_to_parent_region(mpq_numref(ptr)->_mp_d);
}

TAROT_INLINE
tarot_rational* tarot_copy_rational(tarot_rational *rational) {
	tarot_rational *result = tarot_create_rational();
	mpq_set(result, rational);
	return result;
}

TAROT_INLINE
tarot_rational* tarot_create_rational_from_integers(
	tarot_integer *numerator,
	tarot_integer *denominator
) {
	tarot_rational *result = tarot_create_rational();
	mpq_set_num(result, numerator);
	mpq_set_den(result, denominator);
	return result;
}

TAROT_INLINE
tarot_rational* tarot_create_rational_from_float(double value) {
	tarot_rational *result = tarot_create_rational();
	mpq_set_d(result, value);
	return result;
}

TAROT_INLINE
tarot_rational* tarot_create_rational_from_short(int32_t value) {
	tarot_rational *result = tarot_create_rational();
	mpq_set_si(result, value, 1);
	return result;
}

TAROT_INLINE
tarot_rational* tarot_create_rational_from_integer(tarot_integer *value) {
	tarot_rational *result = tarot_create_rational();
	mpq_set_z(result, value);
	return result;
}

TAROT_INLINE
tarot_rational* tarot_create_rational_from_string(struct tarot_string *string) {
	tarot_rational *result = tarot_create_rational();
	mpq_set_str(result, tarot_string_text(string), 10);
	return result;
}

TAROT_INLINE
void tarot_print_rational(
	struct tarot_iostream *stream,
	tarot_rational *rational
) {
	char *str = mpq_get_str(NULL, 10, rational);
	tarot_fputs(stream, str);
	tarot_free(str);
}

TAROT_INLINE
void tarot_print_rational_as_decimal(
	struct tarot_iostream *stream,
	tarot_rational *rational
) {
	tarot_fprintf(stream, "%f", tarot_rational_to_float(rational));
}

TAROT_INLINE
struct tarot_string* tarot_rational_to_string(
	tarot_rational *rational
) {
	struct tarot_string *result;
	char *str = mpq_get_str(NULL, 10, rational);
	result = tarot_create_string(str);
	tarot_free(str);
	return result;
}

TAROT_INLINE
double tarot_rational_to_float(tarot_rational *rational) {
	return mpq_get_d(rational);
}

TAROT_INLINE
int32_t tarot_rational_to_short(tarot_rational *rational) {
	return (int32_t)mpq_get_d(rational);
}

TAROT_INLINE
tarot_rational* tarot_add_rationals(
	tarot_rational *a,
	tarot_rational *b
) {
	tarot_rational *result = tarot_create_rational();
	mpq_add(result, a, b);
	return result;
}

TAROT_INLINE
tarot_rational* tarot_subtract_rationals(
	tarot_rational *a,
	tarot_rational *b
) {
	tarot_rational *result = tarot_create_rational();
	mpq_sub(result, a, b);
	return result;
}

TAROT_INLINE
tarot_rational* tarot_multiply_rationals(
	tarot_rational *a,
	tarot_rational *b
) {
	tarot_rational *result = tarot_create_rational();
	mpq_mul(result, a, b);
	return result;
}

TAROT_INLINE
tarot_rational* tarot_divide_rationals(
	tarot_rational *a,
	tarot_rational *b
) {
	tarot_rational *result = tarot_create_rational();
	mpq_div(result, a, b);
	return result;
}

TAROT_INLINE
tarot_rational* tarot_exponentiate_rationals(
	tarot_rational *a,
	tarot_rational *b
) {
	return tarot_create_rational_from_float(
		pow(
			mpq_get_d(a),
			mpq_get_d(b)
		)
	);
}

TAROT_INLINE
int tarot_compare_rationals(
	tarot_rational *a,
	tarot_rational *b
) {
	assert(a != NULL);
	assert(b != NULL);
	return mpq_cmp(a, b);
}

TAROT_INLINE
tarot_rational* tarot_rational_negate(tarot_rational *z) {
	tarot_rational *result = tarot_create_rational();
	mpq_neg(result, z);
	return result;
}

size_t tarot_sizeof_rational(tarot_rational *z) {
	mpq_ptr ptr = z;
	return (
		tarot_sizeof_integer((void*)mpq_numref(ptr))
		+ tarot_sizeof_integer((void*)mpq_denref(ptr))
	);
}

tarot_rational* tarot_import_rational(uint8_t *buffer) {
	tarot_rational *result;
	size_t n;
	tarot_integer *numerator = tarot_import_integer(buffer, &n);
	tarot_integer *denominator = tarot_import_integer(buffer + n, &n);
	result = tarot_create_rational_from_integers(numerator, denominator);
	tarot_free_integer(numerator);
	tarot_free_integer(denominator);
	return result;
}

void tarot_export_rational(void *buffer, tarot_rational *z) {
	mpq_ptr ptr = z;
	size_t n = tarot_export_integer(buffer, (void*)mpq_numref(ptr));
	tarot_export_integer((uint8_t*)buffer + n, (void*)mpq_denref(ptr));
}

TAROT_INLINE
tarot_rational* tarot_rational_abs(tarot_rational *value) {
	tarot_rational *result = tarot_create_rational();
	mpq_abs(result, value);
	return result;
}

TAROT_INLINE
tarot_rational* tarot_rational_neg(tarot_rational *value) {
	tarot_rational *result = tarot_create_rational();
	mpq_neg(result, value);
	return result;
}
