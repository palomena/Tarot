#define TAROT_SOURCE
#include "tarot.h"

/******************************************************************************
 * MARK: PARSER
 *****************************************************************************/

struct tarot_parser {
	struct tarot_iostream *stream; /**< The token input stream          */
	struct tarot_token *current;   /**< Points to the current token     */
	struct tarot_token *next;      /**< Points to the next token        */
	struct tarot_token tokens[2];  /**< Token buffer for current & next */
	struct scope_stack scopes;
	bool is_calm; /**< Panic indicator, set on error   */
};

static void read_token(
	struct tarot_iostream *stream,
	struct tarot_token *token
) {
	do {
		tarot_read_token(stream, token);
	} while (token->kind == TAROT_TOK_COMMENT);
}

static void initialize_parser(
	struct tarot_parser *parser,
	struct tarot_iostream *stream
) {
	assert(parser != NULL);
	assert(stream != NULL);
	parser->stream  = stream;
	parser->current = &parser->tokens[0];
	parser->next    = &parser->tokens[1];
	parser->is_calm = true;
	read_token(parser->stream, parser->current);
	read_token(parser->stream, parser->next);
	memset(&parser->scopes, 0, sizeof(parser->scopes));
}

static void exit_parser(struct tarot_parser *parser) {
	if (not parser->is_calm) {
		tarot_clear_token(parser->current);
		tarot_clear_token(parser->next);
	}
}

static void add_to_current_scope(
	struct tarot_parser *parser,
	struct tarot_node *node
) {
	if (parser->is_calm) {
		assert(node != NULL);
		add_symbol(&parser->scopes, node);
	}
}

static void advance(struct tarot_parser *parser) {
	assert(parser != NULL);
	if (parser->current->kind != TAROT_TOK_EOF) {
		swap(struct tarot_token*, parser->current, parser->next);
		read_token(parser->stream, parser->next);
	} else {
		parser->is_calm = false;
		tarot_error_at(
			&parser->current->position,
			"Unexpectedly reached the end of file %s!",
			parser->current->position.path
		);
	}
}

static bool current(
	struct tarot_parser *parser,
	enum tarot_token_kind a
) {
	assert(parser != NULL);
	return parser->is_calm and parser->current->kind == a;
}

static bool peek(
	struct tarot_parser *parser,
	enum tarot_token_kind a,
	enum tarot_token_kind b
) {
	assert(parser != NULL);
	assert(a != TAROT_TOK_EOF);
	return (
		parser->is_calm
		and (parser->current->kind == a)
		and (parser->next->kind == b)
	);
}

static bool match(
	struct tarot_parser *parser,
	enum tarot_token_kind type,
	struct tarot_token *token
) {
	bool success = false;
	assert(parser != NULL);
	if (current(parser, type)) {
		if (token != NULL) {
			memcpy(token, parser->current, sizeof(*token));
		}
		/* Only advance if we do not expect the end of the file yet: */
		if (type != TAROT_TOK_EOF) {
			advance(parser);
		}
		success = true;
	}
	return success;
}

static bool match_identifier(
	struct tarot_parser *parser,
	const char *name,
	struct tarot_token *token
) {
	bool success = false;
	assert(parser != NULL);
	if (
		current(parser, TAROT_TOK_IDENTIFIER) and
		tarot_match_string(parser->current->value.String, name)
	) {
		if (token != NULL) {
			memcpy(token, parser->current, sizeof(*token));
		}
		advance(parser);
		success = true;
	}
	return success;
}

static bool expect(
	struct tarot_parser *parser,
	enum tarot_token_kind kind
) {
	bool result = false;
	assert(parser != NULL);
	if (parser->is_calm and not match(parser, kind, NULL)) {
		parser->is_calm = false;
		tarot_error_at(
			&parser->current->position,
			"Expected %s %s%s%s%s but got %s %s%s%s%s instead.",
			tarot_token_class(kind),
			tarot_color_string(TAROT_COLOR_YELLOW),
			tarot_color_string(TAROT_COLOR_BOLD),
			tarot_token_string(kind),
			tarot_color_string(TAROT_COLOR_RESET),
			tarot_token_class(parser->current->kind),
			tarot_color_string(TAROT_COLOR_YELLOW),
			tarot_color_string(TAROT_COLOR_BOLD),
			tarot_token_string(parser->current->kind),
			tarot_color_string(TAROT_COLOR_RESET)
		);
		result = true;
	}
	return result;
}

static bool require(
	struct tarot_parser *parser,
	struct tarot_node *node
) {
	if (parser->is_calm and node == NULL) {
		parser->is_calm = false;
		return false;
	}
	return true;
}

typedef struct tarot_node* syntax_rule(struct tarot_parser *parser);

static struct tarot_node* recover(
	struct tarot_parser *parser,
	syntax_rule rule
) {
	struct tarot_node *node = NULL;
	while (parser->current->kind != TAROT_TOK_EOF) {
		parser->is_calm = true;
		tarot_clear_token(parser->current);
		advance(parser);
		node = rule(parser);
		if (node != NULL) {
			break;
		}
	}
	if (node == NULL) {
		parser->is_calm = false;
		/* error is unrecoverable using the given rule */
	}
	return node;
}

static struct tarot_node* result(
	struct tarot_parser *parser,
	struct tarot_node *node
) {
	if (node != NULL and not parser->is_calm) {
		tarot_free_node(node);
		node = NULL;
	}
	return node;
}

/******************************************************************************
 * MARK: GRAMMAR
 *****************************************************************************/

/* Essential forward declarations */
static struct tarot_node* parse_definition(struct tarot_parser *parser);
static struct tarot_node* parse_expression(struct tarot_parser *parser);
static struct tarot_node* parse_statement(struct tarot_parser *parser);
static struct tarot_node* parse_global(struct tarot_parser *parser);

