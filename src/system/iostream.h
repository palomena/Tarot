#ifndef TAROT_IOSTREAM_H
#define TAROT_IOSTREAM_H

#include "defines.h"

/* iostreams provide an abstract interface around files and strings */
struct tarot_iostream;

/**
 * The stream position path should ideally be a constant string as it is
 * only valid for the duration the iostream is open. Upon fclose it may
 * go out of scope. If objects are created during that time that incorporate
 * the path into their data structure, they should duplicate the path string
 * via strdup for each instance or create a new shared duplicate for all
 * instances. Copying the path verbatim when it not a constant string is risky.
 */
struct tarot_stream_position {
	const char *path;
	size_t line;
	size_t column;
};

extern void tarot_override_stream_position(struct tarot_iostream *stream, struct tarot_stream_position *position);

extern struct tarot_stream_position* tarot_fgetpos(struct tarot_iostream *stream);
extern size_t tarot_stream_offset(struct tarot_iostream *stream);
extern size_t tarot_stream_indentation(struct tarot_iostream *stream);
extern size_t tarot_stream_indent_width(struct tarot_iostream *stream);

/* Input / Output mode of the iostream. Only one mode is valid at a time */
enum tarot_stream_mode {
	TAROT_OUTPUT,
	TAROT_INPUT
};

extern const char* tarot_stream_mode_string(enum tarot_stream_mode mode);

/* An iostream pointer initialized with the platforms stdout handle */
extern struct tarot_iostream* const tarot_stdout;

/* An iostream pointer initialized with the platforms stdin handle */
extern struct tarot_iostream* const tarot_stdin;

/* An iostream pointer initialized with the platforms stderr handle */
extern struct tarot_iostream* const tarot_stderr;

extern void tarot_open_stdout(void *fileptr);
extern void tarot_open_stdin (void *fileptr);
extern void tarot_open_stderr(void *fileptr);

/* Opens a file */
extern struct tarot_iostream* tarot_fopen(
	const char *path,
	enum tarot_stream_mode mode
);

/* Opens a c-string */
extern struct tarot_iostream* tarot_fmemopen(
	char *memory,
	enum tarot_stream_mode mode
);

/* Opens a real string */
extern struct tarot_iostream* tarot_fstropen(
	struct tarot_string **stringptr,
	enum tarot_stream_mode mode
);

/* Opens a dummy stream */
extern struct tarot_iostream* tarot_fdumbopen(enum tarot_stream_mode mode);

extern void tarot_fclose(struct tarot_iostream *stream);
extern bool tarot_feof(struct tarot_iostream *stream);
extern int tarot_fcurr(struct tarot_iostream *stream);
extern int tarot_fgetc(struct tarot_iostream *stream);
extern int tarot_fputc(struct tarot_iostream *stream, int ch);
extern void tarot_fputs(struct tarot_iostream *stream, const char *text);
extern size_t tarot_fread(
	void *buffer,
	struct tarot_iostream *stream,
	size_t size
);
extern size_t tarot_fwrite(
	struct tarot_iostream *stream,
	const void *buffer,
	size_t size
);
extern void tarot_hexdump(
	struct tarot_iostream *stream,
	int seperator,
	const void *data,
	size_t length
);

extern void* tarot_read_file(const char *path, size_t *size);
extern int tarot_write_to_file(
	const char *path,
	const void *buffer,
	size_t size
);
extern void tarot_print(const char *text);
extern void tarot_println(const char *text);
extern void tarot_newline(struct tarot_iostream *stream);
extern size_t tarot_printf(const char *format, ...);
extern size_t tarot_fprintf(
	struct tarot_iostream *stream,
	const char *format,
	...
);
extern size_t tarot_vprintf(const char *format, va_list *ap);
extern size_t tarot_vfprintf(
	struct tarot_iostream *stream,
	const char *format,
	va_list *ap
);

extern size_t tarot_fstrlen(const char *format, ...);
extern size_t tarot_vfstrlen(const char *format, va_list *ap);

enum tarot_color_code {
	TAROT_COLOR_BOLD,
	TAROT_COLOR_BLACK,
	TAROT_COLOR_RED,
	TAROT_COLOR_GREEN,
	TAROT_COLOR_YELLOW,
	TAROT_COLOR_BLUE,
	TAROT_COLOR_PURPLE,
	TAROT_COLOR_CYAN,
	TAROT_COLOR_WHITE,
	TAROT_COLOR_RESET
};

extern void tarot_enable_colored_output(bool enable);
extern const char* tarot_color_string(enum tarot_color_code color);

extern void tarot_format(
	struct tarot_iostream *stream,
	enum tarot_color_code code
);

#ifndef TAROT_MAX_PATH
/** Maximim path length */
#define TAROT_MAX_PATH 260
#endif

/******************************************************************************
 * @brief Sets the default path prefix for opening files
 *****************************************************************************/
extern void tarot_set_path(const char *path);

/******************************************************************************
 * @brief Indents the indentation level of @p stream by @p depth relative
 * to the current indentation level
 *****************************************************************************/
extern void tarot_indent(struct tarot_iostream *stream, int depth);
extern void tarot_set_indent_width(
	struct tarot_iostream *stream,
	unsigned int width
);

extern const char* tarot_getline(struct tarot_iostream *stream);

#endif /* TAROT_IOSTREAM_H */
