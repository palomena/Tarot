#define TAROT_SOURCE
#include "tarot.h"

struct tarot_object {
	union tarot_value *attributes;
	uint8_t num_attributes;
};

struct tarot_object* tarot_create_object(unsigned int n) {
	struct tarot_object *object = tarot_malloc(sizeof(*object) + sizeof(*object->attributes) * n);
	object->attributes = end_of_struct(object);
	object->num_attributes = n;
	tarot_tag(object, TYPE_CUSTOM);
	return object;
}

/* Make free value function */
void tarot_free_object(struct tarot_object *object) {
	unsigned int i;
	for (i = 0; i < object->num_attributes; i++) {
		union tarot_value *attribute = &object->attributes[i];
		switch (header_of(attribute->Pointer)->type) {
			default:
			puts("estststr");
				break;
			case TYPE_INTEGER:
				tarot_free_integer(attribute->Integer);
				break;
			case TYPE_STRING:
				tarot_free_string(attribute->String);
				break;
			case TYPE_LIST:
				tarot_free_list(attribute->List);
				break;
			case TYPE_CUSTOM:
				tarot_free_object(attribute->Object);
				break;
		}
	}
	tarot_free(object);
}

union tarot_value* tarot_object_attribute(
	struct tarot_object *object,
	unsigned int index
) {
	assert(index < object->num_attributes);
	return &object->attributes[index];
}
