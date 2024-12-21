#define TAROT_SOURCE
#include "tarot.h"

/******************************************************************************
 * MARK: Visibility
 *****************************************************************************/

static void serialize_visibility(
	struct tarot_iostream *stream,
	enum tarot_visibility visibility
) {
	assert(stream != NULL);
	if (visibility != VISIBILITY_NONE) {
		tarot_fputs(stream, visibility_string(visibility));
		tarot_fputc(stream, ' ');
	}
}

/******************************************************************************
 * MARK: Operators
 *****************************************************************************/

static void serialize_logical_operator(
	struct tarot_iostream *stream,
	enum LogicalExpressionOperator operator
) {
	switch (operator) {
		default:
			tarot_abort();
		case EXPR_AND:
			tarot_fprintf(stream, " %s ", tarot_token_string(TAROT_TOK_AND));
			break;
		case EXPR_OR:
			tarot_fprintf(stream, " %s ", tarot_token_string(TAROT_TOK_OR));
			break;
		case EXPR_XOR:
			tarot_fprintf(stream, " %s ", tarot_token_string(TAROT_TOK_XOR));
			break;
	}
}

static void serialize_relational_operator(
	struct tarot_iostream *stream,
	enum RelationalExpressionOperator operator
) {
	switch (operator) {
		default:
			tarot_abort();
		case EXPR_LESS:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_LESS_THAN));
			break;
		case EXPR_LESS_EQUAL:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_LESS_EQUAL));
			break;
		case EXPR_GREATER:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_GREATER_THAN));
			break;
		case EXPR_GREATER_EQUAL:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_GREATER_EQUAL));
			break;
		case EXPR_EQUAL:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_EQUAL));
			break;
		case EXPR_NOT_EQUAL:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_NOT_EQUAL));
			break;
		case EXPR_IN:
			tarot_fprintf(stream, " %s ", tarot_token_string(TAROT_TOK_IN));
			break;
	}
}

static void serialize_arithmetic_operator(
	struct tarot_iostream *stream,
	enum ArithmeticExpressionOperator operator
) {
	switch (operator) {
		default:
			tarot_abort();
		case EXPR_ADD:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_PLUS));
			break;
		case EXPR_SUBTRACT:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_MINUS));
			break;
		case EXPR_MULTIPLY:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_MULTIPLY));
			break;
		case EXPR_DIVIDE:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_DIVIDE));
			break;
		case EXPR_MODULO:
			tarot_fprintf(stream, " %s ", tarot_token_string(TAROT_TOK_MOD));
			break;
		case EXPR_POWER:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_POWER));
			break;
	}
}

/******************************************************************************
 * MARK: String
 *****************************************************************************/

static void print_string(
	struct tarot_iostream *stream,
	struct tarot_string *string
) {
	tarot_fputc(stream, '"');
	tarot_print_string(stream, string);
	tarot_fputc(stream, '"');
}

static void serialize_string(
	struct tarot_iostream *stream,
	struct tarot_string *string
) {
	size_t available_width = 80 - tarot_stream_indentation(stream) * tarot_stream_indent_width(stream);
	if (tarot_string_length(string) >= available_width) {
		string = tarot_copy_string(string);
		tarot_newline(stream);
		tarot_indent(stream, 1);
		while (tarot_string_length(string) >= available_width) {
			struct tarot_string *slice = tarot_string_split(&string, available_width);
			print_string(stream, slice);
			tarot_newline(stream);
			tarot_free_string(slice);
		}
		print_string(stream, string);
		tarot_indent(stream, -1);
		tarot_newline(stream);
		tarot_free_string(string);
	} else {
		print_string(stream, string);
	}
}

/******************************************************************************
 * MARK: Literal
 *****************************************************************************/

