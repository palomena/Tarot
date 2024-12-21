#ifndef TAROT_TYPE_DICTIONARY_H
#define TAROT_TYPE_DICTIONARY_H

#include "defines.h"

struct tarot_dictionary;

extern struct tarot_dictionary* tarot_create_dictionary();
extern void tarot_free_dictionary(struct tarot_dictionary *dict);
extern bool tarot_dict_contains(struct tarot_dictionary *dict, char *key);
extern void tarot_dict_insert(struct tarot_dictionary *dict, char *key, void *value);

#endif /* TAROT_TYPE_DICTIONARY_H */