/******************************************************************************
 * MARK: > Utils
 *****************************************************************************/

static enum tarot_visibility read_visibility(struct tarot_parser *parser) {
	enum tarot_visibility result = VISIBILITY_NONE;
	if (match(parser, TAROT_TOK_PUBLIC, NULL)) {
		result = VISIBILITY_PUBLIC;
	} else if (match(parser, TAROT_TOK_PRIVATE, NULL)) {
		result = VISIBILITY_PRIVATE;
	}
	return result;
}

static struct tarot_string* read_string(struct tarot_parser *parser) {
	struct tarot_string *string = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_STRING, &token)) {
		string = token.value.String;
	} else {
		expect(parser, TAROT_TOK_STRING);
	}
	return string;
}

static struct tarot_string* read_identifier(struct tarot_parser *parser) {
	struct tarot_string *string = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_IDENTIFIER, &token)) {
		string = token.value.String;
	} else {
		expect(parser, TAROT_TOK_IDENTIFIER);
	}
	return string;
}

static struct tarot_list* multiparse(
	struct tarot_parser *parser,
	syntax_rule rule,
	bool comma_seperated
) {
	struct tarot_node *it;
	struct tarot_list *list = tarot_create_list(sizeof(it), 5, NULL);
	bool is_calm = parser->is_calm;
	do {
		it = rule(parser);
		if (not parser->is_calm) {
			it = recover(parser, rule);
		}
		if (it != NULL) {
			tarot_list_append(&list, &it);
		}
		if (comma_seperated and not match(parser, TAROT_TOK_COMMA, NULL)) {
			break;
		}
	} while (it != NULL);
	tarot_trim_list(&list);
	if (parser->is_calm) { /* restore to error if previously in error */
		parser->is_calm = is_calm;
	}
	return list;
}

static struct tarot_node* parse_comma_seperated(
	struct tarot_parser *parser,
	syntax_rule rule
) {
	struct tarot_node *node = tarot_create_node(NODE_Block, &parser->current->position);
	struct tarot_list *list = multiparse(parser, rule, true);
	Block(node)->elements = tarot_list_to_array(list);
	Block(node)->num_elements = tarot_list_length(list);
	Block(node)->kind = BLOCK_LIST;
	tarot_free_list(list);
	return result(parser, node);
}

/******************************************************************************
 * MARK: > Identifier
 *****************************************************************************/

static struct tarot_node* parse_identifier(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_SELF, &token)) {
		node = tarot_create_node(NODE_Identifier, &token.position);
		Identifier(node)->name = tarot_create_string("self");
	} else if (match(parser, TAROT_TOK_IDENTIFIER, &token)) {
		node = tarot_create_node(NODE_Identifier, &token.position);
		Identifier(node)->name = token.value.String;
	}
	return result(parser, node);
}

/******************************************************************************
 * MARK: > Literal
 *****************************************************************************/

static enum tarot_datatype literal_to_datatype(enum tarot_literal_kind kind) {
	switch (kind) {
		case VALUE_BOOL:
			return TYPE_BOOLEAN;
		case VALUE_FLOAT:
			return TYPE_FLOAT;
		case VALUE_INTEGER:
			return TYPE_INTEGER;
		case VALUE_RATIONAL:
			return TYPE_RATIONAL;
		case VALUE_RAW_STRING:
		case VALUE_STRING:
			return TYPE_STRING;
	}
	tarot_error("Invalid literal kind");
	tarot_abort();
}

static struct tarot_node* create_literal(
	struct tarot_stream_position *position,
	enum tarot_literal_kind kind
) {
	struct tarot_node *node = tarot_create_node(NODE_Literal, position);
	Literal(node)->kind = kind;
	Literal(node)->type = literal_to_datatype(kind);
	return node;
}

static struct tarot_node* parse_boolean(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_TRUE, &token)) {
		node = create_literal(&token.position, VALUE_BOOL);
		Literal(node)->value.Boolean = true;
	} else if (match(parser, TAROT_TOK_FALSE, &token)) {
		node = create_literal(&token.position, VALUE_BOOL);
		Literal(node)->value.Boolean = false;
	}
	return result(parser, node);
}

static struct tarot_node* parse_float(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_FLOAT, &token)) {
		node = create_literal(&token.position, VALUE_FLOAT);
		Literal(node)->value.Float = token.value.Float;
	}
	return node;
}

static struct tarot_node* parse_integer(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_INTEGER, &token)) {
		node = create_literal(&token.position, VALUE_INTEGER);
		Literal(node)->value.Integer = token.value.Integer;
	}
	return result(parser, node);
}

static struct tarot_node* parse_rational(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_RATIONAL, &token)) {
		node = create_literal(&token.position, VALUE_RATIONAL);
		Literal(node)->value.Rational = token.value.Rational;
	}
	return result(parser, node);
}

static struct tarot_list* parse_fstring(struct tarot_token *token) {
	struct tarot_list *slices = tarot_create_list(sizeof(struct tarot_node*), 1, NULL);
	char *strptr = tarot_string_text(token->value.String);
	while (*strptr) {
		struct tarot_node *object = NULL;
		if (*strptr == '{') { /* F-String expression */
			struct tarot_parser parser;
			struct tarot_iostream *stream;
			unsigned int brackets = 1;
			struct tarot_string *string = tarot_create_string("");
			strptr++; /* skip '{' */
			while (*strptr) {
				if (*strptr == '}') {
					brackets--;
					if (brackets == 0) {
						break;
					}
				} else if (*strptr == '{') {
					brackets++;
				}
				tarot_string_append(&string, "%c", *strptr++);
			}
			stream = tarot_fstropen(&string, TAROT_INPUT);
			tarot_override_stream_position(stream, &token->position);
			initialize_parser(&parser, stream);
			object = tarot_create_node(NODE_FStringExpression, &token->position);
			FStringExpression(object)->expression = parse_expression(&parser);
			expect(&parser, TAROT_TOK_EOF);
			if (FStringExpression(object)->expression == NULL) {
				tarot_error_at(&token->position, "Invalid fstring expression");
			}
			exit_parser(&parser);
			tarot_fclose(stream);
			tarot_free_string(string);
			strptr++; /* skip '}' */
		} else { /* F-String string slice */
			struct tarot_string *string = tarot_create_string("");
			while (*strptr and *strptr != '{') {
				tarot_string_append(&string, "%c", *strptr++);
			}
			object = tarot_create_node(NODE_FStringString, &token->position);
			FStringString(object)->value = string;
		}
		tarot_list_append(&slices, &object);
	}
	tarot_clear_token(token);
	return slices;
}

