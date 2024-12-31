#define TAROT_SOURCE
#include "tarot.h"

static bool match_item(void *element, void *object) {
	struct dict_item *item = element;
	union tarot_value *key = object;
	return tarot_compare_strings(item->key.String, key->String);
}

struct tarot_list* tarot_create_dictionary() {
	struct tarot_list *dict = tarot_create_list(sizeof(struct dict_item), 5, match_item);
	return dict;
}

struct tarot_list* tarot_copy_dict(struct tarot_list *dict) {
	return tarot_copy_list(dict);
}

void tarot_free_dictionary(struct tarot_list *dict) {
	tarot_free_list(dict);
}

bool tarot_dict_contains(struct tarot_list *dict, union tarot_value key) {
	return tarot_list_contains(dict, &key);
}

void tarot_dict_insert(struct tarot_list **dict, union tarot_value key, union tarot_value value) {
	struct dict_item item = {key, value};
	tarot_list_append(dict, &item);
}

union tarot_value tarot_dict_lookup(struct tarot_list *dict, union tarot_value key) {
	struct dict_item *item;
	size_t index = tarot_list_lookup(dict, &key);
	item = tarot_list_element(dict, index);
	return item->value;
}
