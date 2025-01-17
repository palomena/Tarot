#define TAROT_SOURCE
#include "tarot.h"

TAROT_INLINE
static struct tarot_list** current_region(struct tarot_thread *thread) {
	return &current_frame(thread)->scope.region;
}

static uint8_t current_index(struct tarot_thread *thread) {
	if (current_frame(thread)->scope.index == 0) return 0;
	return current_frame(thread)->scope.indices[current_frame(thread)->scope.index-1];
}

void tarot_push_region(struct tarot_thread *thread) {
	void *container;
	current_frame(thread)->scope.index++;
	assert(current_frame(thread)->scope.index < sizeof(current_frame(thread)->scope.indices));
	if (current_frame(thread)->scope.index == 1) {
		*current_region(thread) = tarot_create_list(sizeof(void*), 10, NULL);
	}
	current_frame(thread)->scope.indices[current_frame(thread)->scope.index-1] = tarot_list_length(*current_region(thread));
}

void tarot_pop_region(struct tarot_thread *thread) {
	size_t i;
	assert(current_frame(thread)->scope.index > 0);
	struct tarot_list *region = *current_region(thread);
	size_t length = tarot_list_length(region);
	for (i = current_index(thread); i < length; i++) {
		void *ptr;
		tarot_list_pop(current_region(thread), &ptr);
		switch (header_of(ptr)->type) {
			default:
				tarot_free(ptr);
				break;
			case TYPE_INTEGER:
				tarot_free_integer(ptr);
				break;
			case TYPE_RATIONAL:
				tarot_free_rational(ptr);
				break;
			case TYPE_STRING:
				tarot_free_string(ptr);
				break;
			case TYPE_LIST:
				tarot_free_list(ptr);
				break;
			case TYPE_DICT:
				tarot_free_dictionary(ptr);
				break;
			case TYPE_CUSTOM:
				tarot_free_object(ptr);
				break;
		}
	}
	if (current_frame(thread)->scope.index == 1) {
		tarot_clear_list(region);
		tarot_free_list(region);
	}
	current_frame(thread)->scope.index--;
}

void tarot_add_to_region(struct tarot_thread *thread, void *ptr) {
	tarot_list_append(current_region(thread), &ptr);
}

void tarot_remove_from_region(struct tarot_thread *thread, void *ptr) {
	size_t index = tarot_list_lookup(*current_region(thread), &ptr);
	tarot_list_remove(current_region(thread), index);
}

void tarot_clear_regions(struct tarot_thread *thread) {
	struct stackframe *frame = current_frame(thread);
	while (frame->scope.index > 0) {
		tarot_pop_region(thread);
	}
}

bool tarot_is_tracked(struct tarot_thread *thread, void *ptr) {
	return tarot_list_contains(*current_region(thread), &ptr);
}