static struct tarot_node* parse_string(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_STRING, &token)) {
		node = create_literal(&token.position, VALUE_STRING);
		Literal(node)->value.String = token.value.String;
		while (match(parser, TAROT_TOK_STRING, &token)) {
			struct tarot_string *new_string = tarot_concat_strings(
				Literal(node)->value.String,
				token.value.String
			);
			tarot_free_string(Literal(node)->value.String);
			tarot_clear_token(&token);
			Literal(node)->value.String = new_string;
		}
	} else if (match(parser, TAROT_TOK_FSTRING, &token)) {
		struct tarot_list *slices = parse_fstring(&token);
		node = tarot_create_node(NODE_FString, &token.position);
		FString(node)->elements = tarot_list_to_array(slices);
		FString(node)->num_elements = tarot_list_length(slices);
		tarot_free_list(slices);
	} else if (match(parser, TAROT_TOK_RSTRING, &token)) {
		node = create_literal(&token.position, VALUE_RAW_STRING);
		Literal(node)->value.String = token.value.String;
	}
	return result(parser, node);
}

static struct tarot_node* parse_literal(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	(node = parse_boolean(parser))  or
	(node = parse_float(parser))    or
	(node = parse_integer(parser))  or
	(node = parse_rational(parser)) or
	(node = parse_string(parser));
	return result(parser, node);
}

/******************************************************************************
 * MARK: > Expression
 *****************************************************************************/

/* NULL | Identifier | BinaryExpression[Relation] */
static struct tarot_node* parse_relation(struct tarot_parser *parser) {
	struct tarot_node *node = parse_identifier(parser);
	struct tarot_token token;
	while (node != NULL) {
		struct tarot_node *left_operand = node;
		if (match(parser, TAROT_TOK_DOT, &token)) {
			node = tarot_create_node(NODE_Relation, &token.position);
			Relation(node)->parent = left_operand;
			Relation(node)->child = read_identifier(parser);
		} else break;
	}
	return result(parser, node);
}

/**
 * NULL
 * | Identifier
 * | BinaryExpression[Relation]
 * | BinaryExpression[Subscript]
 * | FunctionCall
 */
static struct tarot_node* parse_primary_expression(struct tarot_parser *parser) {
	struct tarot_node *node = parse_identifier(parser);
	struct tarot_token token;
	while (node != NULL) {
		struct tarot_node *left_operand = node;
		if (match(parser, TAROT_TOK_DOT, &token)) {
			node = tarot_create_node(NODE_Relation, &token.position);
			Relation(node)->parent = left_operand;
			Relation(node)->child = read_identifier(parser);
		} else if (match(parser, TAROT_TOK_OPEN_ANGULAR_BRACKET, &token)) {
			node = tarot_create_node(NODE_Subscript, &token.position);
			Subscript(node)->identifier = left_operand;
			Subscript(node)->index = parse_expression(parser);
			expect(parser, TAROT_TOK_CLOSE_ANGULAR_BRACKET);
		} else if (match(parser, TAROT_TOK_OPEN_BRACKET, &token)) {
			node = tarot_create_node(NODE_FunctionCall, &token.position);
			FunctionCall(node)->identifier = left_operand;
			FunctionCall(node)->arguments = parse_comma_seperated(parser, parse_expression);
			expect(parser, TAROT_TOK_CLOSE_BRACKET);
		} else break;
	}
	return result(parser, node);
}

static struct tarot_node* parse_not_expression(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_NOT, &token)) {
		node = tarot_create_node(NODE_Not, &token.position);
		NotExpression(node)->expression = parse_expression(parser);
	}
	return result(parser, node);
}

static struct tarot_node* parse_neg_expression(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_MINUS, &token)) {
		node = tarot_create_node(NODE_Neg, &token.position);
		NegExpression(node)->expression = parse_expression(parser);
	}
	return result(parser, node);
}

static struct tarot_node* parse_abs_expression(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match_identifier(parser, "abs", &token)) {
		node = tarot_create_node(NODE_Abs, &token.position);
		tarot_clear_token(&token);
		expect(parser, TAROT_TOK_OPEN_BRACKET);
		AbsExpression(node)->expression = parse_expression(parser);
		expect(parser, TAROT_TOK_CLOSE_BRACKET);
	}
	return result(parser, node);
}

static struct tarot_node* parse_input(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match_identifier(parser, "input", &token)) {
		node = tarot_create_node(NODE_Input, &token.position);
		expect(parser, TAROT_TOK_OPEN_BRACKET);
		InputExpression(node)->prompt = parse_string(parser);
		expect(parser, TAROT_TOK_CLOSE_BRACKET);
		tarot_clear_token(&token);
	}
	return result(parser, node);
}

static bool match_builtin_type(
	struct tarot_parser *parser,
	struct tarot_token *token
) {
	return (
		match_identifier(parser, "Boolean", token) or
		match_identifier(parser, "Integer", token) or
		match_identifier(parser, "Rational", token) or
		match_identifier(parser, "Float", token) or
		match_identifier(parser, "String", token)
	);
}

