#define TAROT_SOURCE
#include "tarot.h"

const char* tarot_token_string(enum tarot_token_kind kind) {
	static const char* names[] = {
		/* Sections */
		"EOF", "Comment", "Textblock",
		/* Keywords */
		NULL,
		"and", "as", "assert", "break", "catch", "class", "constant",
		"define_type", "else", "enumeration", "false", "for", "from",
		"foreign_function", "function", "if", "import", "in", "__init__", "is",
		"launch", "match", "mod", "namespace", "not", "operator", "or",
		"alias", "private", "public", "raise", "return", "self", "switch",
		"true", "try", "union", "let", "while", "xor",
		NULL,
		/* Operators */
		"+", "-", "*", "/", "=", "+=", "-=", "*=", "/=",
		"**", ">", "<", ">=", "<=", "==", "!=",
		/* Seperators and punctuators */
		"(", ")", "{", "}", "[", "]", ",", ":", ";", ".", "->",
		/* Literals */
		"string", "formatted string", "raw string",
		"integer", "short", "float", "rational", "bool", "identifier"
	};
	return names[kind];
}

const char* tarot_token_class(enum tarot_token_kind kind) {
	switch (kind) {
		default:
			assert(false);
			return NULL;
		case TAROT_TOK_EOF:
		case TAROT_TOK_COMMENT:
		case TAROT_TOK_TEXTBLOCK:
			return "section";
		/* Keywords */
		case TAROT_KEYWORD_START:
		case TAROT_TOK_AND:
		case TAROT_TOK_AS:
		case TAROT_TOK_ASSERT:
		case TAROT_TOK_BREAK:
		case TAROT_TOK_CATCH:
		case TAROT_TOK_CLASS:
		case TAROT_TOK_CONSTANT:
		case TAROT_TOK_DEFINE_TYPE:
		case TAROT_TOK_ELSE:
		case TAROT_TOK_ENUMERATION:
		case TAROT_TOK_FALSE:
		case TAROT_TOK_FOR:
		case TAROT_TOK_FROM:
		case TAROT_TOK_FOREIGN_FUNCTION:
		case TAROT_TOK_FUNCTION:
		case TAROT_TOK_IF:
		case TAROT_TOK_IMPORT:
		case TAROT_TOK_IN:
		case TAROT_TOK_INIT:
		case TAROT_TOK_IS:
		case TAROT_TOK_LAUNCH:
		case TAROT_TOK_MATCH:
		case TAROT_TOK_MOD:
		case TAROT_TOK_NAMESPACE:
		case TAROT_TOK_NOT:
		case TAROT_TOK_OPERATOR:
		case TAROT_TOK_OR:
		case TAROT_TOK_ALIAS:
		case TAROT_TOK_PRIVATE:
		case TAROT_TOK_PUBLIC:
		case TAROT_TOK_RAISE:
		case TAROT_TOK_RETURN:
		case TAROT_TOK_SELF:
		case TAROT_TOK_SWITCH:
		case TAROT_TOK_TRUE:
		case TAROT_TOK_TRY:
		case TAROT_TOK_UNION:
		case TAROT_TOK_LET:
		case TAROT_TOK_WHILE:
		case TAROT_TOK_XOR:
		case TAROT_KEYWORD_END:
			return "keyword";
		/* Operators */
		case TAROT_TOK_PLUS:
		case TAROT_TOK_MINUS:
		case TAROT_TOK_MULTIPLY:
		case TAROT_TOK_DIVIDE:
		case TAROT_TOK_ASSIGN:
		case TAROT_TOK_ASSIGN_ADD:
		case TAROT_TOK_ASSIGN_SUB:
		case TAROT_TOK_ASSIGN_MUL:
		case TAROT_TOK_ASSIGN_DIV:
		case TAROT_TOK_POWER:
		case TAROT_TOK_GREATER_THAN:
		case TAROT_TOK_LESS_THAN:
		case TAROT_TOK_GREATER_EQUAL:
		case TAROT_TOK_LESS_EQUAL:
		case TAROT_TOK_EQUAL:
		case TAROT_TOK_NOT_EQUAL:
			return "operator";
		/* Seperators and punctuators */
		case TAROT_TOK_OPEN_BRACKET:
		case TAROT_TOK_CLOSE_BRACKET:
		case TAROT_TOK_OPEN_CURLY_BRACKET:
		case TAROT_TOK_CLOSE_CURLY_BRACKET:
		case TAROT_TOK_OPEN_ANGULAR_BRACKET:
		case TAROT_TOK_CLOSE_ANGULAR_BRACKET:
		case TAROT_TOK_COMMA:
		case TAROT_TOK_COLON:
		case TAROT_TOK_SEMICOLON:
		case TAROT_TOK_DOT:
		case TAROT_TOK_ARROW:
			return "punctuator";
		/* Literals */
		case TAROT_TOK_STRING:
		case TAROT_TOK_FSTRING:
		case TAROT_TOK_RSTRING:
		case TAROT_TOK_INTEGER:
		case TAROT_TOK_SHORT:
		case TAROT_TOK_FLOAT:
		case TAROT_TOK_RATIONAL:
		case TAROT_TOK_BOOLEAN:
		case TAROT_TOK_IDENTIFIER:
			return "literal";
	}
}

void tarot_clear_token(struct tarot_token *token) {
	assert(token != NULL);
	assert(token->kind >= 0);
	switch (token->kind) {
		default:
			break;
		case TAROT_TOK_IDENTIFIER:
		case TAROT_TOK_STRING:
		case TAROT_TOK_FSTRING:
		case TAROT_TOK_RSTRING:
		case TAROT_TOK_TEXTBLOCK:
			tarot_free(token->value.String);
			break;
		case TAROT_TOK_INTEGER:
			tarot_free_integer(token->value.Integer);
			break;
		case TAROT_TOK_RATIONAL:
			tarot_free_rational(token->value.Rational);
			break;
	}
}
