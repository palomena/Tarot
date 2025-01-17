#ifndef TAROT_OBJECT_H
#define TAROT_OBJECT_H

#include "defines.h"

struct tarot_object;

/**
 * Allocates an object within the current region.
 */
extern struct tarot_object* tarot_create_object(unsigned int n);

/**
 * Frees an allocated object.
 */
extern void tarot_free_object(struct tarot_object *object);

/**
 *
 */
extern union tarot_value* tarot_object_attribute(
	struct tarot_object *object,
	unsigned int index
);

#endif /* TAROT_OBJECT_H */