static enum tarot_datatype string_to_datatype(struct tarot_string *string) {
	enum tarot_datatype type;
	static const enum tarot_datatype types[] = {
		TYPE_VOID,
		TYPE_BOOLEAN,
		TYPE_INTEGER,
		TYPE_FLOAT,
		TYPE_RATIONAL,
		TYPE_STRING,
		TYPE_LIST,
		TYPE_DICT
	};
	for (type = 0; type < lengthof(types); type++) {
		if (tarot_match_string(string, datatype_string(type))) {
			return type;
		}
	}
	return TYPE_CUSTOM;
}

static struct tarot_node* parse_typecast(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match_builtin_type(parser, &token)) {
		node = tarot_create_node(NODE_Typecast, &token.position);
		CastExpression(node)->kind = string_to_datatype(token.value.String);
		tarot_clear_token(&token);
		expect(parser, TAROT_TOK_OPEN_BRACKET);
		CastExpression(node)->operand = parse_expression(parser);
		expect(parser, TAROT_TOK_CLOSE_BRACKET);
	}
	return result(parser, node);
}

static struct tarot_node* parse_infix_expression(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_OPEN_BRACKET, &token)) {
		node = tarot_create_node(NODE_InfixExpression, &token.position);
		InfixExpression(node)->expression = parse_expression(parser);
		expect(parser, TAROT_TOK_CLOSE_BRACKET);
	}
	return result(parser, node);
}

static struct tarot_node *parse_list(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_OPEN_ANGULAR_BRACKET, &token)) {
		struct tarot_list *list = multiparse(parser, parse_expression, true);
		node = tarot_create_node(NODE_List, &token.position);
		List(node)->elements = tarot_list_to_array(list);
		List(node)->num_elements = tarot_list_length(list);
		tarot_free_list(list);
		expect(parser, TAROT_TOK_CLOSE_ANGULAR_BRACKET);
	}
	return result(parser, node);
}

static struct tarot_node* parse_pair(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_node *key = parse_string(parser);
	if (key != NULL) {
		expect(parser, TAROT_TOK_COLON);
		node = tarot_create_node(NODE_Pair, position_of(key));
		Pair(node)->key = key;
		Pair(node)->value = parse_expression(parser);
	}
	return result(parser, node);
}

static struct tarot_node *parse_dictionary(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_OPEN_CURLY_BRACKET, &token)) {
		struct tarot_list *list = multiparse(parser, parse_pair, true);
		node = tarot_create_node(NODE_Dict, &token.position);
		Dict(node)->elements = tarot_list_to_array(list);
		Dict(node)->num_elements = tarot_list_length(list);
		tarot_free_list(list);
		expect(parser, TAROT_TOK_CLOSE_CURLY_BRACKET);
	}
	return result(parser, node);
}

static struct tarot_node* parse_value_expression(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	(node = parse_list(parser)) or
	(node = parse_dictionary(parser)) or
	(node = parse_literal(parser));
	return result(parser, node);
}

static struct tarot_node* parse_unary_expression(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	(node = parse_infix_expression(parser)) or
	(node = parse_not_expression(parser)) or
	(node = parse_neg_expression(parser)) or
	(node = parse_abs_expression(parser)) or
	(node = parse_typecast(parser)) or
	(node = parse_input(parser)) or
	(node = parse_value_expression(parser)) or
	(node = parse_primary_expression(parser));
	return result(parser, node);
}

static struct tarot_node* create_ArithmeticExpression(
	struct tarot_stream_position *position,
	struct tarot_node *left_operand,
	enum ArithmeticExpressionOperator operator,
	struct tarot_node *right_operand
) {
	struct tarot_node *node = tarot_create_node(NODE_ArithmeticExpression, position);
	ArithmeticExpression(node)->left_operand = left_operand;
	ArithmeticExpression(node)->operator = operator;
	ArithmeticExpression(node)->right_operand = right_operand;
	return node;
}

static struct tarot_node* create_RelationalExpression(
	struct tarot_stream_position *position,
	struct tarot_node *left_operand,
	enum RelationalExpressionOperator operator,
	struct tarot_node *right_operand
) {
	struct tarot_node *node = tarot_create_node(NODE_RelationalExpression, position);
	RelationalExpression(node)->left_operand = left_operand;
	RelationalExpression(node)->operator = operator;
	RelationalExpression(node)->right_operand = right_operand;
	return node;
}

static struct tarot_node* create_LogicalExpression(
	struct tarot_stream_position *position,
	struct tarot_node *left_operand,
	enum LogicalExpressionOperator operator,
	struct tarot_node *right_operand
) {
	struct tarot_node *node = tarot_create_node(NODE_LogicalExpression, position);
	LogicalExpression(node)->left_operand = left_operand;
	LogicalExpression(node)->operator = operator;
	LogicalExpression(node)->right_operand = right_operand;
	return node;
}

static struct tarot_node* parse_postfix_expression(struct tarot_parser *parser) {
	struct tarot_node *node = parse_unary_expression(parser);
	struct tarot_token token;
	if (match(parser, TAROT_TOK_POWER, &token)) {
		node = create_ArithmeticExpression(&token.position, node, EXPR_POWER, parse_postfix_expression(parser));
	}
	return result(parser, node);
}

static struct tarot_node* parse_multiplicative_expression(struct tarot_parser *parser) {
	struct tarot_node *node = parse_postfix_expression(parser);
	struct tarot_token token;
	if (match(parser, TAROT_TOK_MULTIPLY, &token)) {
		node = create_ArithmeticExpression(&token.position, node, EXPR_MULTIPLY, parse_multiplicative_expression(parser));
	} else if (match(parser, TAROT_TOK_DIVIDE, &token)) {
		node = create_ArithmeticExpression(&token.position, node, EXPR_DIVIDE, parse_multiplicative_expression(parser));
	} else if (match(parser, TAROT_TOK_MOD, &token)) {
		node = create_ArithmeticExpression(&token.position, node, EXPR_MODULO, parse_multiplicative_expression(parser));
	}
	return result(parser, node);
}

