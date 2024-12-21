#ifndef TAROT_INITIALIZE_H
#define TAROT_INITIALIZE_H

#include "defines.h"

typedef void  (*tarot_abort_function)  (void);
typedef void* (*tarot_malloc_function) (size_t size);
typedef void* (*tarot_realloc_function)(void *ptr, size_t size);
typedef void  (*tarot_free_function)   (void *ptr);
typedef void* (*tarot_fopen_function)  (const char*, const char*);
typedef int   (*tarot_fclose_function) (void*);
typedef int   (*tarot_fgetc_function)  (void*);
typedef int   (*tarot_fputc_function)  (int, void*);

struct tarot_platform_config {
	tarot_abort_function   abort;
	tarot_malloc_function  malloc;
	tarot_realloc_function realloc;
	tarot_free_function    free;
	tarot_fopen_function   fopen;
	tarot_fclose_function  fclose;
	tarot_fgetc_function   fgetc;
	tarot_fputc_function   fputc;
	void* cin;
	void* cout;
	void* cerr;
};

extern void tarot_initialize(const struct tarot_platform_config *cfg);
extern int  tarot_exit(void);
extern bool tarot_is_initialized(void);

/* Only available to tarot source files */
#ifdef TAROT_SOURCE
extern struct tarot_platform_config tarot_platform;
#endif

/* Endian-sensitive functions */
extern uint8_t  tarot_read8bit (uint8_t buffer[1], uint8_t **endptr);
extern uint16_t tarot_read16bit(uint8_t buffer[2], uint8_t **endptr);
extern uint32_t tarot_read24bit(uint8_t buffer[3], uint8_t **endptr);
extern uint32_t tarot_read32bit(uint8_t buffer[4], uint8_t **endptr);
extern double   tarot_read_float(uint8_t buffer[8]);
extern void tarot_write8bit(uint8_t buffer[1], uint8_t value);
extern void tarot_write16bit(uint8_t buffer[2], uint16_t value);
extern void tarot_write24bit(uint8_t buffer[3], uint32_t value);
extern void tarot_write32bit(uint8_t buffer[4], uint32_t value);
extern void tarot_write_float(uint8_t buffer[8], double value);

/* Alignment and mapping */
extern size_t tarot_align(size_t n);

#endif /* TAROT_INITIALIZE_H */
