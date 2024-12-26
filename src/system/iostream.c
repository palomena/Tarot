#define TAROT_SOURCE
#include "tarot.h"

#define digits_in(type) ((sizeof(type)*CHAR_BIT-1)*28/93+1)

struct tarot_iostream {
	union {
		void *file;
		char *memory;
		struct tarot_string **string;
	} as;
	struct tarot_stream_position position;
	size_t offset;
	enum {
		TAROT_DUMBSTREAM,  /**< operates on nothing  */
		TAROT_FILESTREAM,  /**< operates on a file   */
		TAROT_MEMSTREAM,   /**< operates on a buffer */
		TAROT_STRINGSTREAM /**< operates on a string */
	} kind;
	enum tarot_stream_mode mode;
	int ch;               /**< current character */
	bool eof;             /**< end of file indicator */
	uint8_t indentation;  /**< indentation level */
	uint8_t indent_width; /**< number of characters to indent with */
	uint8_t tabsize;      /**< size of a tab in spaces */
};

static struct tarot_iostream streams[8];
static size_t stream_index;
static struct tarot_iostream* push_stream(void) {
	assert(stream_index < lengthof(streams));
	memset(&streams[stream_index], 0, sizeof(streams[stream_index]));
	return &streams[stream_index++];
}
static void pop_stream(struct tarot_iostream *stream) {
	assert(stream_index > 0);
	--stream_index;
	assert(&streams[stream_index] == stream);
	/* Assert fails if iostreams closed out of order */
}

TAROT_INLINE
void tarot_override_stream_position(struct tarot_iostream *stream, struct tarot_stream_position *position) {
	memcpy(&stream->position, position, sizeof(*position));
}

TAROT_INLINE
struct tarot_stream_position* tarot_fgetpos(struct tarot_iostream *stream) {
	return &stream->position;
}

TAROT_INLINE
size_t tarot_stream_offset(struct tarot_iostream *stream) {
	return stream->offset;
}

TAROT_INLINE
size_t tarot_stream_indentation(struct tarot_iostream *stream) {
	return stream->indentation;
}

TAROT_INLINE
size_t tarot_stream_indent_width(struct tarot_iostream *stream) {
	return stream->indent_width;
}

const char* tarot_stream_mode_string(enum tarot_stream_mode mode) {
	const char *names[] = { "output", "input" };
	assert(mode < lengthof(names));
	return names[mode];
}


static struct tarot_iostream tarot__stdout;
struct tarot_iostream* const tarot_stdout = &tarot__stdout;


static struct tarot_iostream tarot__stdin;
struct tarot_iostream* const tarot_stdin = &tarot__stdin;


static struct tarot_iostream tarot__stderr;
struct tarot_iostream* const tarot_stderr = &tarot__stderr;


void tarot_open_stdout(void *fileptr) {
	tarot_stdout->kind = TAROT_FILESTREAM;
	tarot_stdout->as.file = fileptr;
	tarot_stdout->mode = TAROT_OUTPUT;
	tarot_stdout->position.path = "<stdout>";
	tarot_stdout->position.line = 1;
	tarot_stdout->position.column = 1;
	tarot_stdout->indent_width = 4;
}

void tarot_open_stdin (void *fileptr) {
	tarot_stdin->kind = TAROT_FILESTREAM;
	tarot_stdin->as.file = fileptr;
	tarot_stdin->mode = TAROT_INPUT;
	tarot_stdin->position.path = "<stdin>";
	tarot_stdin->position.line = 1;
	tarot_stdin->position.column = 1;
	tarot_stdin->indent_width = 4;
}

void tarot_open_stderr(void *fileptr) {
	tarot_stderr->kind = TAROT_FILESTREAM;
	tarot_stderr->as.file = fileptr;
	tarot_stderr->mode = TAROT_OUTPUT;
	tarot_stderr->position.path = "<stderr>";
	tarot_stderr->position.line = 1;
	tarot_stderr->position.column = 1;
	tarot_stderr->indent_width = 4;
}


static const char* default_path_prefix = NULL;


static char* fullpath(const char *path) {
	static char buffer[TAROT_MAX_PATH];
	assert(path != NULL);
	memset(buffer, 0, sizeof(buffer));
	if (default_path_prefix != NULL) {
		strcpy(buffer, default_path_prefix);
	}
	strcat(buffer, path);
	return buffer;
}