static struct tarot_node* parse_additive_expression(struct tarot_parser *parser) {
	struct tarot_node *node = parse_multiplicative_expression(parser);
	struct tarot_token token;
	if (match(parser, TAROT_TOK_PLUS, &token)) {
		node = create_ArithmeticExpression(&token.position, node, EXPR_ADD, parse_additive_expression(parser));
	} else if (match(parser, TAROT_TOK_MINUS, &token)) {
		node = create_ArithmeticExpression(&token.position, node, EXPR_SUBTRACT, parse_additive_expression(parser));
	}
	return result(parser, node);
}

static struct tarot_node* parse_relational_expression(struct tarot_parser *parser) {
	struct tarot_node *node = parse_additive_expression(parser);
	struct tarot_token token;
	if (match(parser, TAROT_TOK_GREATER_THAN, &token)) {
		node = create_RelationalExpression(&token.position, node, EXPR_GREATER, parse_relational_expression(parser));
	} else if (match(parser, TAROT_TOK_LESS_THAN, &token)) {
		node = create_RelationalExpression(&token.position, node, EXPR_LESS, parse_relational_expression(parser));
	} else if (match(parser, TAROT_TOK_GREATER_EQUAL, &token)) {
		node = create_RelationalExpression(&token.position, node, EXPR_GREATER_EQUAL, parse_relational_expression(parser));
	} else if (match(parser, TAROT_TOK_LESS_EQUAL, &token)) {
		node = create_RelationalExpression(&token.position, node, EXPR_LESS_EQUAL, parse_relational_expression(parser));
	}
	return result(parser, node);
}

static struct tarot_node* parse_equality_expression(struct tarot_parser *parser) {
	struct tarot_node *node = parse_relational_expression(parser);
	struct tarot_token token;
	if (match(parser, TAROT_TOK_EQUAL, &token)) {
		node = create_RelationalExpression(&token.position, node, EXPR_EQUAL, parse_equality_expression(parser));
	} else if (match(parser, TAROT_TOK_NOT_EQUAL, &token)) {
		node = create_RelationalExpression(&token.position, node, EXPR_NOT_EQUAL, parse_equality_expression(parser));
	} else if (match(parser, TAROT_TOK_IN, &token)) {
		node = create_RelationalExpression(&token.position, node, EXPR_IN, parse_equality_expression(parser));
	}
	return result(parser, node);
}

static struct tarot_node* parse_logical_expression(struct tarot_parser *parser) {
	struct tarot_node *node = parse_equality_expression(parser);
	struct tarot_token token;
	if (match(parser, TAROT_TOK_AND, &token)) {
		node = create_LogicalExpression(&token.position, node, EXPR_AND, parse_logical_expression(parser));
	} else if (match(parser, TAROT_TOK_OR, &token)) {
		node = create_LogicalExpression(&token.position, node, EXPR_OR, parse_logical_expression(parser));
	} else if (match(parser, TAROT_TOK_XOR, &token)) {
		node = create_LogicalExpression(&token.position, node, EXPR_XOR, parse_logical_expression(parser));
	}
	return result(parser, node);
}

static struct tarot_node* parse_expression(struct tarot_parser *parser) {
	return parse_logical_expression(parser);
}

/******************************************************************************
 * MARK: > Defs
 *****************************************************************************/

static struct tarot_node* parse_block(
	struct tarot_parser *parser,
	syntax_rule rule
) {
	struct tarot_node *node = tarot_create_node(NODE_Block, &parser->current->position);
	struct tarot_list *list;
	list = multiparse(parser, rule, false);
	Block(node)->elements = tarot_list_to_array(list);
	Block(node)->num_elements = tarot_list_length(list);
	Block(node)->kind = BLOCK_DEFAULT;
	tarot_free_list(list);
	return result(parser, node);
}

static struct tarot_node* parse_scoped_block(
	struct tarot_parser *parser,
	syntax_rule rule
) {
	struct tarot_node *node = NULL;
	expect(parser, TAROT_TOK_OPEN_CURLY_BRACKET);
	node = parse_block(parser, rule);
	expect(parser, TAROT_TOK_CLOSE_CURLY_BRACKET);
	if (node != NULL) {
		Block(node)->kind = BLOCK_SCOPED;
	}
	return result(parser, node);
}

static struct tarot_node* parse_datatype(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_node *identifier = parse_relation(parser);
	if (identifier != NULL) {
		node = tarot_create_node(NODE_Type, position_of(identifier));
		if (kind_of(identifier) == NODE_Identifier) {
			Type(node)->type = string_to_datatype(name_of(identifier));
			if (Type(node)->type != TYPE_CUSTOM) {
				tarot_free_node(identifier);
				identifier = NULL;
			}
		}
		Type(node)->identifier = identifier;
		if (match(parser, TAROT_TOK_OPEN_ANGULAR_BRACKET, NULL)) {
			Type(node)->subtype = parse_datatype(parser);
			expect(parser, TAROT_TOK_CLOSE_ANGULAR_BRACKET);
		}
	}
	return result(parser, node);
}

static struct tarot_node* parse_constant(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_CONSTANT, &token)) {
		node = tarot_create_node(NODE_Constant, &token.position);
		Constant(node)->name = read_identifier(parser);
		if (match(parser, TAROT_TOK_COLON, NULL)) {
			Constant(node)->type = parse_datatype(parser);
		}
		expect(parser, TAROT_TOK_ASSIGN);
		Constant(node)->value = parse_expression(parser);
		expect(parser, TAROT_TOK_SEMICOLON);
		add_to_current_scope(parser, node);
	}
	return result(parser, node);
}

