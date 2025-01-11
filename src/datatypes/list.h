#ifndef TAROT_TYPE_LIST_H
#define TAROT_TYPE_LIST_H

#include "defines.h"

struct tarot_iostream;
struct tarot_list;
enum tarot_datatype;

extern struct tarot_list* tarot_create_list(
	size_t objsize,
	size_t length,
	bool (*match)(void *element, void *object)
);
extern struct tarot_list* tarot_copy_list(struct tarot_list *list);
extern void tarot_free_list(struct tarot_list *list);
extern void tarot_clear_list(struct tarot_list *list);
extern void* tarot_list_to_array(struct tarot_list *list);
extern size_t tarot_list_length(struct tarot_list *list);
extern size_t tarot_list_objsize(struct tarot_list *list);
extern void tarot_trim_list(struct tarot_list **list);
extern void tarot_reverse_list(struct tarot_list *list);
extern void tarot_list_append(struct tarot_list **list, void *object);
extern void tarot_list_pop(struct tarot_list **list, void *object);
extern void tarot_list_insert(
	struct tarot_list **list,
	size_t index,
	void *object
);
extern bool tarot_list_contains(struct tarot_list *list, void *object);
extern void tarot_list_remove(struct tarot_list **list, size_t index);
extern void* tarot_list_element(struct tarot_list *list, size_t index);
extern bool tarot_list_find(
	struct tarot_list *list,
	void *object,
	size_t *index
);
extern size_t tarot_list_lookup(struct tarot_list *list, void *object);
extern void tarot_list_replace(
	struct tarot_list *list,
	size_t index,
	void *object
);
extern void tarot_set_list_datatype(struct tarot_list *list, enum tarot_datatype type);
extern enum tarot_datatype tarot_get_list_datatype(struct tarot_list *list);
extern void tarot_print_list(struct tarot_iostream *stream, struct tarot_list *list);

#endif /* TAROT_TYPE_LIST_H */
