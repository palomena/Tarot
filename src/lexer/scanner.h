#ifndef TAROT_LEXER_H
#define TAROT_LEXER_H

#include "lexer/token.h"

/* Forward declaration */
struct tarot_token;

extern void tarot_set_max_line_length(size_t value);
extern void tarot_set_max_indentation(size_t value);

extern void tarot_read_token(
	struct tarot_iostream *stream,
	struct tarot_token *token
);

void tarot_read_tokens_from_stream(
	struct tarot_iostream *stream,
	void (*process_token)(struct tarot_token *token, void *userdata),
	void *userdata
);

extern void tarot_read_tokens_from_file(
	const char *path,
	void (*process_token)(struct tarot_token *token, void *userdata),
	void *userdata
);

extern void tarot_read_tokens_from_memory(
	const char *buffer,
	void (*process_token)(struct tarot_token *token, void *userdata),
	void *userdata
);

#endif /* TAROT_LEXER_H */
