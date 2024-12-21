#define TAROT_SOURCE
#include "tarot.h"

static bool warnings_enabled = true;
static size_t num_errors = 0;
static size_t num_warnings = 0;

TAROT_INLINE
void tarot_enable_warnings(bool enable) {
	warnings_enabled = enable;
}

TAROT_INLINE
size_t tarot_num_errors(void) {
	return num_errors;
}

TAROT_INLINE
size_t tarot_num_warnings(void) {
	return num_warnings;
}

static void (*error_handler)(void *data) = NULL;
static void *error_data = NULL;

void tarot_register_error_handler(void (*f)(void *data), void *data) {
	error_handler = f;
	error_data = data;
}

static void invoke_error_handler(void) {
	if (error_handler != NULL) {
		error_handler(error_data);
	}
}

static void print_position(struct tarot_stream_position *pos) {
	assert(pos != NULL);
	tarot_fprintf(tarot_stderr,
		"In file \"%s%s%s%s\" at line %d, column %d\n",
		tarot_color_string(TAROT_COLOR_BLUE),
		tarot_color_string(TAROT_COLOR_BOLD),
		pos->path,
		tarot_color_string(TAROT_COLOR_RESET),
		pos->line, pos->column
	);
}

void tarot_begin_error(struct tarot_stream_position *position) {
	num_errors++;
	tarot_fprintf(tarot_stderr,
		"[%s%sERROR%s : %d] ",
		tarot_color_string(TAROT_COLOR_RED),
		tarot_color_string(TAROT_COLOR_BOLD),
		tarot_color_string(TAROT_COLOR_RESET),
		num_errors
	);
	if (position != NULL) {
		print_position(position);
	}
	tarot_indent(tarot_stderr, 1);
}

void tarot_end_error(void) {
	tarot_indent(tarot_stderr, -1);
	tarot_newline(tarot_stderr);
}

void tarot_error(const char *format, ...) {
	va_list ap;
	assert(format != NULL);
	tarot_begin_error(NULL);
	va_start(ap, format);
	tarot_vfprintf(tarot_stderr, format, &ap);
	va_end(ap);
	tarot_end_error();
	invoke_error_handler();
}

void tarot_error_at(
	struct tarot_stream_position *position,
	const char *format, ...
) {
	va_list ap;
	assert(position != NULL);
	assert(format != NULL);
	tarot_begin_error(position);
	va_start(ap, format);
	tarot_vfprintf(tarot_stderr, format, &ap);
	va_end(ap);
	tarot_end_error();
	invoke_error_handler();
}

void tarot_warning(const char *format, ...) {
	if (warnings_enabled) {
		va_list ap;
		assert(format != NULL);
		num_warnings++;
		tarot_fprintf(
			tarot_stderr,
			"[%s%sWARNING%s : %d] ",
			tarot_color_string(TAROT_COLOR_YELLOW),
			tarot_color_string(TAROT_COLOR_BOLD),
			tarot_color_string(TAROT_COLOR_RESET),
			num_warnings
		);
		va_start(ap, format);
		tarot_indent(tarot_stderr, 1);
		tarot_vfprintf(tarot_stderr, format, &ap);
		tarot_indent(tarot_stderr, -1);
		va_end(ap);
		tarot_fputc(tarot_stderr, '\n');
	}
}

void tarot_warning_at(
	struct tarot_stream_position *position,
	const char *format, ...
) {
	if (warnings_enabled) {
		va_list ap;
		assert(position != NULL);
		assert(format != NULL);
		num_warnings++;
		tarot_fprintf(
			tarot_stderr,
			"[%s%sWARNING%s : %d] ",
			tarot_color_string(TAROT_COLOR_YELLOW),
			tarot_color_string(TAROT_COLOR_BOLD),
			tarot_color_string(TAROT_COLOR_RESET),
			num_warnings
		);
		print_position(position);
		va_start(ap, format);
		tarot_indent(tarot_stderr, 1);
		tarot_vfprintf(tarot_stderr, format, &ap);
		tarot_indent(tarot_stderr, -1);
		va_end(ap);
		tarot_newline(tarot_stderr);
	}
}

void tarot_raise_assert(
	bool ok,
	const char *condition,
	const char *file,
	int line,
	const char *format, ...
) {
	if (not ok) {
		va_list ap;
		tarot_begin_error(NULL);
		tarot_fputs(tarot_stderr, "Assertion failed: ");
		tarot_fputs(tarot_stderr, condition);
		tarot_fprintf(tarot_stderr, " (%s:%d) ", file, line);
		va_start(ap, format);
		tarot_vfprintf(tarot_stderr, format, &ap);
		va_end(ap);
		tarot_end_error();
		tarot_abort();
	}
}

void tarot__abort(const char *file, int line) {
	/* abort function name clashes with stdc abort name at linker level */
	tarot_fprintf(tarot_stderr, "%s:%d\n", file, line);
	tarot_platform.abort();
}