static void raise_file_error(const char *path, enum tarot_stream_mode mode) {
	tarot_error(
		"Failed to open file \"%s%s%s\" in mode %s%s%s!",
		tarot_color_string(TAROT_COLOR_BLUE),
		path,
		tarot_color_string(TAROT_COLOR_RESET),
		tarot_color_string(TAROT_COLOR_YELLOW),
		tarot_stream_mode_string(mode),
		tarot_color_string(TAROT_COLOR_RESET)
	);
}


struct tarot_iostream* tarot_fopen(
	const char *path,
	enum tarot_stream_mode mode
) {
	struct tarot_iostream *stream = NULL;
	static const char* legacy_mode[2] = {"w", "r"};
	void *fileptr = tarot_platform.fopen(fullpath(path), legacy_mode[mode]);
	if (fileptr != NULL) {
		stream = push_stream();
		stream->kind = TAROT_FILESTREAM;
		stream->as.file = fileptr;
		stream->mode = mode;
		stream->position.path = path;
		stream->position.line = 1;
		stream->position.column = 1;
	} else {
		raise_file_error(path, mode);
	}
	return stream;
}


struct tarot_iostream* tarot_fmemopen(
	char *memory,
	enum tarot_stream_mode mode
) {
	struct tarot_iostream *stream = push_stream();
	assert(memory != NULL);
	stream->kind = TAROT_MEMSTREAM;
	stream->as.memory = memory;
	stream->mode = mode;
	stream->position.path = "memory";
	stream->position.line = 1;
	stream->position.column = 1;
	return stream;
}


struct tarot_iostream* tarot_fstropen(
	struct tarot_string **stringptr,
	enum tarot_stream_mode mode
) {
	struct tarot_iostream *stream = push_stream();
	assert(stringptr != NULL);
	stream->kind = TAROT_STRINGSTREAM;
	stream->as.string = stringptr;
	stream->mode = mode;
	if (mode == TAROT_OUTPUT) {
		stream->offset = tarot_string_length(*stringptr);
	} else {
		stream->offset = 0;
	}
	stream->position.path = "string";
	stream->position.line = 1;
	stream->position.column = 1;
	return stream;
}


struct tarot_iostream* tarot_fdumbopen(enum tarot_stream_mode mode) {
	static struct tarot_iostream stream;
	memset(&stream, 0, sizeof(stream));
	stream.kind = TAROT_DUMBSTREAM;
	stream.mode = mode;
	stream.position.path = "<dummy>";
	stream.position.line = 1;
	stream.position.column = 1;
	return &stream;
}


void tarot_fclose(struct tarot_iostream *stream) {
	assert(stream != NULL);
	if(
		(stream == tarot_stdout) or
		(stream == tarot_stderr) or
		(stream == tarot_stdin)
	) {
		return;
	}

	if ((stream->kind == TAROT_FILESTREAM)) {
		tarot_platform.fclose(stream->as.file);
	}

	if (stream->kind != TAROT_DUMBSTREAM) {
		pop_stream(stream);
	}
}

TAROT_INLINE
bool tarot_feof(struct tarot_iostream *stream) {
	return stream->eof;
}

TAROT_INLINE
int tarot_fcurr(struct tarot_iostream *stream) {
	if (tarot_stream_offset(stream) == 0) {
		tarot_fgetc(stream);
	}
	return stream->ch;
}


int tarot_fgetc(struct tarot_iostream *stream) {
	assert(stream != NULL);
	assert(stream->mode == TAROT_INPUT);
	if (stream->kind == TAROT_FILESTREAM) {
		stream->ch = tarot_platform.fgetc(stream->as.file);
		if (stream->ch < 0) {
			stream->eof = true;
		}
	} else if (stream->kind == TAROT_MEMSTREAM) {
		stream->ch = *stream->as.memory++;
		if (stream->ch == '\0') {
			stream->eof = true;
		}
	} else if (stream->kind == TAROT_STRINGSTREAM) {
		if (stream->offset < tarot_string_length(*stream->as.string)) {
			stream->ch = tarot_string_char(*stream->as.string, stream->offset);
		} else {
			stream->ch = '\0';
			stream->eof = true;
		}
	}

	stream->offset++;
	stream->position.column++;
	if (stream->ch == '\n') {
		stream->position.line++;
		stream->position.column = 1;
		stream->indentation = 0;
	} else if (stream->ch == '\t') {
		stream->position.column += stream->tabsize - 1;
		stream->indentation++;
	}

	return stream->ch;
}

