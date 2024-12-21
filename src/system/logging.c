#define TAROT_SOURCE
#include "tarot.h"

static bool logging_is_enabled = false;
static bool debugging_is_enabled = false;

TAROT_INLINE
void tarot_enable_logging(bool enable) {
	assert(tarot_is_initialized());
	logging_is_enabled = enable;
}

TAROT_INLINE
void tarot_enable_debugging(bool enable) {
	debugging_is_enabled = enable;
}

void tarot_log(const char *format, ...) {
	if (logging_is_enabled) {
		va_list ap;
		assert(format != NULL);
		tarot_printf("[%sLOG%s] ",
			tarot_color_string(TAROT_COLOR_CYAN),
			tarot_color_string(TAROT_COLOR_RESET)
		);
		va_start(ap, format);
		tarot_vprintf(format, &ap);
		va_end(ap);
		tarot_newline(tarot_stdout);
	}
}

void tarot_debug(const char *format, ...) {
#ifdef DEBUG
	if (debugging_is_enabled and logging_is_enabled) {
		va_list ap;
		assert(format != NULL);
		tarot_printf("[%sDEBUG%s] ",
			tarot_color_string(TAROT_COLOR_YELLOW),
			tarot_color_string(TAROT_COLOR_RESET)
		);
		va_start(ap, format);
		tarot_vprintf(format, &ap);
		va_end(ap);
		tarot_newline(tarot_stdout);
	}
#else
	unused(format);
#endif /* DEBUG */
}