static struct tarot_node* parse_variable(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_LET, &token)) {
		node = tarot_create_node(NODE_Variable, &token.position);
		Variable(node)->name = read_identifier(parser);
		if (match(parser, TAROT_TOK_COLON, NULL)) {
			Variable(node)->type = parse_datatype(parser);
		}
		expect(parser, TAROT_TOK_ASSIGN);
		Variable(node)->value = parse_expression(parser);
		expect(parser, TAROT_TOK_SEMICOLON);
		add_to_current_scope(parser, node);
	}
	return result(parser, node);
}

static struct tarot_node* parse_parameter(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_IDENTIFIER, &token)) {
		node = tarot_create_node(NODE_Parameter, &token.position);
		Parameter(node)->name = token.value.String;
		expect(parser, TAROT_TOK_COLON);
		Parameter(node)->type = parse_datatype(parser);
		if (match(parser, TAROT_TOK_ASSIGN, NULL)) {
			Parameter(node)->value = parse_expression(parser);
		}
		add_to_current_scope(parser, node);
	}
	return result(parser, node);
}

static void index_parameters(struct tarot_node *block) {
	size_t i;
	for (i = 0; i < Block(block)->num_elements; i++) {
		struct tarot_node *parameter = Block(block)->elements[i];
		Parameter(parameter)->index = i;
	}
}

static struct tarot_node* parse_function(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_FUNCTION, &token)) {
		node = tarot_create_node(NODE_Function, &token.position);
		FunctionDefinition(node)->name = read_identifier(parser);
		FunctionDefinition(node)->scope = create_scope();
		enter_node(&parser->scopes, node);
		expect(parser, TAROT_TOK_OPEN_BRACKET);
		FunctionDefinition(node)->parameters = parse_comma_seperated(parser, parse_parameter);
		index_parameters(FunctionDefinition(node)->parameters);
		expect(parser, TAROT_TOK_CLOSE_BRACKET);
		if (match(parser, TAROT_TOK_ARROW, &token)) {
			FunctionDefinition(node)->return_value = parse_datatype(parser);
		}
		FunctionDefinition(node)->block = parse_scoped_block(parser, parse_statement);
		leave_node(&parser->scopes, node);
	}
	return result(parser, node);
}

static struct tarot_node* parse_foreign_parameter(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_IDENTIFIER, &token)) {
		node = tarot_create_node(NODE_Parameter, &token.position);
		Parameter(node)->name = token.value.String;
		expect(parser, TAROT_TOK_COLON);
		Parameter(node)->type = parse_datatype(parser);
		if (match(parser, TAROT_TOK_ASSIGN, NULL)) {
			Parameter(node)->value = parse_expression(parser);
		}
	}
	return result(parser, node);
}

static struct tarot_node* parse_foreign_function(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_FOREIGN_FUNCTION, &token)) {
		node = tarot_create_node(NODE_ForeignFunction, &token.position);
		ForeignFunction(node)->name = read_identifier(parser);
		expect(parser, TAROT_TOK_OPEN_BRACKET);
		ForeignFunction(node)->parameters = parse_comma_seperated(parser, parse_foreign_parameter);
		expect(parser, TAROT_TOK_CLOSE_BRACKET);
		if (match(parser, TAROT_TOK_ARROW, &token)) {
			ForeignFunction(node)->return_value = parse_datatype(parser);
		}
		expect(parser, TAROT_TOK_SEMICOLON);
	}
	return result(parser, node);
}

static struct tarot_node* parse_attribute(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_IDENTIFIER, &token)) {
		node = tarot_create_node(NODE_Variable, &token.position);
		Variable(node)->name = token.value.String;
		if (match(parser, TAROT_TOK_COLON, &token)) {
			Variable(node)->type = parse_datatype(parser);
		}
		if (match(parser, TAROT_TOK_ASSIGN, &token)) {
			Variable(node)->value = parse_expression(parser);
		}
		expect(parser, TAROT_TOK_SEMICOLON);
	}
	return result(parser, node);
}

static struct tarot_node* parse_import(struct tarot_parser *parser);
static struct tarot_node* parse_member(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	(node = parse_definition(parser)) or
	(node = parse_constant(parser))   or
	(node = parse_import(parser))     or
	(node = parse_attribute(parser));
	return result(parser, node);
}

static struct tarot_node* parse_class(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_CLASS, &token)) {
		node = tarot_create_node(NODE_Class, &token.position);
		ClassDefinition(node)->name = read_identifier(parser);
		if (match(parser, TAROT_TOK_OPEN_BRACKET, NULL)) {
			ClassDefinition(node)->extends = parse_comma_seperated(parser, parse_relation);
			expect(parser, TAROT_TOK_CLOSE_BRACKET);
		}
		ClassDefinition(node)->scope = create_scope();
		enter_scope(&parser->scopes, &ClassDefinition(node)->scope);
		ClassDefinition(node)->block = parse_scoped_block(parser, parse_member);
		leave_scope(&parser->scopes, ClassDefinition(node)->scope);
	}
	return result(parser, node);
}

static struct tarot_node* parse_enumerator(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_IDENTIFIER, &token)) {
		node = tarot_create_node(NODE_Enumerator, &token.position);
		Enumerator(node)->name = token.value.String;
	}
	return result(parser, node);
}

static struct tarot_node* parse_enumeration(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_ENUMERATION, &token)) {
		node = tarot_create_node(NODE_Enum, &token.position);
		EnumDefinition(node)->name = read_identifier(parser);
		expect(parser, TAROT_TOK_OPEN_CURLY_BRACKET);
		EnumDefinition(node)->block = parse_comma_seperated(parser, parse_enumerator);
		expect(parser, TAROT_TOK_CLOSE_CURLY_BRACKET);
		/* Assign unique indices to the list of enumerators: */
		if (EnumDefinition(node)->block != NULL) {
			size_t i;
			for (i = 0; i < Block(EnumDefinition(node)->block)->num_elements; i++) {
				struct tarot_node *element = Block(EnumDefinition(node)->block)->elements[i];
				Enumerator(element)->link = node;
				Enumerator(element)->index = i;
			}
		}
	}
	return result(parser, node);
}

