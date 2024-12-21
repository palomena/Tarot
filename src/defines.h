#ifndef TAROT_DEFINES_H
#define TAROT_DEFINES_H

/* Freestanding ISO C89 standard libary header files */
#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>

/* Tarot provides its own ISO C99 header files where needed */
#include "system/stdbool.h"
#include "system/stdint.h"

/* C inline keyword */
#if defined __has_attribute
#  if __has_attribute (always_inline)
#    define TAROT_INLINE __attribute__ ((always_inline)) __inline__
#  endif
#else
#  define TAROT_INLINE
#endif

/* The build mode is specified by the Makefile */
#if defined DEBUG
#  define TAROT_BUILD_MODE "debug"
#elif defined NDEBUG
#  define TAROT_BUILD_MODE "release"
#else
#  define TAROT_BUILD_MODE "default"
#endif

/* Utility macros to stringize a macro value */
#define TAROT_TO_XSTR(X) #X
#define TAROT_TO_STR(X) TAROT_TO_XSTR(X)

/* Tarot version */
#define TAROT_VERSION_MAJOR  0
#define TAROT_VERSION_MINOR  1
#define TAROT_VERSION_HOTFIX 0
#define TAROT_VERSION                    \
	TAROT_TO_STR(TAROT_VERSION_MAJOR)"." \
	TAROT_TO_STR(TAROT_VERSION_MINOR)"." \
	TAROT_TO_STR(TAROT_VERSION_HOTFIX)

/* The next lines are only available to tarot source files.
 * Tarot source files share "#define TAROT_SOURCE" as their first line.
 */
#ifdef TAROT_SOURCE

/* Returns the number of elements in the array x */
#define lengthof(x) (sizeof(x)/sizeof((x)[0]))

/* Declares the symbol unused (suppresses compiler warning) */
#define unused(Symbol) ((void)Symbol)

/* Swaps the value of type T of variables a and b */
#define swap(T, a, b) do { T tmp = a; a = b; b = tmp; } while (0)

/* Returns a pointer to the end of struct x */
#define end_of_struct(x) (void*)((char*)(x + 1))

#endif /* TAROT_SOURCE */

#endif /* TAROT_DEFINES_H */