static void print_char(struct tarot_iostream *stream, int ch) {
	assert(stream != NULL);
	if (stream->kind == TAROT_FILESTREAM) {
		stream->ch = tarot_platform.fputc(ch, stream->as.file);
	} else if (stream->kind == TAROT_MEMSTREAM) {
		stream->ch = *stream->as.memory++ = ch;
	} else if (stream->kind == TAROT_STRINGSTREAM) {
		tarot_string_append(stream->as.string, "%c", ch);
		stream->ch = ch;
	}

	stream->offset++;
	stream->position.column++;
	if (stream->ch == '\n') {
		stream->position.line++;
		stream->position.column = 1;
	} else if (stream->ch == '\t') {
		stream->position.column += stream->tabsize - 1;
	}
}


int tarot_fputc(struct tarot_iostream *stream, int ch) {
	assert(stream != NULL);
	assert(stream->mode == TAROT_OUTPUT);
	if (stream->position.column == 1) {
		unsigned int indent = stream->indent_width * stream->indentation;
		while (indent > 0) {
			print_char(stream, ' ');
			indent--;
		}
	}

	print_char(stream, ch);
	return stream->ch;
}


void tarot_fputs(struct tarot_iostream *stream, const char *text) {
	assert(stream != NULL);
	assert(text != NULL);
	while (*text) {
		tarot_fputc(stream, *text++);
	}
}


size_t tarot_fread(
	void *buffer,
	struct tarot_iostream *stream,
	size_t size
) {
	size_t bytes_read;
	assert(buffer != NULL);
	assert(stream != NULL);
	assert(stream->mode == TAROT_INPUT);
	for (bytes_read = 0; (bytes_read < size) and !stream->eof; bytes_read++) {
		((char*)buffer)[bytes_read] = tarot_fgetc(stream);
	}
	return bytes_read;
}


size_t tarot_fwrite(
	struct tarot_iostream *stream,
	const void *buffer,
	size_t size
) {
	size_t bytes_written;
	assert(stream != NULL);
	assert(buffer != NULL);
	assert(stream->mode == TAROT_OUTPUT);
	for (bytes_written = 0; bytes_written < size; bytes_written++) {
		tarot_fputc(stream, ((const char*)buffer)[bytes_written]);
	}
	return bytes_written;
}


void tarot_hexdump(
	struct tarot_iostream *stream,
	int seperator,
	const void *data,
	size_t length
) {
	const unsigned char *strptr = data;
	unsigned int c;
	size_t i;
	for (i = 0, c = 1; i < length; i++, c++) {
		tarot_fprintf(stream, "0x%*x", 2, strptr[i]);
		if ((seperator != 0) and ((i+1) < length)) {
			tarot_fputc(stream, seperator);
		}
		tarot_fputc(stream, ' ');
		if (c >= 8) {
			tarot_fputc(stream, '\n');
			c = 0;
		}
	}
	if (c > 1) {
		tarot_fputc(stream, '\n');
	}
}

struct buffer {
	uint8_t *data;
	size_t index;
	size_t size;
};

static void append_byte(struct buffer *buffer, uint8_t byte) {
	if (buffer->index >= buffer->size) {
		buffer->size += 10 + 2 * buffer->size;
		buffer->data = tarot_realloc(buffer->data, buffer->size);
	}
	buffer->data[buffer->index++] = byte;
}

void* tarot_read_file(const char *path, size_t *size) {
	void *contents = NULL;
	struct tarot_iostream *stream = tarot_fopen(path, TAROT_INPUT);
	if (stream != NULL) {
		char ch;
		struct buffer buffer;
		memset(&buffer, 0, sizeof(buffer));
		while (not tarot_feof(stream)) {
			ch = tarot_fgetc(stream);
			append_byte(&buffer, ch);
		}
		ch = '\0';
		append_byte(&buffer, ch);
		contents = buffer.data;
		if (size != NULL) {
			*size = buffer.index;
		}
		tarot_fclose(stream);
	}
	return contents;
}