static struct tarot_node* parse_namespace(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_NAMESPACE, &token)) {
		node = tarot_create_node(NODE_Namespace, &token.position);
		Namespace(node)->name = read_identifier(parser);
		Namespace(node)->scope = create_scope();
		enter_scope(&parser->scopes, &Namespace(node)->scope);
		Namespace(node)->block = parse_scoped_block(parser, parse_global);
		leave_scope(&parser->scopes, Namespace(node)->scope);
	}
	return result(parser, node);
}

static struct tarot_node* parse_typedef(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_DEFINE_TYPE, &token)) {
		node = tarot_create_node(NODE_Type, &token.position);
		TypeDefinition(node)->name = read_identifier(parser);
		expect(parser, TAROT_TOK_AS);
		TypeDefinition(node)->original = parse_datatype(parser);
		expect(parser, TAROT_TOK_SEMICOLON);
	}
	return result(parser, node);
}

static struct tarot_node* parse_union(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_UNION, &token)) {
		node = tarot_create_node(NODE_Union, &token.position);
		UnionDefinition(node)->name = read_identifier(parser);
		UnionDefinition(node)->block = parse_scoped_block(parser, parse_definition);
	}
	return result(parser, node);
}

static void set_visibility(
	struct tarot_node *node,
	enum tarot_visibility visibility
) {
	switch (kind_of(node)) {
		default:
			assert(false); /* Unhandled node kind */
			tarot_abort();
		case NODE_Class:
			ClassDefinition(node)->visibility = visibility;
			break;
		case NODE_Enum:
			EnumDefinition(node)->visibility = visibility;
			break;
		case NODE_Function:
			FunctionDefinition(node)->visibility = visibility;
			break;
		case NODE_ForeignFunction:
			ForeignFunction(node)->visibility = visibility;
			break;
		case NODE_Namespace:
			Namespace(node)->visibility = visibility;
			break;
		case NODE_TypeDefinition:
			TypeDefinition(node)->visibility = visibility;
			break;
		case NODE_Union:
			UnionDefinition(node)->visibility = visibility;
			break;
	}
}

static struct tarot_node* parse_definition(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	enum tarot_visibility visibility = read_visibility(parser);
	(node = parse_class(parser))            or
	(node = parse_enumeration(parser))      or
	(node = parse_function(parser))         or
	(node = parse_foreign_function(parser)) or
	(node = parse_namespace(parser))        or
	(node = parse_typedef(parser))          or
	(node = parse_union(parser));
	if (node != NULL) {
		add_to_current_scope(parser, node);
		set_visibility(node, visibility);
	} else if (parser->is_calm and visibility != 0) {
		/* Is this ever even triggered? */
		tarot_error_at(
			&parser->current->position,
			"Expected a definition after the visibility decorator '%s'!",
			visibility_string(visibility)
		);
	}
	return result(parser, node);
}

/******************************************************************************
 * MARK: > Stmt
 *****************************************************************************/

static struct tarot_node* parse_assert(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_ASSERT, &token)) {
		node = tarot_create_node(NODE_Assert, &token.position);
		AssertStatement(node)->condition = parse_expression(parser);
		AssertStatement(node)->function = parser->scopes.function;
		expect(parser, TAROT_TOK_SEMICOLON);
	}
	return result(parser, node);
}

static struct tarot_node* parse_raise(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_RAISE, &token)) {
		node = tarot_create_node(NODE_Raise, &token.position);
		RaiseStatement(node)->identififer = parse_primary_expression(parser);
		expect(parser, TAROT_TOK_SEMICOLON);
	}
	return result(parser, node);
}

static struct tarot_node* parse_return(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_RETURN, &token)) {
		node = tarot_create_node(NODE_Return, &token.position);
		ReturnStatement(node)->function = parser->scopes.function;
		ReturnStatement(node)->expression = parse_expression(parser);
		expect(parser, TAROT_TOK_SEMICOLON);
	}
	return result(parser, node);
}

static struct tarot_node* parse_if(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_IF, &token)) {
		node = tarot_create_node(NODE_If, &token.position);
		IfStatement(node)->condition = parse_expression(parser);
		IfStatement(node)->block = parse_scoped_block(parser, parse_statement);
		if (match(parser, TAROT_TOK_ELSE, &token)) {
			(IfStatement(node)->elseif = parse_if(parser)) or
			(IfStatement(node)->elseif = parse_scoped_block(parser, parse_statement));
			if (not require(parser, IfStatement(node)->elseif)) {
				tarot_error_at(&token.position, "else followed by invalid");
			}
		}
	}
	return result(parser, node);
}

static struct tarot_node* parse_while(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_WHILE, &token)) {
		node = tarot_create_node(NODE_While, &token.position);
		WhileLoop(node)->condition = parse_expression(parser);
		WhileLoop(node)->block = parse_scoped_block(parser, parse_statement);
	}
	return result(parser, node);
}

static struct tarot_node* parse_case(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_node *condition = parse_expression(parser);
	if (condition != NULL) {
		node = tarot_create_node(NODE_Case, position_of(condition));
		CaseStatement(node)->condition = condition;
		CaseStatement(node)->block = parse_scoped_block(parser, parse_statement);
	}
	return result(parser, node);
}

static struct tarot_node* parse_match(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_MATCH, &token)) {
		node = tarot_create_node(NODE_Match, &token.position);
		MatchStatement(node)->pattern = parse_expression(parser);
		MatchStatement(node)->block = parse_scoped_block(parser, parse_case);
	}
	return result(parser, node);
}

