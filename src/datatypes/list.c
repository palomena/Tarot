#define TAROT_SOURCE
#include "tarot.h"

struct tarot_list {
	size_t length;
	size_t capacity;
	size_t objsize;
	enum tarot_datatype element_type;
	bool (*match)(void *element, void *object);
};

static bool match(struct tarot_list *list, void *element, void *object) {
	if (list->match != NULL) {
		return list->match(element, object);
	}
	return !memcmp(element, object, list->objsize);
}

struct tarot_list* tarot_create_list(
	size_t objsize,
	size_t n,
	bool (*match)(void *element, void *object)
) {
	struct tarot_list *list = tarot_malloc(sizeof(*list) + objsize * n);
	assert(objsize > 0);
	list->length = 0;
	list->capacity = n;
	list->objsize = objsize;
	list->match = match;
	tarot_tag(list, TYPE_LIST);
	return list;
}

TAROT_INLINE
static void* data_of(struct tarot_list *list) {
	return end_of_struct(list);
}

struct tarot_list* tarot_copy_list(struct tarot_list *list) {
	struct tarot_list *copy = tarot_create_list(
		list->objsize,
		list->capacity,
		list->match
	);
	tarot_set_list_datatype(copy, list->element_type);
	copy->length = list->length;
	switch (list->element_type) {
		size_t i;
		default:
			memcpy(data_of(copy), data_of(list), list->length * list->objsize);
			break;
		case TYPE_INTEGER:
			for (i = 0; i < list->length; i++) {
				union tarot_value *value = tarot_list_element(list, i);
				union tarot_value *valptr = tarot_list_element(copy, i);
				valptr->Integer = tarot_copy_integer(value->Integer);
			}
			break;
	}
	return copy;
}

void tarot_free_list(struct tarot_list *list) {
	if (list != NULL) {
		size_t i;
		bool state = tarot_enable_regions(false);
		for (i = 0; i < list->length; i++) {
			union tarot_value *value = tarot_list_element(list, i);
			switch (list->element_type) {
				default:
					break;
				case TYPE_LIST:
					tarot_free_list(value->List);
					break;
				case TYPE_INTEGER:
					tarot_free_integer(value->Integer);
					break;
				case TYPE_RATIONAL:
					tarot_free_rational(value->Rational);
					break;
				case TYPE_STRING:
					tarot_free_string(value->String);
					break;
			}
		}
		tarot_enable_regions(state);
		tarot_free(list);
	}
}

void* tarot_list_to_array(struct tarot_list *list) {
	if (list->length == 0) {
		return NULL;
	} else {
		void *data = tarot_malloc(list->objsize * list->length);
		memcpy(data, data_of(list), list->objsize * list->length);
		return data;
	}
}

size_t tarot_list_length(struct tarot_list *list) {
	return list->length;
}

size_t tarot_list_objsize(struct tarot_list *list) {
	return list->objsize;
}

void tarot_clear_list(struct tarot_list *list) {
	assert(list != NULL);
	list->length = 0;
	list->capacity = 0;
}

static void extend_list(struct tarot_list **listptr, size_t n) {
	struct tarot_list *list = *listptr;
	assert(n > 0);
	list->capacity += n;
	list = tarot_realloc(list, sizeof(*list) + list->capacity * list->objsize);
	*listptr = list;
}

static void shrink_list(struct tarot_list **listptr, size_t n) {
	struct tarot_list *list = *listptr;
	assert(n < list->capacity);
	if (n > 0) {
		list->capacity -= n;
		list = tarot_realloc(list, sizeof(*list) + list->capacity * list->objsize);
		*listptr = list;
	}
}

void tarot_trim_list(struct tarot_list **listptr) {
	struct tarot_list *list = *listptr;
	if (list->length > 0) {
		shrink_list(listptr, list->capacity - list->length);
	}
}

void tarot_reverse_list(struct tarot_list *list) {
	size_t index;
	uint8_t *buffer = tarot_malloc(list->objsize);
	for (index = 0; index < list->length; index++) {
		void *left = tarot_list_element(list, index);
		void *right = tarot_list_element(list, list->length - 1 - index);
		memcpy(buffer, left, list->objsize);
		memcpy(left, right, list->objsize);
		memcpy(right, buffer, list->objsize);
	}
	tarot_free(buffer);
}

