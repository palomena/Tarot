#ifndef TAROT_ERROR_H
#define TAROT_ERROR_H

#include "defines.h"

/* Forward declaration */
struct tarot_stream_position;

extern void tarot_register_error_handler(void (*f)(void *data), void *data);

extern void tarot_begin_error(struct tarot_stream_position *position);
extern void tarot_end_error(void);

extern void tarot_error(const char *format, ...);
extern void tarot_error_at(
	struct tarot_stream_position *position,
	const char *format, ...
);

extern void tarot_warning(const char *format, ...);
extern void tarot_warning_at(
	struct tarot_stream_position *position,
	const char *format, ...
);

extern void tarot_sourcecode_error(const char *file, int line, const char *format, ...);

extern void tarot_enable_warnings(bool enable);
extern size_t tarot_num_errors(void);
extern size_t tarot_num_warnings(void);

extern void tarot_error(const char *format, ...);

#endif /* TAROT_ERROR_H */
