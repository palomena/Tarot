#ifndef TAROT_MALLOC_H
#define TAROT_MALLOC_H

#include "defines.h"

extern void* tarot_malloc(size_t size);
extern void* tarot_realloc(void *ptr, size_t size);
extern void  tarot_free(void *ptr);
extern void tarot_tag(void *ptr, int type);

extern size_t tarot_num_allocations(void);
extern size_t tarot_num_reallocations(void);
extern size_t tarot_num_frees(void);
extern size_t tarot_total_memory(void);
extern size_t tarot_num_active_regions(void);

#ifdef TAROT_SOURCE

struct block_header {
	size_t size;  /**< The size in bytes of the block of memory */
	int type;
};

extern struct block_header* header_of(void *ptr);

#endif /* TAROT_SOURCE */

#endif /* TAROT_MALLOC_H */