static struct tarot_node* parse_assign_or_expression(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	struct tarot_node *identifier = parse_primary_expression(parser);
	if (identifier != NULL) {
		if (match(parser, TAROT_TOK_ASSIGN, &token)) {
			node = tarot_create_node(NODE_Assignment, &token.position);
			Assignment(node)->identifier = identifier;
			Assignment(node)->value = parse_expression(parser);
			expect(parser, TAROT_TOK_SEMICOLON);
		} else {
			node = tarot_create_node(NODE_ExpressionStatement, position_of(identifier));
			ExprStatement(node)->expression = identifier;
			expect(parser, TAROT_TOK_SEMICOLON);
		}
	}
	return result(parser, node);
}

static struct tarot_node* parse_catch(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_CATCH, &token)) {
		node = tarot_create_node(NODE_Catch, &token.position);
		CatchStatement(node)->identifiers = parse_comma_seperated(parser, parse_primary_expression);
		CatchStatement(node)->block = parse_scoped_block(parser, parse_statement);
	}
	return result(parser, node);
}

static struct tarot_node* parse_try(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_TRY, &token)) {
		node = tarot_create_node(NODE_Try, &token.position);
		TryStatement(node)->block = parse_scoped_block(parser, parse_statement);
		TryStatement(node)->handlers = parse_block(parser, parse_catch);
	}
	return result(parser, node);
}

static struct tarot_node* parse_import(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (match(parser, TAROT_TOK_FROM, &token)) {
		node = tarot_create_node(NODE_Import, &token.position);
		ImportStatement(node)->path = read_string(parser);
		expect(parser, TAROT_TOK_IMPORT);
		ImportStatement(node)->identifier = parse_relation(parser);
		if (match(parser, TAROT_TOK_AS, NULL)) {
			ImportStatement(node)->alias = read_identifier(parser);
		} else if (kind_of(ImportStatement(node)->identifier) == NODE_Relation) {
			ImportStatement(node)->alias = tarot_copy_string(
				Relation(ImportStatement(node)->identifier)->child
			);
		} else {
			ImportStatement(node)->alias = tarot_copy_string(name_of(ImportStatement(node)->identifier));
		}
		expect(parser, TAROT_TOK_SEMICOLON);
		add_to_current_scope(parser, node);
	}
	return result(parser, node);
}

static struct tarot_node* parse_print(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	struct tarot_token token;
	if (
		match_identifier(parser, "print", &token) or
		match_identifier(parser, "println", &token)
	) {
		node = tarot_create_node(NODE_Print, &token.position);
		expect(parser, TAROT_TOK_OPEN_BRACKET);
		PrintStatement(node)->arguments = parse_expression(parser);
		PrintStatement(node)->newline = tarot_match_string(token.value.String, "println");
		expect(parser, TAROT_TOK_CLOSE_BRACKET);
		expect(parser, TAROT_TOK_SEMICOLON);
		tarot_clear_token(&token);
	}
	return result(parser, node);
}

static struct tarot_node* parse_statement(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	(node = parse_assert(parser))     or
	(node = parse_import(parser))     or
	(node = parse_raise(parser))      or
	(node = parse_return(parser))     or
	(node = parse_if(parser))         or
	(node = parse_while(parser))      or
	(node = parse_match(parser))      or
	(node = parse_try(parser))        or
	(node = parse_print(parser))      or
	(node = parse_constant(parser))   or
	(node = parse_variable(parser))   or
	(node = parse_assign_or_expression(parser));
	return result(parser, node);
}

/******************************************************************************
 * MARK: > Module
 *****************************************************************************/

static struct tarot_node* parse_global(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	(node = parse_import(parser))   or
	(node = parse_constant(parser)) or
	(node = parse_definition(parser));
	return result(parser, node);
}

static struct tarot_node* parse_module(struct tarot_parser *parser) {
	struct tarot_node *node = NULL;
	node = tarot_create_node(NODE_Module, &parser->current->position);
	Module(node)->path = parser->current->position.path;
	Module(node)->scope = create_scope();
	enter_scope(&parser->scopes, &Module(node)->scope);
	Module(node)->block = parse_block(parser, parse_global);
	leave_scope(&parser->scopes, Module(node)->scope);
	Module(node)->num_nodes = tarot_num_nodes();
	expect(parser, TAROT_TOK_EOF);
	return result(parser, node);
}

/******************************************************************************
 * MARK: PUBLIC
 *****************************************************************************/

static void panic(void *data) {
	struct tarot_parser *parser = data;
	parser->is_calm = false;
}

struct tarot_node* tarot_parse(struct tarot_iostream *stream) {
	struct tarot_node *ast = NULL;
	struct tarot_parser parser;
	assert(stream != NULL);
	tarot_log("Parsing source file \"%s\"", tarot_fgetpos(stream)->path);
	tarot_register_error_handler(panic, &parser);
	initialize_parser(&parser, stream);
	ast = parse_module(&parser);
	exit_parser(&parser);
	tarot_register_error_handler(NULL, NULL);
	return ast;
}

struct tarot_node* tarot_parse_text(const char *data) {
	struct tarot_node *ast = NULL;
	struct tarot_iostream *stream;
	assert(data != NULL);
	stream = tarot_fmemopen((char*)data, TAROT_INPUT);
	if (stream != NULL) {
		ast = tarot_parse(stream);
		tarot_fclose(stream);
	}
	return ast;
}

struct tarot_node* tarot_parse_file(const char *path) {
	struct tarot_node *ast = NULL;
	struct tarot_iostream *stream;
	char *pathptr = strdup(path);
	stream = tarot_fopen(pathptr, TAROT_INPUT);
	if (stream != NULL) {
		ast = tarot_parse(stream);
		tarot_fclose(stream);
	} else {
		tarot_free(pathptr);
	}
	return ast;
}
