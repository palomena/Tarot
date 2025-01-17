#ifndef TAROT_REGION_H
#define TAROT_REGION_H

/**
 * Automatic memory management using memory regions.
 * When a tarot function is called it creates a new callframe on the stack.
 * This callframe contains space for a fixed amount of local regions.
 */

#include "defines.h"

/**
 * Creates a new region and makes it current.
 */
extern void tarot_push_region(struct tarot_thread *thread);

/**
 * Removes the current region and makes the previous region current.
 */
extern void tarot_pop_region(struct tarot_thread *thread);

extern void tarot_add_to_region(struct tarot_thread *thread, void *ptr);
extern void tarot_remove_from_region(struct tarot_thread *thread, void *ptr);

extern void tarot_clear_regions(struct tarot_thread *thread);

extern bool tarot_is_tracked(struct tarot_thread *thread, void *ptr);

#endif /* TAROT_REGION_H */
