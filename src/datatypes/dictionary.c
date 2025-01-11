#define TAROT_SOURCE
#include "tarot.h"

static bool match_item(void *element, void *object) {
	struct dict_item *item = element;
	union tarot_value *key = object;
	return tarot_compare_strings(item->key.String, key->String);
}

struct tarot_list* tarot_create_dictionary() {
	struct tarot_list *dict = tarot_create_list(sizeof(struct dict_item), 5, match_item);
	tarot_tag(dict, TYPE_DICT);
	return dict;
}

struct tarot_list* tarot_copy_dict(struct tarot_list *dict) {
	return tarot_copy_list(dict);
}

void tarot_free_dictionary(struct tarot_list *dict) {
	/* FIXME: since dict list is tagged with subtype, it attempts to free said type and not the struct dict_item */
	tarot_free_list(dict);
}

bool tarot_dict_contains(struct tarot_list *dict, union tarot_value key) {
	return tarot_list_contains(dict, &key);
}

void tarot_dict_insert(struct tarot_list **dict, union tarot_value key, union tarot_value value) {
	struct dict_item item = {key, value};
	tarot_list_append(dict, &item);
}

union tarot_value* tarot_dict_lookup(struct tarot_list *dict, union tarot_value key) {
	struct dict_item *item;
	size_t index = tarot_list_lookup(dict, &key);
	item = tarot_list_element(dict, index);
	return &item->value;
}

void tarot_print_dict(struct tarot_iostream *stream, struct tarot_list *dict) {
	size_t i;
	tarot_fputc(stream, '{');
	tarot_newline(stream);
	tarot_indent(stream, 1);
	for (i = 0; i < tarot_list_length(dict); i++) {
		struct dict_item *item = tarot_list_element(dict, i);
		union tarot_value *value = &item->value;
		bool is_first_element = i == 0;
		if (not is_first_element) {
			tarot_fputc(stream, ',');
			tarot_newline(stream);
		}
		tarot_fputc(stream, '"');
		tarot_print_string(stream, item->key.String);
		tarot_fputc(stream, '"');
		tarot_fputc(stream, ':');
		tarot_fputc(stream, ' ');
		switch (tarot_get_list_datatype(dict)) {
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
	tarot_newline(stream);
	tarot_indent(stream, -1);
	tarot_fputc(stream, '}');
	tarot_newline(stream);
}
