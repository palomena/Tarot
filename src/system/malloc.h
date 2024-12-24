#ifndef TAROT_MALLOC_H
#define TAROT_MALLOC_H

#include "defines.h"

#ifdef TAROT_SOURCE
extern void tarot_initialize_memory_functions(void);
extern void tarot_clear_regions(void); /* TODO: rename to deinitialize_mem...? */
#endif

extern bool tarot_enable_regions(bool enable);
extern void tarot_push_region(void);
extern void tarot_pop_region(void);
extern void tarot_move_to_parent_region(void *ptr);
extern void tarot_remove_from_region(void *ptr);
extern void tarot_activate_relative_region(int rel);
extern void tarot_print_region(uint16_t index);

extern void* tarot_malloc(size_t size);
extern void* tarot_realloc(void *ptr, size_t size);
extern void  tarot_free(void *ptr);

extern size_t tarot_num_allocations(void);
extern size_t tarot_num_reallocations(void);
extern size_t tarot_num_frees(void);
extern size_t tarot_total_memory(void);
extern size_t tarot_num_active_regions(void);

#endif /* TAROT_MALLOC_H */
