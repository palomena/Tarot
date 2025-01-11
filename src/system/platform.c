#define TAROT_SOURCE
#include "tarot.h"

static bool is_initialized = false;
struct tarot_platform_config tarot_platform;

static enum tarot_byteorder {
	TAROT_LITTLE_ENDIAN=1,
	TAROT_BIG_ENDIAN
} byteorder;

TAROT_INLINE
static void determine_byteorder(void) {
	static const int x = 1;
	byteorder = (*((char*)&x) == 1) ? TAROT_LITTLE_ENDIAN : TAROT_BIG_ENDIAN;
}

static void* gmp_realloc(void *ptr, size_t old_size, size_t size) {
	unused(old_size);
	return tarot_realloc(ptr, size);
}

static void gmp_free(void *ptr, size_t size) {
	unused(size);
	tarot_free(ptr);
}

void tarot_initialize(const struct tarot_platform_config *cfg) {
	assert(not is_initialized);
	assert(sizeof(int8_t)   * CHAR_BIT == 8);
	assert(sizeof(uint8_t)  * CHAR_BIT == 8);
	assert(sizeof(int16_t)  * CHAR_BIT == 16);
	assert(sizeof(uint16_t) * CHAR_BIT == 16);
	assert(sizeof(int32_t)  * CHAR_BIT == 32);
	assert(sizeof(uint32_t) * CHAR_BIT == 32);
	memcpy(&tarot_platform, cfg, sizeof(tarot_platform));
	determine_byteorder();
	tarot_open_stdout(tarot_platform.cout);
	tarot_open_stderr(tarot_platform.cerr);
	tarot_open_stdin(tarot_platform.cin);
	mp_set_memory_functions(tarot_malloc, gmp_realloc, gmp_free);
	is_initialized = true;
	tarot_log("Initialized platform interface");
}

TAROT_INLINE
bool tarot_is_initialized(void) {
	return is_initialized;
}

int tarot_exit(void) {
	if (is_initialized) {
		/*tarot_clear_regions();*/
		tarot_log("De-initialized platform interface");
		if (tarot_num_allocations() != tarot_num_frees()) {
			tarot_warning(
				"Leaked memory: %zu allocations / %zu frees",
				tarot_num_allocations(),
				tarot_num_frees()
			);
		}
		tarot_fclose(tarot_stdout);
		tarot_fclose(tarot_stderr);
		tarot_fclose(tarot_stdin);
		memset(&tarot_platform, 0, sizeof(tarot_platform));
		is_initialized = false;
	}
	/* TODO: Reset error/memory statistics */
	return tarot_num_errors() != 0u;
}

TAROT_INLINE
uint8_t tarot_read8bit(uint8_t buffer[1], uint8_t **endptr) {
	if (endptr) *endptr = &buffer[1];
	return buffer[0];
}

TAROT_INLINE
uint16_t tarot_read16bit(uint8_t buffer[2], uint8_t **endptr) {
	if (endptr) *endptr = &buffer[2];
	if (byteorder & TAROT_LITTLE_ENDIAN) {
		return (buffer[1] << 8) | buffer[0];
	} else {
		return (buffer[0] << 8) | buffer[1];
	}
}

TAROT_INLINE
uint32_t tarot_read24bit(uint8_t buffer[3], uint8_t **endptr) {
	if (endptr) *endptr = &buffer[3];
	if (byteorder & TAROT_LITTLE_ENDIAN) {
		return (buffer[2] << 8) | (buffer[1] << 8) | buffer[0];
	} else {
		return (buffer[0] << 8) | (buffer[1] << 8) | buffer[2];
	}
}

TAROT_INLINE
uint32_t tarot_read32bit(uint8_t buffer[4], uint8_t **endptr) {
	if (endptr) *endptr = &buffer[4];
	if (byteorder & TAROT_LITTLE_ENDIAN) {
		return (
			(buffer[3] << 24)
			| (buffer[2] << 16)
			| (buffer[1] << 8)
			| buffer[0]
		);
	} else {
		return (
			(buffer[0] << 24)
			| (buffer[1] << 16)
			| (buffer[2] << 8)
			| buffer[3]
		);
	}
}

TAROT_INLINE
double tarot_read_float(uint8_t buffer[8]) {
	uint32_t aligned_buffer[2];
	aligned_buffer[0] = tarot_read32bit(buffer, NULL);
	aligned_buffer[1] = tarot_read32bit(buffer+4, NULL);
	return *(double*)aligned_buffer; /* FIXME: guilty of type punning! */
}

TAROT_INLINE
void tarot_write8bit(uint8_t buffer[1], uint8_t value) {
	buffer[0] = value & 0xFF;
}

TAROT_INLINE
void tarot_write16bit(uint8_t buffer[2], uint16_t value) {
	if (byteorder & TAROT_LITTLE_ENDIAN) {
		buffer[0] = value & 0xFF;
		buffer[1] = (value >> 8) & 0xFF;
	} else {
		buffer[1] = value & 0xFF;
		buffer[0] = (value >> 8) & 0xFF;
	}
}

TAROT_INLINE
void tarot_write24bit(uint8_t buffer[3], uint32_t value) {
	if (byteorder & TAROT_LITTLE_ENDIAN) {
		buffer[0] = value & 0xFF;
		buffer[1] = (value >> 8) & 0xFF;
		buffer[2] = (value >> 16) & 0xFF;
	} else {
		buffer[2] = value & 0xFF;
		buffer[1] = (value >> 8) & 0xFF;
		buffer[0] = (value >> 16) & 0xFF;
	}
}

TAROT_INLINE
void tarot_write32bit(uint8_t buffer[4], uint32_t value) {
	if (byteorder & TAROT_LITTLE_ENDIAN) {
		buffer[0] = value & 0xFF;
		buffer[1] = (value >> 8) & 0xFF;
		buffer[2] = (value >> 16) & 0xFF;
		buffer[3] = (value >> 24) & 0xFF;
	} else {
		buffer[3] = value & 0xFF;
		buffer[2] = (value >> 8) & 0xFF;
		buffer[1] = (value >> 16) & 0xFF;
		buffer[0] = (value >> 24) & 0xFF;
	}
}

TAROT_INLINE
void tarot_write_float(uint8_t buffer[8], double value) {
	uint32_t aligned_buffer[2];
	*(double*)aligned_buffer = value; /* FIXME: type punning! */
	tarot_write32bit(buffer, aligned_buffer[0]);
	tarot_write32bit(buffer+4, aligned_buffer[1]);
}

TAROT_INLINE
size_t tarot_align(size_t n) {
	if (n % sizeof(void*)) {
		n += sizeof(void*) - n % sizeof(void*);
	}
	return n;
}

uint8_t tarot_cast8bit(uint32_t value) {
	assert (value < UINT8_MAX);
	return value;
}

uint16_t tarot_cast16bit(uint32_t value) {
	assert (value < UINT16_MAX);
	return value;
}

uint32_t tarot_cast24bit(uint32_t value) {
	assert (value < UINT32_MAX/UINT8_MAX);
	return value;
}
