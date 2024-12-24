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

struct block_header {
	size_t size;  /**< The size in bytes of the block of memory */
};

static struct block_header* header_of(void *ptr) {
	struct block_header *header = ptr;
	assert(header != NULL);
	return --header;
}

/* REGIONS */

static struct tarot_list *regions = NULL;
static uint16_t region_index = 0;
static bool regions_enabled = true;

static TAROT_INLINE struct tarot_list** get_region(uint16_t index) {
	assert(index < region_index);
	return tarot_list_element(regions, index);
}

static TAROT_INLINE struct tarot_list** current_region(void) {
	return get_region(region_index-1u);
}

static TAROT_INLINE struct tarot_list** parent_region(void) {
	return get_region(region_index-2u);
}

static TAROT_INLINE bool region_available(void) {
	return regions_enabled and region_index > 0;
}

TAROT_INLINE bool tarot_enable_regions(bool enable) {
	bool previous_state = regions_enabled;
	regions_enabled = enable;
	return previous_state;
}

void tarot_push_region(void) {
	void *container;
	region_index++;
	bool state = tarot_enable_regions(false);
	if (regions == NULL) {
		regions = tarot_create_list(sizeof(regions), 10, NULL);
	}
	container = tarot_create_list(sizeof(void*), 10, NULL);
	tarot_list_append(&regions, &container);
	tarot_enable_regions(state);
}

void tarot_pop_region(void) {
	size_t i;
	assert(region_index > 0);
	bool state = tarot_enable_regions(false);
	for (i = 0; i < tarot_list_length(*current_region()); i++) {
		void **ptr = tarot_list_element(*current_region(), i);
		tarot_free(*ptr);
	}
	tarot_clear_list(*current_region());
	tarot_free_list(*current_region());
	tarot_list_pop(&regions, NULL);
	tarot_enable_regions(state);
	region_index--;
}

void tarot_move_to_parent_region(void *ptr) {
	size_t index = tarot_list_lookup(*current_region(), &ptr);
	bool state = tarot_enable_regions(false);
	tarot_list_remove(current_region(), index);
	tarot_list_append(parent_region(), &ptr);
	tarot_enable_regions(state);
}

void tarot_remove_from_region(void *ptr) {
	size_t index = tarot_list_lookup(*current_region(), &ptr);
	bool state = tarot_enable_regions(false);
	tarot_list_remove(current_region(), index);
	tarot_enable_regions(state);
}

void tarot_activate_relative_region(int rel) {
	region_index += rel;
}

void tarot_print_region(uint16_t index) {
	size_t i;
	tarot_printf("region %zu\n", index);
	for (i = 0; i < tarot_list_length(*get_region(index)); i++) {
		void **ptr = tarot_list_element(*get_region(index), i);
		tarot_printf("[%zu] %p\n", i, *ptr);
	}
}

size_t tarot_num_active_regions(void) {
	return region_index;
}

/* Initialization */

static void* gmp_realloc(void *ptr, size_t old_size, size_t size) {
	unused(old_size);
	return tarot_realloc(ptr, size);
}

static void gmp_free(void *ptr, size_t size) {
	unused(size);
	tarot_free(ptr);
}

void tarot_initialize_memory_functions(void) {
	mp_set_memory_functions(tarot_malloc, gmp_realloc, gmp_free);
}

void tarot_clear_regions(void) {
	while (region_index > 0) {
		tarot_pop_region();
	}
	bool state = tarot_enable_regions(false);
	tarot_free_list(regions);
	tarot_enable_regions(state);
}

/* Allocators */

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
		/* Disable regions because list_append might allocate */
		if (region_available()) {
			bool state = tarot_enable_regions(false);
			tarot_list_append(current_region(), &ptr);
			tarot_enable_regions(state);
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
		if (region_available()) {
			size_t index = tarot_list_lookup(*current_region(), &ptr);
			tarot_list_replace(*current_region(), index, &new_ptr);
		}
		num_reallocations++;
	}
	return new_ptr;
}

void tarot_free(void *ptr) {
	if ((not region_available()) and ptr != NULL) {
		struct block_header *header;
		assert(num_frees < num_allocations);
		header = header_of(ptr);
		allocated_memory -= header->size;
		tarot_platform.free(header);
		num_frees++;
	}
}