int tarot_write_to_file(
	const char *path,
	const void *buffer,
	size_t size
) {
	int result = -1;
	struct tarot_iostream *stream = tarot_fopen(path, TAROT_OUTPUT);
	if (stream != NULL) {
		result = tarot_fwrite(stream, buffer, size) == size;
		tarot_fclose(stream);
	}
	return result;
}

void tarot_print(const char *text) {
	assert(text != NULL);
	tarot_fputs(tarot_stdout, text);
}

TAROT_INLINE
void tarot_newline(struct tarot_iostream *stream) {
	if (stream->position.column > 1) {
		tarot_fputc(stream, '\n');
	}
}

void tarot_println(const char *text) {
	assert(text != NULL);
	tarot_print(text);
	tarot_newline(tarot_stdout);
}

size_t tarot_printf(const char *format, ...) {
	size_t bytes_written;
	va_list ap;
	assert(format != NULL);
	va_start(ap, format);
	bytes_written = tarot_vprintf(format, &ap);
	va_end(ap);
	return bytes_written;
}

size_t tarot_fprintf(
	struct tarot_iostream *stream,
	const char *format,
	...
) {
	size_t bytes_written;
	va_list ap;
	assert(stream != NULL);
	assert(format != NULL);
	va_start(ap, format);
	bytes_written = tarot_vfprintf(stream, format, &ap);
	va_end(ap);
	return bytes_written;
}

size_t tarot_vprintf(const char *format, va_list *ap) {
	size_t bytes_written;
	assert(format != NULL);
	bytes_written = tarot_vfprintf(tarot_stdout, format, ap);
	return bytes_written;
}

static void print_unsigned_integer(
	struct tarot_iostream *stream,
	size_t number,
	unsigned int base,
	unsigned int width
) {
	static const char hex[] = "0123456789ABCDEF";
	char buffer[digits_in(size_t) + 1];
	unsigned int index = lengthof(buffer);
	unsigned int count = 0;
	assert(stream != NULL);
	buffer[--index] = '\0';
	do {
		buffer[--index] = hex[number % base];
		number /= base;
		count++;
	} while ((number > 0) and (index > 0));
	while ((width > count) and (index > 0)) {
		buffer[--index] = '0';
		width--;
	}
	tarot_fputs(stream, buffer + index);
}

static void print_integer(
	struct tarot_iostream *stream,
	long int number,
	unsigned int base
) {
	size_t unsigned_number;
	assert(stream != NULL);
	if (number < 0) {
		unsigned_number = -number;
		tarot_fputc(stream, '-');
	} else {
		unsigned_number = number;
	}
	print_unsigned_integer(stream, unsigned_number, base, 0);
}

/* Calculates 10^n */
static long int ipow10(long int n) {
	long int result = 1;
	if (n > 0) {
		for (result = 10; n > 1; n--) {
			result *= 10;
		}
	}
	return result;
}

static void print_float(
	struct tarot_iostream *stream,
	double number
) {
	double factor;
	long int lead;
	long int fraction;
	int n;
	unsigned int num_zeros;
	assert(stream != NULL);
	factor = (double)ipow10(digits_in(int) - 1);
	lead = (long)number;
	fraction = (size_t)fabs((number - (double)lead) * factor);
	num_zeros = digits_in(int) - 1;
	if (fraction == 0) {
		num_zeros = 1;
	}
	for (n = fraction; n > 0; n /= 10) {
		num_zeros--;
	}
	while ((fraction > 0) and (fraction % 10 == 0)) {
		fraction /= 10;
	}
	print_integer(stream, lead, 10);
	tarot_fputc(stream, '.');
	while (num_zeros-- > 0) {
		tarot_fputc(stream, '0');
	}
	print_unsigned_integer(stream, fraction, 10, 0);
}

void tarot_print_float(
	struct tarot_iostream *stream,
	double value
) {
	print_float(stream, value);
}

void tarot_print_short(
	struct tarot_iostream *stream,
	int32_t value
) {
	print_integer(stream, value, 10);
}

