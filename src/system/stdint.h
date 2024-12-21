#ifndef TAROT_STDINT_H
#define TAROT_STDINT_H

#include <limits.h>

/* 8 bit integers */
#ifndef TAROT_INT8
#if CHAR_BIT == 8 && SCHAR_MAX == 0x7F && UCHAR_MAX == 0xFF
#define TAROT_INT8 char
#else
#error "Unsupported platform: Invalid width of fundamental type tarot_int8."
#endif
#endif /* ifndef TAROT_INT8 */

/* 16 bit integers */
#ifndef TAROT_INT16
#if USHRT_MAX == 0xFFFF
#define TAROT_INT16 short
#elif UINT_MAX == 0xFFFF
#define TAROT_INT16 int
#elif ULLONG_MAX == 0xFFFF
#define TAROT_INT16 long
#else
#error "Unsupported platform: Invalid width of fundamental type tarot_int16."
#endif
#endif /* ifndef TAROT_INT16 */

/* 32 bit integers */
#ifndef TAROT_INT32
#if USHRT_MAX == 0xFFFFFFFF
#define TAROT_INT32 short
#elif UINT_MAX == 0xFFFFFFFF
#define TAROT_INT32 int
#elif ULLONG_MAX == 0xFFFFFFFF
#define TAROT_INT32 long
#else
#error "Unsupported platform: Invalid width of fundamental type tarot_int32."
#endif
#endif /* ifndef TAROT_INT32 */

#ifndef INT8_MAX
/*A signed 8-bit integer type with range [-128, 127] */
typedef signed TAROT_INT8 int8_t;
#define INT8_MIN (-128)
#define INT8_MAX (127)
#endif

#ifndef UINT8_MAX
/* An unsigned 8-bit integer type with range [0, 255] */
typedef unsigned TAROT_INT8 uint8_t;
# define UINT8_MAX (255)
#endif

#ifndef INT16_MAX
/* A signed 16-bit integer type with range [-32.768, 32.767] */
typedef signed TAROT_INT16 int16_t;
#define INT16_MAX (32767)
#define INT16_MIN (-32767-1)
#endif

#ifndef UINT16_MAX
/* An unsigned 16-bit integer type with range [0, 65.535] */
typedef unsigned TAROT_INT16 uint16_t;
#define UINT16_MAX (65535)
#endif

#ifndef INT32_MAX
/* A signed 32-bit integer type; range [-2.147.483.648, 2.147.483.647] */
typedef signed TAROT_INT32 int32_t;
#define INT32_MAX (2147483647)
#define INT32_MIN (-2147483647-1)
#endif

#ifndef UINT32_MAX
/* An unsigned 32-bit integer type; range [0, 4.294.967.295] */
typedef unsigned TAROT_INT32 uint32_t;
#define UINT32_MAX (4294967295U)
#endif

#undef TAROT_INT8
#undef TAROT_INT16
#undef TAROT_INT32

#endif /* TAROT_STDINT_H */
