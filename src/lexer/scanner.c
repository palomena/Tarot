#define TAROT_SOURCE
#include "tarot.h"

static size_t max_line_length = 80;
static size_t max_indentation = 4;

TAROT_INLINE
void tarot_set_max_line_length(size_t value) {
	max_line_length = value;
}

TAROT_INLINE
void tarot_set_max_indentation(size_t value) {
	max_indentation = value;
}

static bool line_is_too_long(struct tarot_iostream *stream) {
	return tarot_fgetpos(stream)->column == max_line_length;
}

static bool line_is_overindented(struct tarot_iostream *stream) {
	return tarot_fcurr(stream) == '\t' and tarot_stream_indentation(stream) > max_indentation;
}

static bool line_has_mixed_indentation(struct tarot_iostream *stream) {
	return (
		tarot_fcurr(stream) == '\t' and
		tarot_fgetpos(stream)->column > (tarot_stream_indentation(stream) + 1)
	);
}

static void perform_suggestions(struct tarot_iostream *stream) {
	if (line_is_too_long(stream)) {
		tarot_warning_at(tarot_fgetpos(stream),
			"Line exceeds recommended length limit of %d characters.\n"
			"Consider splitting the line into multiple smaller ones, as\n"
			"long lines tend to make sourcecode harder to read.\n",
			max_line_length
		);
	}
	if (line_has_mixed_indentation(stream)) {
		tarot_warning_at(tarot_fgetpos(stream),
			"Tab mixed with regular characters.\n"
			"Tabs are used to indent lines and therefore should "
			"only ever appear as the first characters of a line.\n"
			"Using tabs anywhere else within a line will lead to "
			"alignment issues."
		);
	}
	if (line_is_overindented(stream)) {
		tarot_warning_at(tarot_fgetpos(stream),
			"Overindented line - the line is indented %d times!\n"
			"This exceeds the maximum recommended indentation of %d.\n"
			"Consider putting the affected section into a new function.",
			tarot_stream_indentation(stream),
			max_indentation
		);
	}
}

static int advance(struct tarot_iostream *stream) {
	perform_suggestions(stream);
	return tarot_fgetc(stream);
}

static bool match(struct tarot_iostream *stream, int ch) {
	bool result = tarot_fcurr(stream) == ch;
	if (result) {
		advance(stream);
	}
	return result;
}

static bool is_intchar(int ch) {
	return isdigit(ch) or (isxdigit(ch) and isupper(ch)) or (ch == '_');
}

static void raise_seperator_error(
	struct tarot_iostream *stream,
	const char *at
) {
	assert(stream != NULL);
	assert(at != NULL);
	tarot_error_at(
		tarot_fgetpos(stream),
		"The visual underscore seperator is only permitted between "
		"digits. It is not allowed at the %s of a series of digits.",
		at
	);
}

