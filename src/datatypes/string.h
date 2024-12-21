#ifndef TAROT_TYPE_STRING_H
#define TAROT_TYPE_STRING_H

#include "defines.h"

/* Forward declarations: */
struct tarot_iostream;
struct tarot_string;

/**
 * Creates a new string via the specified printf format string
 * and accompanying variadic arguments. The format string may
 * be NULL. A valid string as return value is guaranteed.
 */
extern struct tarot_string* tarot_create_string(const char *format, ...);

/**
 * Creates a new string via the specified printf format string
 * and accompanying variadic arguments. The format string may
 * be NULL. A valid string as return value is guaranteed.
 */
extern struct tarot_string* tarot_const_string(const char *text);

/**
 * Creates a copy of the given string by allocating a new string
 * and copying the contents of the given string into the new one.
 */
extern struct tarot_string* tarot_copy_string(struct tarot_string *string);

/**
 * Clears the string. Resets length to 0, and erases the text.
 */
extern void tarot_clear_string(struct tarot_string *string);

/**
 * Clears the contents of and frees the allocated string object.
 * The given string may be NULL in which case nothing is done.
 */
extern void tarot_free_string(struct tarot_string *string);

/**
 * Returns a C-string representation of the string.
 */
extern char* tarot_string_to_cstring(struct tarot_string *string);

/**
 * Returns the length of the string in bytes. For the number
 * of characters in the string use tarot_string_count(string).
 */
extern size_t tarot_string_length(struct tarot_string *string);

/**
 * Returns the raw text buffer of the string. It is advised
 * to not modify the returned value, though it is absolutely
 * possible to do so.
 */
extern char* tarot_string_text(struct tarot_string *string);

/**
 * Appends data to the given string. The data is given as a printf
 * format string. The given string pointer is modified if reallocation
 * is required.
 */
extern void tarot_string_append(struct tarot_string **string, const char *format, ...);

/**
 * Inserts data into the given string. The data is given as a printf
 * format string. The given string pointer is modified if reallocation
 * is required. The index at which data is to be inserted may not
 * exceed the strings character count!
 */
extern void tarot_string_insert(
	struct tarot_string **string,
	size_t index,
	const char *format, ...
);

/**
 * Removes n characters of the string starting at index until index+n.
 * Both index and index+n must be within 0 and the strings character count.
 */
extern void tarot_string_remove(
	struct tarot_string **string,
	size_t index,
	size_t n
);

/**
 * Looks up substring within string. If found returns true and sets index.
 */
extern bool tarot_string_find(
	struct tarot_string *string,
	const char *substring,
	size_t *index
);

/**
 * Replaces all occurances of the given substring within string
 * with a new substring constructed from the printf format string.
 */
extern void tarot_string_replace(
	struct tarot_string **string,
	struct tarot_string *old,
	const char *format, ...
);

/**
 * Concatenates two strings a and b by appending b to a and
 * returns the result of that operation as a new string.
 * Both a and b may not be NULL!
 */
extern struct tarot_string* tarot_concat_strings(
	const struct tarot_string *a,
	const struct tarot_string *b
);

/**
 * Reverses the order of the characters in the string
 */
extern void tarot_reverse_string(struct tarot_string *string);

/**
 * Returns the character at position n in the string. The index
 * of the character may not exceed the strings character count!
 * If the index exceeds the character count, an exception is raised.
 */
extern int tarot_string_char(struct tarot_string *string, size_t n);

/**
 * Compares two strings a and b for equality.
 * If the strings are equal, the function returns true.
 * If not, the function returns false.
 */
extern bool tarot_compare_strings(
	const struct tarot_string *a,
	const struct tarot_string *b
);

/**
 * Compares the string with a c-string.
 */
extern bool tarot_match_string(
	const struct tarot_string *a,
	const char *b
);

/**
 * Tests whether the string contains the given value.
 */
extern bool tarot_string_contains(
	const struct tarot_string *string,
	const struct tarot_string *value
);

/**
 * Splits the string at the given index. The returned string contains
 * the first part, which in turn is removed from the given string.
 */
extern struct tarot_string* tarot_string_split(
	struct tarot_string **string,
	size_t index
);

/**
 * Prints the string to the iostream.
 */
extern void tarot_print_string(
	struct tarot_iostream *stream,
	const struct tarot_string *string
);

/**
 * Imports a string from the buffer. The string inside the buffer
 * must be null-terminated!
 */
extern struct tarot_string* tarot_import_string(void *buffer);

/**
 * Exports the given string to the given buffer.
 */
extern void tarot_export_string(void *buffer, struct tarot_string *string);

/**
 * Reads a string from the input stream.
 */
extern struct tarot_string* tarot_input_string(struct tarot_iostream *stream);

#endif /* TAROT_TYPE_STRING_H */
