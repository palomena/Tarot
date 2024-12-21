#define TAROT_SOURCE
#include "tarot.h"

/* Calculates 10^n */
static long int ipow10(long int n) {
	long int result = 1;
	if (n > 0) {
		for (result = 10; n > 1; n--) {
			result *= 10;
		}
	}
	return result;
}

long int strtol(const char *str, char **endptr, unsigned int base) {
	long int number = 0;
	long int factor;
	int sign = 1;
	size_t length;

	unused(base);
	unused(endptr);

	assert(str != NULL);
	length = strlen(str);
	assert(length > 0);
	length -= 1;

	/* Check optional sign character */
	if (*str == '+') {
		str++;
		length--;
	} else if (*str == '-') {
		sign = -1;
		str++;
		length--;
	}

	for (
		factor = ipow10(length);
		isdigit(*str);
		factor /= 10, str++
	) {
		number += (*str - '0') * factor;
	}

	return sign * number;
}

double strtod(const char *str, char **endptr) {
	double number = 0.0;
	double sign = 1.0;

	assert(str != NULL);
	unused(endptr);

	/* Check optional sign character */
	if (*str == '+') {
		str++;
	} else if (*str == '-') {
		str++;
		sign = -1.0;
	}

	/* significant part */
	while (isdigit(*str)) {
		number = number * 10.0 + (double)(*str++ - '0');
	}

	if (*str == '.') {
		str++;
		while (isdigit(*str)) {
			sign /= 10.0;
			number = number * 10.0 + (double)(*str++ - '0');
		}
	}

	return sign * number;
}

static int memory_areas_overlap(
	void *destination,
	const void *source,
	size_t num_bytes
) {
	return ((
		(destination > source) and
		((char*)destination < ((char*)source + num_bytes))
	) or (
		((destination < source) and
		((char*)destination + num_bytes) > (char*)source)
	));
}

void* memcpy(void *destination, const void *source, size_t num_bytes) {
	size_t c = num_bytes / sizeof(unsigned int); /* full word operations */
	size_t r = num_bytes % sizeof(unsigned int); /* remaining bytes */
	size_t i; /* iterator */

	return memmove(destination, source, num_bytes);

	assert(destination != NULL);
	assert(source != NULL);
	assert (not memory_areas_overlap(destination, source, num_bytes));

	/* Copy in native processor word batches */
	for (i = 0; i < c; i++) {
		((unsigned int*)destination)[i] = ((unsigned int*)source)[i];
	}

	/* Copy remainder in bytes */
	for (i = 0; i < r; i++) {
		((unsigned char*)destination + c * sizeof(int))[i] = \
			((unsigned char*)source + c * sizeof(int))[i];
	}

	return destination;
}

void* memmove(void *destination, const void *source, size_t num_bytes) {
	size_t i;
	for (i = 0; i < num_bytes; i++) {
		uint8_t *destptr = destination;
		const uint8_t *sourceptr = source;
		destptr[i] = sourceptr[i];
	}
	return destination;
}

/* LMAO @gcc on Makefile release target:
 * The inner function body resembles memset and thus is optimized out to
 * a call to memset. However the function itself is also called memset,
 * thereby leading to a recursive call:
 * void *memset(void *s, int ch, size_t n) {
 *     memset(s, ch, n);
 * }
 * If any statement like printf is inserted in the loop block,
 * the optimization is discarded and the function is used as is.
 */
void* memset(void *s, int ch, size_t n) {
	size_t i; /* iterator */
	assert(s != NULL);
	for (i = 0; i < n; i++) {
		((uint8_t*)s)[i] = ch;
	}
	return s;
}

void* memset2(void *s, int ch, size_t n) {
	size_t c = n / sizeof(unsigned int); /* full word operations */
	size_t r = n % sizeof(unsigned int); /* remaining bytes */
	size_t i; /* iterator */

	assert(s != NULL);

	/* When memsetting something like a string + offset */
	/* we are not necessarily starting out aligned. */
	/* Therefore we need an additional prefix copy in bytes */
	/* before we can continue with full word batches. FIXME/TODO*/

	/* Set in native processor word batches */
	for (i = 0; i < c; i++) {
		((unsigned int*)s)[i] = ch;
	}
	/* Copy remainder in bytes */
	for (i = c * sizeof(unsigned int); i < r; i++) {
		((unsigned char*)s)[i] = ch;
	}

	return s;
}

int memcmp(const void *a, const void *b, size_t n) {
	int result = 0;
	size_t c = n / sizeof(unsigned int); /* possible full word operations */
	size_t r = n % sizeof(unsigned int); /* remaining bytes */
	size_t i; /* iterator */
	const char *dest = a;
	const char *src = b;
	assert(a != NULL);
	assert(b != NULL);

	/* Test if <dest> and <src> overlap. If they do, test only byte-wise */
	if ( ((dest > src) and ((char*)dest < ((char*)src + n))) or
		((dest < src) and ((char*)dest + n) > (char*)src)
	) { r = n; c = 0; }

	/* Test in native processor word batches */
	for (i = 0; i < c; i++) {
		if (((unsigned int*)dest)[i] != ((unsigned int*)src)[i]) {
			result = -1;
			break;
		}
	}

	/* Test remainder in bytes */
	if (result == 0) {
		for (i = 0; i < r; i++) {
			if (
				((unsigned char*)dest + c * sizeof(int))[i] !=
				((unsigned char*)src + c * sizeof(int))[i]
			) {
				result = -1;
				break;
			}
		}
	}

	return result;
}