static size_t read_digits(
	struct tarot_string **string,
	struct tarot_iostream *stream
) {
	size_t length = 0;
	if (isdigit(tarot_fcurr(stream))) {
		char last_character = 0;
		while (is_intchar(tarot_fcurr(stream))) {
			if (tarot_fcurr(stream) != '_') {
				tarot_string_append(string, "%c", tarot_fcurr(stream));
				length++;
			}
			last_character = tarot_fcurr(stream);
			advance(stream);
		}
		if (last_character == '_') {
			raise_seperator_error(stream, "end");
		}
	} else if (tarot_fcurr(stream) == '_') {
		raise_seperator_error(stream, "start");
	}
	return length;
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

static void tokenize_number_as_decimal(
	struct tarot_iostream *stream,
	struct tarot_string **string,
	struct tarot_token *token
) {
	long int length = 0;
	length = read_digits(string, stream);
	if (length != 0)  {
		tarot_integer *numerator;
		tarot_integer *denominator;
		long int decile = ipow10(length);
		numerator = tarot_create_integer_from_string(*string, 10);
		denominator = tarot_create_integer_from_short(decile);
		token->value.Rational = tarot_create_rational_from_integers(
			numerator, denominator
		);
		if (match(stream, 'f')) {
			double value = tarot_rational_to_float(token->value.Rational);
			tarot_free_rational(token->value.Rational);
			token->value.Float = value;
			token->kind = TAROT_TOK_FLOAT;
		} else {
			token->kind = TAROT_TOK_RATIONAL;
		}
		tarot_free_integer(numerator);
		tarot_free_integer(denominator);
	} else {
		tarot_error_at(&token->position, "Invalid fractional part");
	}
}

static void tokenize_number_as_rational(
	struct tarot_iostream *stream,
	struct tarot_string **string,
	struct tarot_token *token
) {
	tarot_integer *numerator = tarot_create_integer_from_string(*string, 10);
	tarot_clear_string(*string);
	if (read_digits(string, stream) == 0) {
		tarot_error_at(&token->position, "Invalid denominator");
	} else {
		tarot_integer *denominator;
		denominator = tarot_create_integer_from_string(*string, 10);
		token->value.Rational = tarot_create_rational_from_integers(
			numerator, denominator
		);
		token->kind = TAROT_TOK_RATIONAL;
		tarot_free_integer(denominator);
	}
	tarot_free_integer(numerator);
}

static int read_numbase(struct tarot_iostream *stream) {
	int base = 10;
	if (match(stream, ':')) {
		char buffer[3] = {0};
		unsigned int i;
		for (i = 0; i < 2; i++) {
			if (not isdigit(tarot_fcurr(stream))) {
				break;
			}
			buffer[i] = tarot_fcurr(stream);
			advance(stream);
		}
		if (i > 0) {
			buffer[2] = '\0';
			base = strtol(buffer, NULL, 10);
		}
	}
	return base;
}

static void tokenize_number(
	struct tarot_iostream *stream,
	struct tarot_token *token
) {
	struct tarot_string *string = tarot_create_string(NULL);
	read_digits(&string, stream);
	if (match(stream, '.')) {
		tokenize_number_as_decimal(stream, &string, token);
	} else if (match(stream, '|')) {
		tokenize_number_as_rational(stream, &string, token);
	} else {
		int base = 10;
		token->kind = TAROT_TOK_INTEGER;
		base = read_numbase(stream);
		token->value.Integer = tarot_create_integer_from_string(string, base);
	}
	tarot_free(string);
}

static bool tarot_match_keyword(
	struct tarot_string *string,
	enum tarot_token_kind *type
) {
	bool result = false;
	enum tarot_token_kind it;
	for (it = TAROT_KEYWORD_START+1; it < TAROT_KEYWORD_END; it++) {
		if (tarot_match_string(string, tarot_token_string(it))) {
			result = true;
			if (type != NULL) {
				*type = it;
			}
			break;
		}
	}
	return result;
}

static void tokenize_identifier(
	struct tarot_iostream *stream,
	struct tarot_token *token,
	int first_character
) {
	struct tarot_string *string = tarot_create_string(NULL);
	enum tarot_token_kind keyword;
	tarot_string_append(&string, "%c", first_character);
	while (isalnum(tarot_fcurr(stream)) or (tarot_fcurr(stream) == '_')) {
		/* If tarot_feof(stream), the while condition evaluates to false */
		tarot_string_append(&string, "%c", tarot_fcurr(stream));
		advance(stream);
	}
	if (tarot_match_keyword(string, &keyword)) {
		token->kind = keyword;
		tarot_free(string);
	} else {
		token->kind = TAROT_TOK_IDENTIFIER;
		token->value.String = string;
	}
}

static void handle_escape_sequence(
	struct tarot_string **stringptr,
	struct tarot_iostream *stream
) {
	switch(tarot_fcurr(stream)) {
		default:
			break;
		case '"':
		case '\\':
			tarot_string_append(stringptr, "%c", tarot_fcurr(stream));
			advance(stream);
			break;
		case 'b':
			tarot_string_append(stringptr, "\b");
			advance(stream);
			break;
		case 'f':
			tarot_string_append(stringptr, "\f");
			advance(stream);
			break;
		case 'n':
			tarot_string_append(stringptr, "\n");
			advance(stream);
			break;
		case 'r':
			tarot_string_append(stringptr, "\r");
			advance(stream);
			break;
		case 't':
			tarot_string_append(stringptr, "\t");
			advance(stream);
			break;
		case '{':
			tarot_string_append(stringptr, "{");
			advance(stream);
			break;
		case '}':
			tarot_string_append(stringptr, "}");
			advance(stream);
			break;
	}
}

static void add_character(
	struct tarot_string **stringptr,
	struct tarot_iostream *stream
) {
	if (tarot_fcurr(stream) == '\\') {
		advance(stream);
		handle_escape_sequence(stringptr, stream);
	} else {
		tarot_string_append(stringptr, "%c", tarot_fcurr(stream));
		advance(stream);
	}
}

static void tokenize_string(
	struct tarot_iostream *stream,
	struct tarot_token *token
) {
	struct tarot_string *string = tarot_create_string(NULL);
	while (not tarot_feof(stream) and tarot_fcurr(stream) != '"') {
		add_character(&string, stream);
	}
	if (match(stream, '"')) {
		token->kind = TAROT_TOK_STRING;
		token->value.String = string;
	} else {
		tarot_free(string);
		tarot_error_at(&token->position, "Unterminated string");
	}
}

static void tokenize_fstring(
	struct tarot_iostream *stream,
	struct tarot_token *token
) {
	struct tarot_string *string = tarot_create_string(NULL);
	unsigned int brackets = 0;
	assert(tarot_fcurr(stream) == '"');
	advance(stream);
	while (not tarot_feof(stream)) {
		if (tarot_fcurr(stream) == '{') {
			brackets++;
		} else if (tarot_fcurr(stream) == '}') {
			if (brackets > 0) {
				brackets--;
			} else {
				tarot_error_at(&token->position, "Too many closing }");
			}
		} else if (tarot_fcurr(stream) == '"' and brackets == 0) {
			break;
		}
		add_character(&string, stream);
	}
	if (match(stream, '"')) {
		token->kind = TAROT_TOK_FSTRING;
		token->value.String = string;
	} else {
		tarot_free(string);
		tarot_error_at(&token->position, "Unterminated fstring");
	}
}

static void tokenize_rstring(
	struct tarot_iostream *stream,
	struct tarot_token *token
) {
	struct tarot_string *string = tarot_create_string(NULL);
	assert(tarot_fcurr(stream) == '"');
	advance(stream);
	while (not tarot_feof(stream) and tarot_fcurr(stream) != '"') {
		tarot_string_append(&string, "%c", tarot_fcurr(stream));
		advance(stream);
	}
	if (match(stream, '"')) {
		token->kind = TAROT_TOK_RSTRING;
		token->value.String = string;
	} else {
		tarot_free(string);
		tarot_error_at(&token->position, "Unterminated rstring");
	}
}

static bool match_textblock(struct tarot_iostream *stream) {
	return match(stream, '`') and match(stream, '`') and match(stream, '`');
}

static void tokenize_textblock(
	struct tarot_iostream *stream,
	struct tarot_token *token
) {
	struct tarot_string *string;
	string = tarot_create_string(NULL);
	while (not match_textblock(stream) and not tarot_feof(stream)) {
		tarot_string_append(&string, "%c", tarot_fcurr(stream));
		advance(stream);
	}
	if (match(stream, '`')) {
		token->kind = TAROT_TOK_TEXTBLOCK;
		token->value.String = string;
	} else {
		tarot_free(string);
		tarot_error_at(&token->position, "Unterminated textblock");
	}
}

static void tokenize_comment(
	struct tarot_iostream *stream,
	struct tarot_token *token
) {
	unsigned int nesting = 1;
	token->kind = TAROT_TOK_COMMENT;
	while ((nesting > 0) and not tarot_feof(stream)) {
		switch (tarot_fcurr(stream)) {
			default:
				advance(stream);
				break;
			case '/':
				if (advance(stream) == '*') {
					advance(stream);
					nesting++;
				} break;
			case '*':
				if (advance(stream) == '/') {
					advance(stream);
					nesting--;
				} break;
		}
	}
}

static void tokenize(
	struct tarot_iostream *stream,
	struct tarot_token *token
) {
	int current = tarot_fcurr(stream);
	int next = advance(stream);
	assert(token != NULL);
	switch (current) {
		default:
			tarot_error_at(&token->position, "Invalid token '%c'!", current);
			break;
		case '+':
			if (match(stream, '=')) {
				token->kind = TAROT_TOK_ASSIGN_ADD;
			} else {
				token->kind = TAROT_TOK_PLUS;
			}
			break;
		case '-':
			if (match(stream, '>')) {
				token->kind = TAROT_TOK_ARROW;
			} else if (match(stream, '=')) {
				token->kind = TAROT_TOK_ASSIGN_SUB;
			} else {
				token->kind = TAROT_TOK_MINUS;
			} break;
		case '*':
			if (match(stream, '*')) {
				token->kind = TAROT_TOK_POWER;
			} else if (match(stream, '=')) {
				token->kind = TAROT_TOK_ASSIGN_MUL;
			} else {
				token->kind = TAROT_TOK_MULTIPLY;
			} break;
		case '/':
			if (next == '*') {
				tokenize_comment(stream, token);
			} else if (match(stream, '=')) {
				token->kind = TAROT_TOK_ASSIGN_DIV;
			} else {
				token->kind = TAROT_TOK_DIVIDE;
			} break;
		case '=':
			if (match(stream, '=')) {
				token->kind = TAROT_TOK_EQUAL;
			} else {
				token->kind = TAROT_TOK_ASSIGN;
			} break;
		case '!':
			if (match(stream, '=')) {
				token->kind = TAROT_TOK_NOT_EQUAL;
			} else {
				tarot_error_at(&token->position, "Invalid token '%c'!", current);
			} break;
		case '<':
			if (match(stream, '=')) {
				token->kind = TAROT_TOK_LESS_EQUAL;
			} else {
				token->kind = TAROT_TOK_LESS_THAN;
			} break;
		case '>':
			if (match(stream, '=')) {
				token->kind = TAROT_TOK_GREATER_EQUAL;
			} else {
				token->kind = TAROT_TOK_GREATER_THAN;
			} break;
		case '(':
			token->kind = TAROT_TOK_OPEN_BRACKET;
			break;
		case ')':
			token->kind = TAROT_TOK_CLOSE_BRACKET;
			break;
		case '{':
			token->kind = TAROT_TOK_OPEN_CURLY_BRACKET;
			break;
		case '}':
			token->kind = TAROT_TOK_CLOSE_CURLY_BRACKET;
			break;
		case '[':
			token->kind = TAROT_TOK_OPEN_ANGULAR_BRACKET;
			break;
		case ']':
			token->kind = TAROT_TOK_CLOSE_ANGULAR_BRACKET;
			break;
		case ',':
			token->kind = TAROT_TOK_COMMA;
			break;
		case ';':
			token->kind = TAROT_TOK_SEMICOLON;
			break;
		case ':':
			if (match(stream, '=')) {
				token->kind = TAROT_TOK_ASSIGN;
			} else {
				token->kind = TAROT_TOK_COLON;
			}
			break;
		case '.':
			token->kind = TAROT_TOK_DOT;
			break;
		case '"':
			tokenize_string(stream, token);
			break;
		case '#':
			while (not match(stream, '\n')) {
				if (tarot_feof(stream)) {
					break;
				}
				advance(stream);
			}
			token->kind = TAROT_TOK_COMMENT;
			break;
	}
}

static void skip_whitespace(struct tarot_iostream *stream) {
	while (isspace(tarot_fcurr(stream))) {
		advance(stream);
	}
}

static void reset_token(
	struct tarot_token *token,
	struct tarot_iostream *stream
) {
	token->kind = TAROT_TOK_EOF;
	memset(&token->value, 0, sizeof(token->value));
	memcpy(&token->position, tarot_fgetpos(stream), sizeof(token->position));
}

static void tokenize_string_or_identifier(
	struct tarot_iostream *stream,
	struct tarot_token *token
) {
	int ch = tarot_fcurr(stream);
	advance(stream);
	if (ch == 'f' and tarot_fcurr(stream) == '"') {
		tokenize_fstring(stream, token);
	} else if (ch == 'r' and tarot_fcurr(stream) == '"') {
		tokenize_rstring(stream, token);
	} else {
		tokenize_identifier(stream, token, ch);
	}
}

void tarot_read_token(
	struct tarot_iostream *stream,
	struct tarot_token *token
) {
	assert(stream != NULL);
	assert(token != NULL);
	skip_whitespace(stream);
	reset_token(token, stream);
	if (tarot_feof(stream)) {
		token->kind = TAROT_TOK_EOF;
	} else if (isalpha(tarot_fcurr(stream)) or tarot_fcurr(stream) == '_') {
		tokenize_string_or_identifier(stream, token);
	} else if (isdigit(tarot_fcurr(stream))) {
		tokenize_number(stream, token);
	} else if (match_textblock(stream)) {
		tokenize_textblock(stream, token);
	} else {
		tokenize(stream, token);
	}
}

void tarot_read_tokens_from_stream(
	struct tarot_iostream *stream,
	void (*process)(struct tarot_token *token, void *data),
	void *data
) {
	struct tarot_token token;
	assert(stream != NULL);
	assert(process != NULL);
	do {
		tarot_read_token(stream, &token);
		process(&token, data);
	} while (token.kind != TAROT_TOK_EOF);
}

void tarot_read_tokens_from_file(
	const char *path,
	void (*process)(struct tarot_token *token, void *data),
	void *data
) {
	struct tarot_iostream *stream = tarot_fopen(path, TAROT_INPUT);
	assert(process != NULL);
	if (stream != NULL) {
		tarot_read_tokens_from_stream(stream, process, data);
		tarot_fclose(stream);
	}
}
