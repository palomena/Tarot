#define TAROT_SOURCE
#include "tarot.h"

/* DIAGNOSTICS */

static size_t num_allocations = 0;
static size_t num_reallocations = 0;
static size_t num_frees = 0;
static size_t allocated_memory = 0;
static size_t total_memory = 0;

TAROT_INLINE
size_t tarot_num_allocations(void) {
	return num_allocations;
}

TAROT_INLINE
size_t tarot_num_reallocations(void) {
	return num_reallocations;
}

TAROT_INLINE
size_t tarot_num_frees(void) {
	return num_frees;
}

TAROT_INLINE
size_t tarot_total_memory(void) {
	return total_memory;
}

/* Metadata */

struct block_header* header_of(void *ptr) {
	struct block_header *header = ptr;
	assert(header != NULL);
	return --header;
}

/* Allocators */

void tarot_tag(void *ptr, int type) {
	header_of(ptr)->type = type;
}

void* tarot_malloc(size_t size) {
	void *ptr = NULL;
	struct block_header *header;
	if (size > 0) {
		header = tarot_platform.malloc(sizeof(*header) + size);
		assert(header != NULL);
		header->size = size;
		ptr = end_of_struct(header);
		memset(ptr, 0, size);
		num_allocations++;
		allocated_memory += size;
		if (allocated_memory > total_memory) {
			total_memory = allocated_memory;
		}
	}
	return ptr;
}

void* tarot_realloc(void *ptr, size_t size) {
	void *new_ptr;
	if (ptr == NULL) {
		new_ptr = tarot_malloc(size);
	} else {
		struct block_header *header;
		struct block_header *old_header = header_of(ptr);
		size_t old_size = old_header->size;
		allocated_memory -= old_size;
		header = tarot_platform.realloc(old_header, size + sizeof(*header));
		header->size = size;
		new_ptr = end_of_struct(header);
		if (size > old_size) {
			memset((char*)new_ptr + old_size, 0, size - old_size);
		}
		allocated_memory += size;
		if (allocated_memory > total_memory) {
			total_memory = allocated_memory;
		}
		num_reallocations++;
	}
	return new_ptr;
}

void tarot_free(void *ptr) {
	if (ptr != NULL) {
		struct block_header *header;
		assert(num_frees < num_allocations);
		header = header_of(ptr);
		allocated_memory -= header->size;
		tarot_platform.free(header);
		num_frees++;
	}
}
