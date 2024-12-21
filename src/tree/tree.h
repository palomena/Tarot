#ifndef TAROT_TREE_H
#define TAROT_TREE_H

#include "defines.h"
#include "tree/node.h"
#include "tree/scope.h"

/**
 * Imports the source file at path into a ready-to-use abstract syntax tree.
 * The resulting abstract syntax tree requires no further processing
 * and is valid. Invalid ASTs are destroyed and subsequently NULL is returned.
 */
extern struct tarot_node* tarot_import(const char *path);
extern struct tarot_node* tarot_parse(struct tarot_iostream *stream);
extern struct tarot_node* tarot_parse_text(const char *data);
extern struct tarot_node* tarot_parse_file(const char *path);
extern bool tarot_validate(struct tarot_node *ast);

#endif /* TAROT_TREE_H */