static void serialize_literal(
	struct tarot_iostream *stream,
	struct Literal *node
) {
	switch (node->kind) {
		default:
			break;
		case VALUE_BOOL:
			if (node->value.Boolean) {
				tarot_fputs(stream, tarot_token_string(TAROT_TOK_TRUE));
			} else {
				tarot_fputs(stream, tarot_token_string(TAROT_TOK_FALSE));
			}
			break;
		case VALUE_INTEGER:
			tarot_print_integer(stream, node->value.Integer);
			break;
		case VALUE_FLOAT:
			tarot_fprintf(stream, "%ff", node->value.Float);
			break;
		case VALUE_RATIONAL:
			tarot_print_rational(stream, node->value.Rational);
			break;
		case VALUE_RAW_STRING:
			tarot_fputc(stream, 'r');
			/* Fallthrough */
		case VALUE_STRING:
			serialize_string(stream, node->value.String);
			break;
	}
}

/******************************************************************************
 * MARK: Block
 *****************************************************************************/

static void serialize_list_block(
	struct tarot_iostream *stream,
	struct tarot_node *node
) {
	size_t i;
	bool insert_newlines = Block(node)->num_elements > 3;
	if (insert_newlines) {
		tarot_newline(stream);
		tarot_indent(stream, 1);
	}
	for (i = 0; i < Block(node)->num_elements; i++) {
		tarot_serialize_node(stream, Block(node)->elements[i]);
		if (i+1 < Block(node)->num_elements) {
			tarot_fputs(stream, ", ");
			if (insert_newlines) {
				tarot_newline(stream);
			}
		}
	}
	if (insert_newlines) {
		tarot_indent(stream, -1);
		tarot_newline(stream);
	}
}

static void serialize_default_block(
	struct tarot_iostream *stream,
	struct tarot_node *node
) {
	size_t i;
	for (i = 0; i < Block(node)->num_elements; i++) {
		tarot_serialize_node(stream, Block(node)->elements[i]);
		tarot_newline(stream);
	}
}

static void serialize_scoped_block(
	struct tarot_iostream *stream,
	struct tarot_node *node
) {
	tarot_fputc(stream, '{');
	tarot_newline(stream);
	tarot_indent(stream, 1);
	serialize_default_block(stream, node);
	tarot_indent(stream, -1);
	tarot_fputc(stream, '}');
	tarot_newline(stream);
}

static void serialize_block(
	struct tarot_iostream *stream,
	struct tarot_node *node
) {
	switch (Block(node)->kind) {
		default:
			tarot_abort();
		case BLOCK_DEFAULT:
			serialize_default_block(stream, node);
			break;
		case BLOCK_SCOPED:
			serialize_scoped_block(stream, node);
			break;
		case BLOCK_LIST:
			serialize_list_block(stream, node);
			break;
	}
}

/******************************************************************************
 * MARK: Node
 *****************************************************************************/

