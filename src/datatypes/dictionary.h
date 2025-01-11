#ifndef TAROT_TYPE_DICTIONARY_H
#define TAROT_TYPE_DICTIONARY_H

#include "defines.h"

struct tarot_list;
union tarot_value;

extern struct tarot_list* tarot_create_dictionary();
extern struct tarot_list* tarot_copy_dict(struct tarot_list *dict);
extern void tarot_free_dictionary(struct tarot_list *dict);
extern bool tarot_dict_contains(struct tarot_list *dict, union tarot_value key);
extern void tarot_dict_insert(struct tarot_list **dict, union tarot_value key, union tarot_value value);
extern union tarot_value* tarot_dict_lookup(struct tarot_list *dict, union tarot_value key);

#ifdef TAROT_SOURCE

#include "datatypes/value.h"

struct dict_item {
	union tarot_value key;
	union tarot_value value;
};

#endif

#endif /* TAROT_TYPE_DICTIONARY_H */
