#define TAROT_SOURCE
#include "tarot.h"

TAROT_INLINE
tarot_integer* tarot_create_integer(void) {
	tarot_integer *integer = tarot_malloc(sizeof(mpz_t));
	tarot_initialize_integer(integer);
	return integer;
}

TAROT_INLINE
void tarot_free_integer(tarot_integer *integer) {
	if (integer != NULL) {
		mpz_clear(integer);
	}
	tarot_free(integer);
}

TAROT_INLINE
void tarot_initialize_integer(tarot_integer *integer) {
	mpz_init(integer);
}

TAROT_INLINE
tarot_integer* tarot_create_integer_from_short(long number) {
	tarot_integer *integer = tarot_create_integer();
	mpz_init_set_si(integer, number);
	return integer;
}

TAROT_INLINE
tarot_integer* tarot_create_integer_from_float(double number) {
	tarot_integer *integer = tarot_create_integer();
	mpz_init_set_d(integer, number);
	return integer;
}

TAROT_INLINE
tarot_integer* tarot_create_integer_from_rational(tarot_rational *number) {
	tarot_integer *integer = tarot_create_integer();
	mpz_init_set_si(integer, tarot_rational_to_short(number));
	return integer;
}

TAROT_INLINE
tarot_integer* tarot_create_integer_from_string(
	struct tarot_string *string,
	int base
) {
	tarot_integer *integer = tarot_create_integer();
	assert(string != NULL);
	if (mpz_init_set_str(integer, tarot_string_text(string), base)) {
		tarot_error(
			"Integer conversion error for string \"%s\" with base %d.",
			tarot_string_text(string), base
		);
	}
	return integer;
}

TAROT_INLINE
void tarot_transfer_integer(tarot_integer *integer) {
	mpz_ptr ptr = integer;
	tarot_move_to_parent_region(ptr);
	tarot_move_to_parent_region(ptr->_mp_d);
}

TAROT_INLINE
tarot_integer* tarot_copy_integer(tarot_integer *integer) {
	tarot_integer *result = tarot_create_integer();
	mpz_init_set(result, integer);
	return result;
}

TAROT_INLINE
void tarot_print_integer(
	struct tarot_iostream *stream,
	tarot_integer *integer
) {
	struct tarot_string *string;
	assert(stream != NULL);
	assert(integer != NULL);
	string = tarot_integer_to_string(integer);
	tarot_print_string(stream, string);
	tarot_free(string);
}

TAROT_INLINE
struct tarot_string* tarot_integer_to_string(
	tarot_integer *integer
) {
	struct tarot_string *string = NULL;
	char *strptr;
	assert(integer != NULL);
	strptr = mpz_get_str(NULL, 10, integer);
	string = tarot_create_string(strptr);
	tarot_free(strptr);
	return string;
}

TAROT_INLINE
double tarot_integer_to_float(tarot_integer *integer) {
	assert(integer != NULL);
	return mpz_get_d(integer);
}

TAROT_INLINE
bool tarot_integer_fits_short(tarot_integer *integer) {
#if INT32_MAX == LONG_MAX
	return mpz_fits_slong_p(integer);
#elif INT32_MAX == INT_MAX
	return mpz_fits_sint_p(integer);
#elif INT32_MAX == SHORT_MAX
	return mpz_fits_sshort_p(integer);
#else
#error "Unsupported integer range!"
#endif
}

TAROT_INLINE
int32_t tarot_integer_to_short(tarot_integer *integer) {
	int32_t value = mpz_get_si(integer);
	return value;
}

TAROT_INLINE
tarot_integer* tarot_add_integers(
	tarot_integer *a,
	tarot_integer *b
) {
	tarot_integer *result = tarot_create_integer();
	assert(a != NULL);
	assert(b != NULL);
	mpz_add(result, a, b);
	return result;
}

