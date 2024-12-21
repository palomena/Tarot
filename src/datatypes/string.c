#define TAROT_SOURCE
#include "tarot.h"

struct tarot_string {
	char *text;
	size_t length;
	size_t capacity;
	size_t num_characters;
};

static struct tarot_string* tarot_allocate_string(size_t capacity) {
	struct tarot_string *string = tarot_malloc(sizeof(*string) + capacity);
	string->capacity = capacity;
	return string;
}

struct tarot_string* tarot_create_string(const char *format, ...) {
	struct tarot_string *string = tarot_allocate_string(8);
	if (format != NULL) {
		struct tarot_iostream *stream = tarot_fstropen(&string, TAROT_OUTPUT);
		va_list ap;

		va_start(ap, format);
		tarot_vfprintf(stream, format, &ap);
		va_end(ap);
		tarot_fclose(stream);
	}
	return string;
}

struct tarot_string* tarot_const_string(const char *text) {
	static struct tarot_string circular_string_buffer[8];
	static uint8_t length = lengthof(circular_string_buffer);
	static uint8_t index = 0;
	struct tarot_string *string = &circular_string_buffer[index++ % length];
	string->text = (char*)text;
	string->capacity = string->length = strlen(text);
	string->num_characters = string->length;
	return string;
}

TAROT_INLINE
static char* text_of(const struct tarot_string *string) {
	if (string->text != NULL) return string->text;
	return end_of_struct(string);
}

struct tarot_string* tarot_copy_string(struct tarot_string *string) {
	struct tarot_string *copy = tarot_allocate_string(string->capacity);
	copy->length = string->length;
	copy->num_characters = string->num_characters;
	memcpy(text_of(copy), text_of(string), string->length);
	return copy;
}

void tarot_clear_string(struct tarot_string *string) {
	memset(text_of(string), 0, string->length);
	string->length = 0;
	string->num_characters = 0;
}

void tarot_free_string(struct tarot_string *string) {
	tarot_free(string);
}

char* tarot_string_to_cstring(struct tarot_string *string) {
	char *text = tarot_malloc(string->length);
	memcpy(text, text_of(string), string->length);
	return text;
}

size_t tarot_string_length(struct tarot_string *string) {
	return string->length;
}

char* tarot_string_text(struct tarot_string *string) {
	return text_of(string);
}

static size_t strlen_utf8(const char *strptr) {
	size_t length = 0;
	assert(strptr != NULL);
	while (*strptr) {
		if ((*strptr & 0xC0) != 0x80) {
			length++;
		}
		strptr++;
	}
	return length;
}

static struct tarot_string* resize_string(
	struct tarot_string **stringptr,
	size_t size
) {
	struct tarot_string *string = *stringptr;
	if (string->capacity < size or (string->capacity / 2 > size)) {
		string = tarot_realloc(string, sizeof(*string) + size);
		string->capacity = size;
		*stringptr = string;
	}
	return string;
}

static struct tarot_string* extend_string(
	struct tarot_string **stringptr,
	size_t n
) {
	struct tarot_string *string = *stringptr;
	if ((string->length + n) >= string->capacity) {
		string->capacity = 2 * (string->length + n);
		string = tarot_realloc(string, sizeof(*string) + string->capacity);
		*stringptr = string;
	}
	return string;
}

void tarot_string_append(
	struct tarot_string **stringptr,
	const char *format, ...
) {
	struct tarot_string *string = *stringptr;
	struct tarot_iostream *stream;
	size_t length;
	va_list ap;

	va_start(ap, format);
	length = tarot_vfstrlen(format, &ap);
	va_end(ap);

	string = extend_string(stringptr, length);
	stream = tarot_fmemopen(text_of(string) + string->length, TAROT_OUTPUT);

	va_start(ap, format);
	tarot_vfprintf(stream, format, &ap);
	va_end(ap);
	tarot_fclose(stream);

	string->num_characters += strlen_utf8(text_of(string) + string->length);
	string->length += length;
}