void tarot_list_append(struct tarot_list **listptr, void *object) {
	struct tarot_list *list = *listptr;
	assert(object != NULL);
	if (list->length >= list->capacity) {
		extend_list(listptr, list->length * 2 + 1);
		list = *listptr;
	}
	list->length++;
	if (object != NULL) {
		memcpy(
			tarot_list_element(list, list->length-1),
			object,
			list->objsize
		);
	}
}

void tarot_list_pop(struct tarot_list **listptr, void *object) {
	struct tarot_list *list = *listptr;
	assert(list->length > 0);
	if (object != NULL) {
		memcpy(
			object,
			tarot_list_element(list, list->length-1),
			list->objsize
		);
	}
	list->length--;
}

void tarot_list_insert(
	struct tarot_list **listptr,
	size_t index,
	void *object
) {
	struct tarot_list *list = *listptr;
	void *destination;
	void *source;
	list->length++;
	source = tarot_list_element(list, index);
	destination = (uint8_t*)source + list->objsize;
	assert(list->length > 0);
	assert(index < list->length);
	if (index < list->length) {
		memmove(destination, source, (list->length - index) * list->objsize);
	}
	tarot_list_replace(list, index, object);
}

bool tarot_list_contains(struct tarot_list *list, void *object) {
	size_t i;
	for (i = 0; i < tarot_list_length(list); i++) {
		if (match(list, tarot_list_element(list, i), object)) {
			return true;
		}
	}
	return false;
}

void tarot_list_remove(struct tarot_list **listptr, size_t index) {
	struct tarot_list *list = *listptr;
	void *destination = tarot_list_element(list, index);
	void *source = (uint8_t*)destination + list->objsize;
	assert(list->length > 0);
	assert(index < list->length);
	list->length--;
	if (index < list->length) {
		memmove(destination, source, (list->length - index) * list->objsize);
	}
}

void* tarot_list_element(struct tarot_list *list, size_t index) {
	assert(index < list->length);
	return (uint8_t*)data_of(list) + list->objsize * index;
}

bool tarot_list_find(
	struct tarot_list *list,
	void *object,
	size_t *index
) {
	bool result = false;
	size_t i;
	assert(object != NULL);
	assert(index != NULL);
	*index = 0;
	for (i = 0; i < list->length; i++) {
		if (match(list, tarot_list_element(list, i), object)) {
			result = true;
			*index = i;
			break;
		}
	}
	return result;
}

size_t tarot_list_lookup(struct tarot_list *list, void *object) {
	size_t index;
	bool success = tarot_list_find(list, object, &index);
	assert(success);
	return index;
}

void tarot_list_replace(
	struct tarot_list *list,
	size_t index,
	void *object
) {
	memcpy(tarot_list_element(list, index), object, list->objsize);
}

void tarot_set_list_datatype(struct tarot_list *list, enum tarot_datatype type) {
	list->element_type = type;
}

enum tarot_datatype tarot_get_list_datatype(struct tarot_list *list) {
	return list->element_type;
}

void tarot_print_list(struct tarot_iostream *stream, struct tarot_list *list) {
	size_t i;
	tarot_fputc(stream, '[');
	for (i = 0; i < list->length; i++) {
		union tarot_value *value = tarot_list_element(list, i);
		bool is_first_element = i == 0;
		if (not is_first_element) {
			tarot_fputc(stream, ',');
			tarot_fputc(stream, ' ');
		}
		switch (list->element_type) {
			default:
				tarot_print("???");
				break;
			case TYPE_FLOAT:
				tarot_fprintf(stream, "%f", value->Float);
				break;
			case TYPE_INTEGER:
				tarot_print_integer(stream, value->Integer);
				break;
			case TYPE_LIST:
				tarot_print_list(stream, value->List);
				break;
			case TYPE_RATIONAL:
				tarot_print_rational(stream, value->Rational);
				break;
			case TYPE_STRING:
				tarot_fputc(stream, '"');
				tarot_print_string(stream, value->String);
				tarot_fputc(stream, '"');
				break;
		}
	}
	tarot_fputc(stream, ']');
}
/* where are lists used? maybe always have datatype? or different list type */