size_t strlen(const char *str) {
	size_t length = 0;
	assert(str != NULL);
	while (str[length]) length++;
	return length;
}

char* strcpy(char *str1, const char *str2) {
	char *strptr;
	assert(str1 != NULL);
	assert(str2 != NULL);
	strptr = str1;
	while (*str2) {
		*strptr++ = *str2++;
	}
	*strptr = '\0';
	return str1;
}

char* strncpy(char *str1, const char *str2, size_t n) {
	char *strptr;
	assert(str1 != NULL);
	assert(str2 != NULL);
	strptr = str1;
	while (*str2 and (n > 0)) {
		*strptr++ = *str2++;
		n--;
	}
	*strptr = '\0';
	return str1;
}

char* strrev(char *str) {
	size_t a;
	size_t b = 0;
	assert(str != NULL);
	assert(*str != '\0');
	a = strlen(str) - 1;
	while (a > b) {
		char tmp = str[a];
		str[a] = str[b];
		str[b] = tmp;
		a++;
		b++;
	}
	return str;
}

int strcmp(const char *a, const char *b) {
	int result = 0;
	assert(a != NULL);
	assert(b != NULL);
	while (*a and *b) {
		if (*a++ != *b++) {
			result = -1;
			break;
		}
	}
	if (*a != *b) {
		result = -1;
	}
	return result;
}

int strncmp(const char *a, const char *b, size_t n) {
	int result = 0;
	assert(a != NULL);
	assert(b != NULL);
	while (*a and *b and (n > 0)) {
		if (*a++ != *b++) {
			if (n != 0) {
				result = -1;
			}
			break;
		}
		n--;
	}
	if ((n > 0) and (*a != *b)) result = -1;
	return result;
}

int strcasecmp(const char *a, const char *b) {
	int result = 0;
	assert(a != NULL);
	assert(b != NULL);
	while (*a and *b) {
		if (toupper(*a++) != toupper(*b++)) {
			result = -1;
			break;
		}
	}
	return result;
}

char* strdup(const char *str) {
	char *new_string;
	size_t length;
	assert(str != NULL);
	length = strlen(str);
	new_string = tarot_malloc(length + 1);
	memmove(new_string, str, length);
	new_string[length] = '\0';
	return new_string;
}

char* strcat(char *str1, const char *str2) {
	size_t length;
	assert(str1 != NULL);
	assert(str2 != NULL);
	length = strlen(str1);
	strcpy(&str1[length], str2);
	return str1;
}

char* strncat(char *str1, const char *str2, long unsigned int n) {
	size_t length;
	assert(str1 != NULL);
	assert(str2 != NULL);
	length = strlen(str1);
	strncpy(&str1[length], str2, n);
	return str1;
}

char* strchr(const char *str, int ch) {
	char *result = NULL;
	assert(str != NULL);
	while (*str) {
		if (*str == ch) {
			result = (char*)str;
			break;
		}
		str++;
	}
	return result;
}

char* strrchr(const char *str, int ch) {
	char *result = NULL;
	const char *strptr;
	assert(str != NULL);
	for (strptr = str + strlen(str); strptr > str; strptr--) {
		if (*strptr == ch) {
			result = (char*)strptr;
			break;
		}
	}
	return result;
}

char* strstr(const char *str, const char *text) {
	char *result = NULL;
	assert(str != NULL);
	assert(text != NULL);
	while (*str) {
		if (!strcmp(str, text)) {
			result = (char*)str;
			break;
		}
		str++;
	}
	return result;
}

char* strrstr(char *str, const char *text) {
	char *result = NULL;
	char *strptr;
	size_t len_str;
	size_t len_text;
	assert(str != NULL);
	assert(text != NULL);
	len_str = strlen(str);
	len_text = strlen(text);
	assert(len_str >= len_text);
	for (strptr = str + len_str - len_text; strptr > str; strptr--) {
		if (!strcmp(strptr, text)) {
			result = strptr;
			break;
		}
	}
	return result;
}

char* tokenize_string(char *s, const char *delim) {
	register char *spanp;
	register int c, sc;
	char *tok;
	static char *last;

	if (s == NULL and (s = last) == NULL) {
		return NULL;
	}
cont:
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc) goto cont;
	}
	if (c == 0) {
		last = NULL;
		return NULL;
	}
	tok = s - 1;
	for (;;) {
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0) {
					s = NULL;
				} else {
					s[-1] = 0;
				}
				last = s;
				return tok;
			}
		} while (sc != 0);
	}
	assert(0); /* should never be reached */
	return NULL;
}