void tarot_string_insert(
	struct tarot_string **stringptr,
	size_t index,
	const char *format, ...
) {
	struct tarot_string *string = *stringptr;
	struct tarot_iostream *stream;
	size_t length;
	va_list ap;

	va_start(ap, format);
	length = tarot_vfstrlen(format, &ap);
	va_end(ap);

	string = extend_string(stringptr, length);
	memmove(text_of(string) + index + length, text_of(string) + index, length);
	stream = tarot_fmemopen(text_of(string) + index, TAROT_OUTPUT);

	va_start(ap, format);
	tarot_vfprintf(stream, format, &ap);
	va_end(ap);
	tarot_fclose(stream);

	string->length += length;
	string->num_characters = strlen_utf8(text_of(string));
}

void tarot_string_remove(
	struct tarot_string **stringptr,
	size_t index,
	size_t n
) {
	struct tarot_string *string = *stringptr;
	size_t length;
	assert(index + n < string->length); /* Out of bounds? */
	string->length -= n;
	length = string->length - index;
	memmove(text_of(string) + index, text_of(string) + index + n, length);
	memset(text_of(string) + index + n, 0, length);
}

bool tarot_string_find(
	struct tarot_string *string,
	const char *substring,
	size_t *index
) {
	char *result = strstr(text_of(string), substring);
	if (result != NULL) {
		*index = result - text_of(string);
	}
	return false;
}

struct tarot_string* tarot_concat_strings(
	const struct tarot_string *a,
	const struct tarot_string *b
) {
	struct tarot_string *result = tarot_allocate_string(a->length + b->length + 2);
	memcpy(text_of(result), text_of(a), a->length);
	memmove(text_of(result) + a->length, text_of(b), b->length);
	result->length = a->length + b->length;
	result->num_characters = a->num_characters + b->num_characters;
	return result;
}

void tarot_reverse_string(struct tarot_string *string) {
	strrev(text_of(string));
}

int tarot_string_char(struct tarot_string *string, size_t n) {
	assert(n < string->num_characters);
	/* Assert fails if character index exceeds string character count */
	return text_of(string)[n]; /* FIXME: Not utf8 compliant */
}

bool tarot_compare_strings(
	const struct tarot_string *a,
	const struct tarot_string *b
) {
	assert(a != NULL);
	assert(b != NULL);
	return (a->length == b->length) and tarot_match_string(a, text_of(b));
}

TAROT_INLINE
bool tarot_match_string(
	const struct tarot_string *a,
	const char *b
) {
	return !strcmp(text_of(a), b);
}

bool tarot_string_contains(
	const struct tarot_string *string,
	const struct tarot_string *value
) {
	bool result = false;
	size_t i;
	assert(string != NULL);
	assert(value != NULL);
	for (i = 0; i < string->length; i++) {
		size_t remaining = string->length - i;
		if (remaining < value->length) {
			break;
		}
		if (!strncmp(&text_of(string)[i], text_of(value), value->length)) {
			result = true;
			break;
		}
	}
	return result;
}

struct tarot_string* tarot_string_split(
	struct tarot_string **string,
	size_t index
) {
	struct tarot_string *slice = tarot_allocate_string(index);
	assert(index < (*string)->length);
	slice->length = index;
	memcpy(text_of(slice), text_of(*string), index);
	tarot_string_remove(string, 0, index);
	return slice;
}

void tarot_print_string(
	struct tarot_iostream *stream,
	const struct tarot_string *string
) {
	size_t i;
	assert(stream != NULL);
	assert(string != NULL);
	for (i = 0; i < string->length; i++) {
		tarot_fputc(stream, text_of(string)[i]);
	}
}

struct tarot_string* tarot_import_string(void *buffer) {
	return tarot_create_string("%s", buffer);
}

void tarot_export_string(void *buffer, struct tarot_string *string) {
	strcpy(buffer, text_of(string));
}

struct tarot_string* tarot_input_string(struct tarot_iostream *stream) {
	struct tarot_string *string = tarot_allocate_string(32);
	int ch;
	for (ch = tarot_fgetc(stream); ch != '\n' and ch != -1; ch = tarot_fgetc(stream)) {
		tarot_string_append(&string, "%c", ch);
	}
	return string;
}