TAROT_INLINE
tarot_integer* tarot_subtract_integers(
	tarot_integer *a,
	tarot_integer *b
) {
	tarot_integer *result = tarot_create_integer();
	assert(a != NULL);
	assert(b != NULL);
	mpz_sub(result, a, b);
	return result;
}

TAROT_INLINE
tarot_integer* tarot_multiply_integers(
	tarot_integer *a,
	tarot_integer *b
) {
	tarot_integer *result = tarot_create_integer();
	assert(a != NULL);
	assert(b != NULL);
	mpz_mul(result, a, b);
	return result;
}

TAROT_INLINE
tarot_integer* tarot_divide_integers(
	tarot_integer *a,
	tarot_integer *b
) {
	tarot_integer *result = tarot_create_integer();
	assert(a != NULL);
	assert(b != NULL);
	mpz_tdiv_q(result, a, b);
	return result;
}

TAROT_INLINE
tarot_integer* tarot_modulo_integers(
	tarot_integer *a,
	tarot_integer *b
) {
	tarot_integer *result = tarot_create_integer();
	assert(a != NULL);
	assert(b != NULL);
	mpz_mod(result, a, b);
	return result;
}

TAROT_INLINE
tarot_integer* tarot_exponentiate_integers(
	tarot_integer *a,
	tarot_integer *b
) {
	tarot_integer *result = tarot_create_integer();
	assert(a != NULL);
	assert(b != NULL);
	mpz_pow_ui(result, a, tarot_integer_to_short(b));
	return result;
}

TAROT_INLINE
int tarot_compare_integers(
	tarot_integer *a,
	tarot_integer *b
) {
	assert(a != NULL);
	assert(b != NULL);
	return mpz_cmp(a, b);
}

TAROT_INLINE
tarot_integer* tarot_integer_negate(tarot_integer *integer) {
	tarot_integer *result = tarot_create_integer();
	mpz_neg(result, integer);
	return result;
}

TAROT_INLINE
size_t tarot_sizeof_integer(tarot_integer *integer) {
	size_t numb = 8 * sizeof(char) - 0;
	size_t count = (mpz_sizeinbase(integer, 2) + numb - 1) / numb;
	return count + sizeof(uint8_t) + sizeof(uint16_t);
	/* +1 for sign +1 for length in sizeof(long) */
}

TAROT_INLINE
tarot_integer* tarot_import_integer(
	void *buffer,
	size_t *size
) {
	tarot_integer *result = tarot_create_integer();
	int sign;
	size_t length;
	uint8_t *byteptr = buffer;
	sign = tarot_read8bit(byteptr, &byteptr);
	length = tarot_read16bit(byteptr, &byteptr);
	mpz_import(result, length, 1, sizeof(char), 0, 0, byteptr);
	if (sign == 0) {
		struct tarot_integer *n = tarot_integer_negate(result);
		tarot_free_integer(result);
		result = n;
	}
	if (size != NULL) {
		*size = length + sizeof(uint8_t) + sizeof(uint16_t);
	}
	return result;
}

size_t tarot_export_integer(
	void *buffer,
	tarot_integer *integer
) {
	size_t length;
	uint8_t *byteptr = buffer;
	tarot_write8bit(buffer, mpz_sgn(integer) > 0 ? 1 : 0);
	byteptr++;
	mpz_export(
		byteptr+sizeof(uint16_t),
		&length,
		1,
		sizeof(char),
		0,
		0,
		integer
	);
	tarot_write16bit(byteptr, length);
	return length + sizeof(uint8_t) + sizeof(uint16_t);
}

TAROT_INLINE
tarot_integer* tarot_integer_abs(tarot_integer *value) {
	tarot_integer *result = tarot_create_integer();
	mpz_abs(result, value);
	return result;
}

TAROT_INLINE
tarot_integer* tarot_integer_neg(tarot_integer *value) {
	tarot_integer *result = tarot_create_integer();
	mpz_neg(result, value);
	return result;
}