void tarot_serialize_node(
	struct tarot_iostream *stream,
	struct tarot_node *node
) {
	switch (kind_of(node)) {
		size_t i;
		case NODE_ERROR:
			tarot_fputs(stream, "ERROR");
			break;
		case NODE_Module:
			tarot_serialize_node(stream, Module(node)->block);
			break;
		case NODE_Block:
			serialize_block(stream, node);
			break;
		case NODE_LogicalExpression:
			tarot_serialize_node(stream, LogicalExpression(node)->left_operand);
			serialize_logical_operator(stream, LogicalExpression(node)->operator);
			tarot_serialize_node(stream, LogicalExpression(node)->right_operand);
			break;
		case NODE_RelationalExpression:
			tarot_serialize_node(stream, RelationalExpression(node)->left_operand);
			serialize_relational_operator(stream, RelationalExpression(node)->operator);
			tarot_serialize_node(stream, RelationalExpression(node)->right_operand);
			break;
		case NODE_ArithmeticExpression:
			tarot_serialize_node(stream, ArithmeticExpression(node)->left_operand);
			serialize_arithmetic_operator(stream, ArithmeticExpression(node)->operator);
			tarot_serialize_node(stream, ArithmeticExpression(node)->right_operand);
			break;
		case NODE_InfixExpression:
			tarot_fputc(stream, '(');
			tarot_serialize_node(stream, InfixExpression(node)->expression);
			tarot_fputc(stream, ')');
			break;
		case NODE_Not:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_NOT));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, NotExpression(node)->expression);
			break;
		case NODE_Neg:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_MINUS));
			tarot_serialize_node(stream, NegExpression(node)->expression);
			break;
		case NODE_Abs:
			tarot_fputs(stream, "abs(");
			tarot_serialize_node(stream, AbsExpression(node)->expression);
			tarot_fputc(stream, ')');
			break;
		case NODE_FunctionCall:
			tarot_serialize_node(stream, FunctionCall(node)->identifier);
			tarot_fputc(stream, '(');
			tarot_serialize_node(stream, FunctionCall(node)->arguments);
			tarot_fputc(stream, ')');
			break;
		case NODE_Relation:
			tarot_serialize_node(stream, Relation(node)->parent);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_DOT));
			tarot_print_string(stream, Relation(node)->child);
			break;
		case NODE_Subscript:
			tarot_serialize_node(stream, Subscript(node)->identifier);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_OPEN_ANGULAR_BRACKET));
			tarot_serialize_node(stream, Subscript(node)->index);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_CLOSE_ANGULAR_BRACKET));
			break;
		case NODE_Pair:
			tarot_serialize_node(stream, Pair(node)->key);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_COLON));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, Pair(node)->value);
			break;
		case NODE_Typecast:
			tarot_fputs(stream, datatype_string(CastExpression(node)->kind));
			tarot_fputc(stream, '(');
			tarot_serialize_node(stream, CastExpression(node)->operand);
			tarot_fputc(stream, ')');
			break;
		case NODE_Literal:
			serialize_literal(stream, Literal(node));
			break;
		case NODE_List:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_OPEN_ANGULAR_BRACKET));
			tarot_newline(stream);
			tarot_indent(stream, 1);
			for (i = 0; i < List(node)->num_elements; i++) {
				tarot_serialize_node(stream, List(node)->elements[i]);
				tarot_fputc(stream, ',');
				tarot_newline(stream);
			}
			tarot_indent(stream, -1);
			tarot_newline(stream);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_CLOSE_ANGULAR_BRACKET));
			break;
		case NODE_Dict:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_OPEN_CURLY_BRACKET));
			tarot_newline(stream);
			tarot_indent(stream, 1);
			for (i = 0; i < Dict(node)->num_elements; i++) {
				tarot_serialize_node(stream, Dict(node)->elements[i]);
				tarot_fputc(stream, ',');
				tarot_newline(stream);
			}
			tarot_indent(stream, -1);
			tarot_newline(stream);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_CLOSE_CURLY_BRACKET));
			break;
		case NODE_FString:
			tarot_fputs(stream, "f\"");
			for (i = 0; i < FString(node)->num_elements; i++) {
				tarot_serialize_node(stream, FString(node)->elements[i]);
			}
			tarot_fputc(stream, '"');
			break;
		case NODE_FStringString:
			tarot_print_string(stream, FStringString(node)->value);
			break;
		case NODE_FStringExpression:
			tarot_fputc(stream, '{');
			tarot_serialize_node(stream, FStringExpression(node)->expression);
			tarot_fputc(stream, '}');
			break;
		case NODE_Identifier:
			tarot_print_string(stream, Identifier(node)->name);
			break;
		case NODE_Enumerator:
			tarot_print_string(stream, Enumerator(node)->name);
			break;
		case NODE_Type:
			if (Type(node)->identifier != NULL) {
				tarot_serialize_node(stream, Type(node)->identifier);
			} else {
				tarot_fputs(stream, datatype_string(Type(node)->type));
			}
			if (Type(node)->subtype != NULL) {
				tarot_fputc(stream, '[');
				tarot_serialize_node(stream, Type(node)->subtype);
				tarot_fputc(stream, ']');
			}
			break;
		case NODE_Import:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_FROM));
			tarot_fputs(stream, " \"");
			tarot_print_string(stream, ImportStatement(node)->path);
			tarot_fputs(stream, "\" ");
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_IMPORT));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, ImportStatement(node)->identifier);
			if (ImportStatement(node)->alias) {
				tarot_fputc(stream, ' ');
				tarot_fputs(stream, tarot_token_string(TAROT_TOK_AS));
				tarot_fputc(stream, ' ');
				tarot_print_string(stream, ImportStatement(node)->alias);
			}
			tarot_fputc(stream, ';');
			tarot_newline(stream);
			break;
		case NODE_If:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_IF));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, IfStatement(node)->condition);
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, IfStatement(node)->block);
			if (IfStatement(node)->elseif != NULL) {
				tarot_fputc(stream, ' ');
				tarot_fputs(stream, tarot_token_string(TAROT_TOK_ELSE));
				tarot_fputc(stream, ' ');
				tarot_serialize_node(stream, IfStatement(node)->elseif);
			}
			break;
		case NODE_While:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_WHILE));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, WhileLoop(node)->condition);
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, WhileLoop(node)->block);
			break;
		case NODE_Match:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_MATCH));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, MatchStatement(node)->pattern);
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, MatchStatement(node)->block);
			break;
		case NODE_Case:
			tarot_serialize_node(stream, CaseStatement(node)->condition);
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, CaseStatement(node)->block);
			break;
		case NODE_Assignment:
			tarot_serialize_node(stream, Assignment(node)->identifier);
			tarot_fputs(stream, " = ");
			tarot_serialize_node(stream, Assignment(node)->value);
			tarot_fputc(stream, ';');
			tarot_newline(stream);
			break;
		case NODE_ExpressionStatement:
			tarot_serialize_node(stream, ExprStatement(node)->expression);
			tarot_fputc(stream, ';');
			break;
		case NODE_Print:
			if (PrintStatement(node)->newline) {
				tarot_fputs(stream, "println");
			} else {
				tarot_fputs(stream, "print");
			}
			tarot_fputc(stream, '(');
			tarot_serialize_node(stream, PrintStatement(node)->arguments);
			tarot_fputc(stream, ')');
			break;
		case NODE_Try:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_TRY));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, TryStatement(node)->block);
			tarot_serialize_node(stream, TryStatement(node)->handlers);
			break;
		case NODE_Catch:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_CATCH));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, CatchStatement(node)->identifiers);
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, CatchStatement(node)->block);
			break;
		case NODE_Raise:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_RAISE));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, RaiseStatement(node)->identififer);
			tarot_fputs(stream, ";\n");
			break;
		case NODE_Assert:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_ASSERT));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, AssertStatement(node)->condition);
			tarot_fputs(stream, ";\n");
			break;
		case NODE_Class:
			serialize_visibility(stream, ClassDefinition(node)->visibility);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_CLASS));
			tarot_fputc(stream, ' ');
			tarot_print_string(stream, ClassDefinition(node)->name);
			if (ClassDefinition(node)->extends != NULL) {
				tarot_fputc(stream, '(');
				tarot_serialize_node(stream, ClassDefinition(node)->extends);
				tarot_fputc(stream, ')');
			}
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, ClassDefinition(node)->block);
			break;
		case NODE_Enum:
			serialize_visibility(stream, EnumDefinition(node)->visibility);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_ENUMERATION));
			tarot_fputc(stream, ' ');
			tarot_print_string(stream, EnumDefinition(node)->name);
			tarot_fputc(stream, ' ');
			tarot_fputc(stream, '{');
			tarot_newline(stream);
			tarot_indent(stream, 1);
			tarot_serialize_node(stream, EnumDefinition(node)->block);
			tarot_indent(stream, -1);
			tarot_newline(stream);
			tarot_fputc(stream, '}');
			tarot_newline(stream);
			break;
		case NODE_Function:
			serialize_visibility(stream, FunctionDefinition(node)->visibility);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_FUNCTION));
			tarot_fputc(stream, ' ');
			tarot_print_string(stream, FunctionDefinition(node)->name);
			tarot_fputc(stream, '(');
			tarot_serialize_node(stream, FunctionDefinition(node)->parameters);
			tarot_fputc(stream, ')');
			if (FunctionDefinition(node)->return_value) {
				tarot_fputc(stream, ' ');
				tarot_fputs(stream, tarot_token_string(TAROT_TOK_ARROW));
				tarot_fputc(stream, ' ');
				tarot_serialize_node(stream, FunctionDefinition(node)->return_value);
				tarot_fputc(stream, ' ');
			}
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, FunctionDefinition(node)->block);
			break;
		case NODE_ForeignFunction:
			serialize_visibility(stream, ForeignFunction(node)->visibility);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_FOREIGN_FUNCTION));
			tarot_fputc(stream, ' ');
			tarot_print_string(stream, ForeignFunction(node)->name);
			tarot_fputc(stream, '(');
			tarot_serialize_node(stream, ForeignFunction(node)->parameters);
			tarot_fputc(stream, ')');
			tarot_fputc(stream, ';');
			tarot_newline(stream);
			break;
		case NODE_Namespace:
			serialize_visibility(stream, Namespace(node)->visibility);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_NAMESPACE));
			tarot_fputc(stream, ' ');
			tarot_print_string(stream, Namespace(node)->name);
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, Namespace(node)->block);
			break;
		case NODE_TypeDefinition:
			serialize_visibility(stream, TypeDefinition(node)->visibility);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_DEFINE_TYPE));
			tarot_fputc(stream, ' ');
			tarot_print_string(stream, TypeDefinition(node)->name);
			tarot_fputc(stream, ' ');
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_AS));
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, TypeDefinition(node)->original);
			tarot_fputc(stream, ';');
			break;
		case NODE_Union:
			serialize_visibility(stream, UnionDefinition(node)->visibility);
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_UNION));
			tarot_fputc(stream, ' ');
			tarot_print_string(stream, UnionDefinition(node)->name);
			tarot_fputc(stream, ' ');
			tarot_serialize_node(stream, UnionDefinition(node)->block);
			break;
		case NODE_Variable:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_LET));
			tarot_fputc(stream, ' ');
			tarot_print_string(stream, Variable(node)->name);
			if (Variable(node)->type != NULL) {
				tarot_fputs(stream, ": ");
				tarot_serialize_node(stream, Variable(node)->type);
			}
			if (Variable(node)->value != NULL) {
				tarot_fputs(stream, " = ");
				tarot_serialize_node(stream, Variable(node)->value);
			}
			tarot_fputc(stream, ';');
			break;
		case NODE_Constant:
			tarot_fputs(stream, tarot_token_string(TAROT_TOK_CONSTANT));
			tarot_fputc(stream, ' ');
			tarot_print_string(stream, Variable(node)->name);
			if (Variable(node)->type != NULL) {
				tarot_fputs(stream, ": ");
				tarot_serialize_node(stream, Variable(node)->type);
			}
			if (Variable(node)->value != NULL) {
				tarot_fputs(stream, " = ");
				tarot_serialize_node(stream, Variable(node)->value);
			}
			tarot_fputc(stream, ';');
			break;
		case NODE_Parameter:
			tarot_print_string(stream, Parameter(node)->name);
			tarot_fputs(stream, ": ");
			tarot_serialize_node(stream, Parameter(node)->type);
			if (Parameter(node)->value != NULL) {
				tarot_fputs(stream, " = ");
				tarot_serialize_node(stream, Parameter(node)->value);
			}
			break;
	}
}

/******************************************************************************
 * MARK: Stringize
 *****************************************************************************/

struct tarot_string* tarot_stringize_node(struct tarot_node *node) {
	struct tarot_string *string = tarot_create_string(NULL);
	struct tarot_iostream *stream;
	assert(node != NULL);
	stream = tarot_fstropen(&string, TAROT_OUTPUT);
	if (stream != NULL) {
		tarot_serialize_node(stream, node);
		tarot_fclose(stream);
	}
	return string;
}