static const char* print_formatted(
	struct tarot_iostream *stream,
	const char *format,
	va_list *ap,
	unsigned int width
) {
	int ch;
	assert(stream != NULL);
	assert(format != NULL);
	assert(ap != NULL);
	switch((ch = *format++)) {
		default:
			tarot_fputc(stream, ch);
			break;
		case '*':
			width = va_arg(*ap, unsigned int);
			format = print_formatted(stream, format, ap, width);
			break;
		case 'b':
			print_unsigned_integer(stream, va_arg(*ap, int), 2, 0);
			break;
		case 'c':
			tarot_fputc(stream, va_arg(*ap, int));
			break;
		case 'd':
			print_integer(stream, va_arg(*ap, int), 10);
			break;
		case 'f':
			print_float(stream, va_arg(*ap, double));
			break;
		case 's':
			tarot_fputs(stream, va_arg(*ap, void*));
			break;
		case 'p':
			print_unsigned_integer(stream, (size_t)va_arg(*ap, void*), 16, 0);
			break;
		case 'u':
			print_unsigned_integer(stream, va_arg(*ap, unsigned int), 10, width);
			break;
		case 'x':
			print_unsigned_integer(stream, va_arg(*ap, int), 16, width);
			break;
		case 'z':
			if (*format == 'u') {
				format++;
				print_unsigned_integer(stream, va_arg(*ap, size_t), 10, width);
			} else {
				print_integer(stream, va_arg(*ap, long), 10);
			}
			break;
	}
	return format;
}

size_t tarot_vfprintf(
	struct tarot_iostream *stream,
	const char *format,
	va_list *ap
) {
	size_t bytes_written;
	int ch;
	assert(stream != NULL);
	assert(format != NULL);
	bytes_written = stream->offset;
	while ((ch = *format++)) {
		if (ch == '%') {
			format = print_formatted(stream, format, ap, 0);
		} else {
			tarot_fputc(stream, ch);
		}
	}
	bytes_written = stream->offset - bytes_written;
	return bytes_written;
}

size_t tarot_fstrlen(const char *format, ...) {
	size_t bytes_required;
	va_list ap;
	assert(format != NULL);
	va_start(ap, format);
	bytes_required = tarot_vfstrlen(format, &ap);
	va_end(ap);
	return bytes_required;
}

size_t tarot_vfstrlen(const char *format, va_list *ap) {
	struct tarot_iostream *stream = tarot_fdumbopen(TAROT_OUTPUT);
	size_t bytes_required = tarot_vfprintf(stream, format, ap);
	tarot_fclose(stream);
	return bytes_required;
}

static bool colored_output_enabled = false;

void tarot_enable_colored_output(bool enable) {
	colored_output_enabled = enable;
}

const char* tarot_color_string(enum tarot_color_code color) {
	static const char* color_sequences[] = {
		"\033[1m",
		"\033[0;30m", "\033[0;31m", "\033[0;32m",
		"\033[0;33m", "\033[0;34m", "\033[0;35m",
		"\033[0;36m", "\033[0;37m", "\033[0m"
	};
	const char *sequence = "";
	if (colored_output_enabled) {
		assert(color < lengthof(color_sequences));
		sequence = color_sequences[color];
	}

	return sequence;
}

void tarot_format(
	struct tarot_iostream *stream,
	enum tarot_color_code code
) {
	if (colored_output_enabled) {
		tarot_fputs(stream, tarot_color_string(code));
	}
}

void tarot_set_path(const char *path) {
	default_path_prefix = path;
	tarot_log("Set default path prefix to \"%s\"", path);
}

void tarot_indent(struct tarot_iostream *stream, int depth) {
	assert((int)stream->indentation + depth >= 0);
	assert((int)stream->indentation + depth < UCHAR_MAX);
	stream->indentation += depth;
}

void tarot_set_indent_width(
	struct tarot_iostream *stream,
	unsigned int width
) {
	assert(stream != NULL);
	assert(stream->mode == TAROT_OUTPUT);
	stream->indent_width = width;
}

const char* tarot_getline(struct tarot_iostream *stream) {
	static char buffer[128];
	static size_t buffersize = 0;
	size_t i;
	if (buffersize == 0) {
		buffersize = lengthof(buffer);
	}
	for (i = 0; i < buffersize; i++) {
		int ch = tarot_fgetc(stream);
		if (ch == '\n') {
			break;
		}
		buffer[i] = ch;
	}
	buffer[i] = '\0';
	return buffer;
}
