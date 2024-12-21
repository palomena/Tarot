#define TAROT_SOURCE
#include "tarot.h"

/******************************************************************************
 * MARK: Arithmetics
 *****************************************************************************/

static double compute_floats(
	enum ArithmeticExpressionOperator operator,
	double a,
	double b
) {
	switch (operator) {
		default:
			tarot_abort();
		case EXPR_ADD:
			return a + b;
		case EXPR_SUBTRACT:
			return a - b;
		case EXPR_MULTIPLY:
			return a * b;
		case EXPR_DIVIDE:
			return a / b;
		case EXPR_POWER:
			return pow(a, b);
		case EXPR_MODULO:
			return modf(a, &b);
	}
}

static struct tarot_node* simplify_float_arithmetics(struct tarot_node *node) {
	struct tarot_node *a = ArithmeticExpression(node)->left_operand;
	struct tarot_node *b = ArithmeticExpression(node)->right_operand;
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(node));
	Literal(result)->kind = VALUE_FLOAT;
	Literal(result)->type = TYPE_FLOAT;
	Literal(result)->value.Float = compute_floats(
		ArithmeticExpression(node)->operator,
		Literal(a)->value.Float,
		Literal(b)->value.Float
	);
	return result;
}

static tarot_integer* compute_integers(
	enum ArithmeticExpressionOperator operator,
	tarot_integer *a,
	tarot_integer *b
) {
	switch (operator) {
		default:
			tarot_abort();
		case EXPR_ADD:
			return tarot_add_integers(a, b);
		case EXPR_SUBTRACT:
			return tarot_subtract_integers(a, b);
		case EXPR_MULTIPLY:
			return tarot_multiply_integers(a, b);
		case EXPR_DIVIDE:
			return tarot_divide_integers(a, b);
		case EXPR_POWER:
			return tarot_exponentiate_integers(a, b);
		case EXPR_MODULO:
			return tarot_modulo_integers(a, b);
	}
}

static struct tarot_node* simplify_integer_arithmetics(struct tarot_node *node) {
	struct tarot_node *a = ArithmeticExpression(node)->left_operand;
	struct tarot_node *b = ArithmeticExpression(node)->right_operand;
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(node));
	Literal(result)->kind = VALUE_INTEGER;
	Literal(result)->type = TYPE_INTEGER;
	Literal(result)->value.Integer = compute_integers(
		ArithmeticExpression(node)->operator,
		Literal(a)->value.Integer,
		Literal(b)->value.Integer
	);
	return result;
}

static tarot_rational* compute_rationals(
	enum ArithmeticExpressionOperator operator,
	tarot_rational *a,
	tarot_rational *b
) {
	switch (operator) {
		default:
			tarot_abort();
		case EXPR_ADD:
			return tarot_add_rationals(a, b);
		case EXPR_SUBTRACT:
			return tarot_subtract_rationals(a, b);
		case EXPR_MULTIPLY:
			return tarot_multiply_rationals(a, b);
		case EXPR_DIVIDE:
			return tarot_divide_rationals(a, b);
		case EXPR_POWER:
			return tarot_exponentiate_rationals(a, b);
		case EXPR_MODULO:
			tarot_abort();
	}
}

static struct tarot_node* simplify_rational_arithmetics(struct tarot_node *node) {
	struct tarot_node *a = ArithmeticExpression(node)->left_operand;
	struct tarot_node *b = ArithmeticExpression(node)->right_operand;
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(node));
	Literal(result)->kind = VALUE_RATIONAL;
	Literal(result)->type = TYPE_RATIONAL;
	Literal(result)->value.Rational = compute_rationals(
		ArithmeticExpression(node)->operator,
		Literal(a)->value.Rational,
		Literal(b)->value.Rational
	);
	return result;
}

static struct tarot_node* simplify_arithmetic_expression(struct tarot_node *node) {
	switch (Type(type_of(node))->type) {
		default:
			return node;
		case TYPE_FLOAT:
			return simplify_float_arithmetics(node);
		case TYPE_INTEGER:
			return simplify_integer_arithmetics(node);
		case TYPE_RATIONAL:
			return simplify_rational_arithmetics(node);
	}
}

/******************************************************************************
 * MARK: Relations
 *****************************************************************************/

static bool evaluate_relation(
	enum RelationalExpressionOperator operator,
	int relation
) {
	switch (operator) {
		default:
			tarot_abort();
		case EXPR_GREATER:
			return relation > 0;
		case EXPR_LESS:
			return relation < 0;
		case EXPR_GREATER_EQUAL:
			return relation >= 0;
		case EXPR_LESS_EQUAL:
			return relation <= 0;
		case EXPR_EQUAL:
			return relation == 0;
		case EXPR_NOT_EQUAL:
			return relation != 0;
	}
}

static int compare_floats(double a, double b) {
	/* FIXME */
	return a - b;
}

static struct tarot_node* simplify_float_relation(struct tarot_node *node) {
	struct tarot_node *a = RelationalExpression(node)->left_operand;
	struct tarot_node *b = RelationalExpression(node)->right_operand;
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(node));
	int relation = compare_floats(Literal(a)->value.Float, Literal(b)->value.Float);
	Literal(result)->kind = VALUE_BOOL;
	Literal(result)->type = TYPE_BOOLEAN;
	Literal(result)->value.Boolean = evaluate_relation(RelationalExpression(node)->operator, relation);
	return result;
}

static struct tarot_node* simplify_integer_relation(struct tarot_node *node) {
	struct tarot_node *a = RelationalExpression(node)->left_operand;
	struct tarot_node *b = RelationalExpression(node)->right_operand;
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(node));
	int relation = tarot_compare_integers(Literal(a)->value.Integer, Literal(b)->value.Integer);
	Literal(result)->kind = VALUE_BOOL;
	Literal(result)->type = TYPE_BOOLEAN;
	Literal(result)->value.Boolean = evaluate_relation(RelationalExpression(node)->operator, relation);
	return result;
}

static struct tarot_node* simplify_rational_relation(struct tarot_node *node) {
	struct tarot_node *a = RelationalExpression(node)->left_operand;
	struct tarot_node *b = RelationalExpression(node)->right_operand;
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(node));
	int relation = tarot_compare_rationals(Literal(a)->value.Rational, Literal(b)->value.Rational);
	Literal(result)->kind = VALUE_BOOL;
	Literal(result)->type = TYPE_BOOLEAN;
	Literal(result)->value.Boolean = evaluate_relation(RelationalExpression(node)->operator, relation);
	return result;
}

static bool compare_strings(
	enum RelationalExpressionOperator operator,
	struct tarot_string *a,
	struct tarot_string *b
) {
	switch (operator) {
		default:
			tarot_abort();
		case EXPR_EQUAL:
			return tarot_compare_strings(a, b);
		case EXPR_NOT_EQUAL:
			return not tarot_compare_strings(a, b);
	}
}

static struct tarot_node* simplify_string_relation(struct tarot_node *node) {
	struct tarot_node *a = RelationalExpression(node)->left_operand;
	struct tarot_node *b = RelationalExpression(node)->right_operand;
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(node));
	Literal(result)->kind = VALUE_BOOL;
	Literal(result)->type = TYPE_BOOLEAN;
	Literal(result)->value.Boolean = compare_strings(
		RelationalExpression(node)->operator,
		Literal(a)->value.String,
		Literal(b)->value.String
	);
	return result;
}

static struct tarot_node* simplify_relational_expression(struct tarot_node *node) {
	switch (Type(type_of(RelationalExpression(node)->right_operand))->type) {
		default:
			return node;
		case TYPE_INTEGER:
			return simplify_integer_relation(node);
		case TYPE_FLOAT:
			return simplify_float_relation(node);
		case TYPE_RATIONAL:
			return simplify_rational_relation(node);
		case TYPE_STRING:
			return simplify_string_relation(node);
	}
}

/******************************************************************************
 * MARK: Logical
 *****************************************************************************/

static bool evaluate_logical(
	enum LogicalExpressionOperator operator,
	bool a, bool b
) {
	switch (operator) {
		default:
			tarot_abort();
		case EXPR_AND:
			return a and b;
		case EXPR_OR:
			return a or b;
		case EXPR_XOR:
			return (not a and b) or (a and not b);
	}
}

static struct tarot_node* simplify_logical_expression(struct tarot_node *node) {
	struct tarot_node *a = LogicalExpression(node)->left_operand;
	struct tarot_node *b = LogicalExpression(node)->right_operand;
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(node));
	Literal(result)->kind = VALUE_BOOL;
	Literal(result)->type = TYPE_BOOLEAN;
	Literal(result)->value.Boolean = evaluate_logical(
		LogicalExpression(node)->operator,
		Literal(a)->value.Boolean,
		Literal(b)->value.Boolean
	);
	return result;
}

/******************************************************************************
 * MARK: TypeCasts
 *****************************************************************************/

static struct tarot_node* cast_to_boolean(struct tarot_node *operand) {
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(operand));
	bool value;
	Literal(result)->kind = VALUE_BOOL;
	Literal(result)->type = TYPE_BOOLEAN;
	switch (Literal(operand)->type) {
		default:
			tarot_abort();
		case TYPE_BOOLEAN:
			value = Literal(operand)->value.Boolean;
			break;
		case TYPE_STRING:
			value = tarot_match_string(Literal(operand)->value.String, "true");
			break;
	}
	Literal(result)->value.Boolean = value;
	return result;
}

static struct tarot_node* cast_to_float(struct tarot_node *operand) {
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(operand));
	double value;
	Literal(result)->kind = VALUE_FLOAT;
	Literal(result)->type = TYPE_FLOAT;
	switch (Literal(operand)->type) {
		default:
			tarot_abort();
		case TYPE_FLOAT:
			value = Literal(operand)->value.Float;
			break;
		case TYPE_INTEGER:
			value = tarot_integer_to_float(Literal(operand)->value.Integer);
			break;
		case TYPE_RATIONAL:
			value = tarot_rational_to_float(Literal(operand)->value.Rational);
			break;
		case TYPE_STRING:
			value = strtod(tarot_string_text(Literal(operand)->value.String), NULL);
			break;
	}
	Literal(result)->value.Float = value;
	return result;
}

static struct tarot_node* cast_to_integer(struct tarot_node *operand) {
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(operand));
	struct tarot_integer *value = NULL;
	Literal(result)->kind = VALUE_INTEGER;
	Literal(result)->type = TYPE_INTEGER;
	switch (Literal(operand)->type) {
		default:
			tarot_abort();
		case TYPE_FLOAT:
			value = tarot_create_integer_from_float(Literal(operand)->value.Float);
			break;
		case TYPE_INTEGER:
			value = tarot_copy_integer(Literal(operand)->value.Integer);
			break;
		case TYPE_RATIONAL:
			value = tarot_create_integer_from_rational(Literal(operand)->value.Rational);
			break;
		case TYPE_STRING:
			value = tarot_create_integer_from_string(Literal(operand)->value.String, 10);
			break;
	}
	Literal(result)->value.Integer = value;
	return result;
}

static struct tarot_node* cast_to_rational(struct tarot_node *operand) {
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(operand));
	struct tarot_rational *value = NULL;
	Literal(result)->kind = VALUE_RATIONAL;
	Literal(result)->type = TYPE_RATIONAL;
	switch (Literal(operand)->type) {
		default:
			tarot_abort();
		case TYPE_FLOAT:
			value = tarot_create_rational_from_float(Literal(operand)->value.Float);
			break;
		case TYPE_INTEGER:
			value = tarot_create_rational_from_integer(Literal(operand)->value.Integer);
			break;
		case TYPE_RATIONAL:
			value = tarot_copy_rational(Literal(operand)->value.Rational);
			break;
		case TYPE_STRING:
			value = tarot_create_rational_from_string(Literal(operand)->value.String);
			break;
	}
	Literal(result)->value.Rational = value;
	return result;
}

static struct tarot_node* cast_to_string(struct tarot_node *operand) {
	struct tarot_node *result = tarot_create_node(NODE_Literal, position_of(operand));
	struct tarot_string *value = NULL;
	Literal(result)->kind = VALUE_STRING;
	Literal(result)->type = TYPE_STRING;
	switch (Literal(operand)->type) {
		default:
			tarot_abort();
		case TYPE_BOOLEAN:
			value = tarot_create_string(tarot_bool_string(Literal(operand)->value.Boolean));
			break;
		case TYPE_FLOAT:
			value = tarot_create_string("%f", Literal(operand)->value.Float);
			break;
		case TYPE_INTEGER:
			value = tarot_integer_to_string(Literal(operand)->value.Integer);
			break;
		case TYPE_RATIONAL:
			value = tarot_rational_to_string(Literal(operand)->value.Rational);
			break;
		case TYPE_STRING:
			value = tarot_copy_string(Literal(operand)->value.String);
			break;
	}
	Literal(result)->value.String = value;
	return result;
}

static struct tarot_node* simplify_typecast(struct tarot_node *node) {
	switch (CastExpression(node)->kind) {
		default:
			tarot_abort();
		case TYPE_BOOLEAN:
			return cast_to_boolean(CastExpression(node)->operand);
		case TYPE_FLOAT:
			return cast_to_float(CastExpression(node)->operand);
		case TYPE_INTEGER:
			return cast_to_integer(CastExpression(node)->operand);
		case TYPE_RATIONAL:
			return cast_to_rational(CastExpression(node)->operand);
		case TYPE_STRING:
			return cast_to_string(CastExpression(node)->operand);
	}
}

/******************************************************************************
 * MARK: abs
 *****************************************************************************/

/* TODO: Move and make public for parser, etc */
static struct tarot_node* create_float(
	struct tarot_stream_position *position,
	double value
) {
	struct tarot_node *node = tarot_create_node(NODE_Literal, position);
	Literal(node)->kind = VALUE_FLOAT;
	Literal(node)->type = TYPE_FLOAT;
	Literal(node)->value.Float = value;
	return node;
}

static struct tarot_node* create_integer(
	struct tarot_stream_position *position,
	tarot_integer *value
) {
	struct tarot_node *node = tarot_create_node(NODE_Literal, position);
	Literal(node)->kind = VALUE_INTEGER;
	Literal(node)->type = TYPE_INTEGER;
	Literal(node)->value.Integer = value;
	return node;
}

static struct tarot_node* create_rational(
	struct tarot_stream_position *position,
	tarot_rational *value
) {
	struct tarot_node *node = tarot_create_node(NODE_Literal, position);
	Literal(node)->kind = VALUE_RATIONAL;
	Literal(node)->type = TYPE_RATIONAL;
	Literal(node)->value.Rational = value;
	return node;
}

/* FIXME: Assumes literal as argument, when in fact it could be NegExpression! */
static struct tarot_node* simplify_abs(struct tarot_node *node) {
	switch (Type(type_of(node))->type) {
		default:
			tarot_abort();
		case TYPE_FLOAT:
			return create_float(
				position_of(node),
				fabs(Literal(AbsExpression(node)->expression)->value.Float)
			);
		case TYPE_INTEGER:
			return create_integer(
				position_of(node),
				tarot_integer_abs(Literal(AbsExpression(node)->expression)->value.Integer)
			);
		case TYPE_RATIONAL:
			return create_rational(
				position_of(node),
				tarot_rational_abs(Literal(AbsExpression(node)->expression)->value.Rational)
			);
	}
}

/******************************************************************************
 * MARK: neg
 *****************************************************************************/

static struct tarot_node* simplify_neg(struct tarot_node *node) {
	switch (Type(type_of(node))->type) {
		default:
			tarot_abort();
		case TYPE_FLOAT:
			return create_float(
				position_of(node),
				-Literal(NegExpression(node)->expression)->value.Float
			);
		case TYPE_INTEGER:
			return create_integer(
				position_of(node),
				tarot_integer_negate(Literal(NegExpression(node)->expression)->value.Integer)
			);
		case TYPE_RATIONAL:
			return create_rational(
				position_of(node),
				tarot_rational_negate(Literal(NegExpression(node)->expression)->value.Rational)
			);
	}
}

/******************************************************************************
 * MARK: Simplify
 *****************************************************************************/

static struct tarot_node* simplify(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			return node;
		case NODE_ArithmeticExpression:
			return simplify_arithmetic_expression(node);
		case NODE_LogicalExpression:
			return simplify_logical_expression(node);
		case NODE_RelationalExpression:
			return simplify_relational_expression(node);
		case NODE_InfixExpression:
			return tarot_copy_node(InfixExpression(node)->expression);
		case NODE_Typecast:
			return simplify_typecast(node);
		case NODE_Abs:
			return simplify_abs(node);
		case NODE_Neg:
			return simplify_neg(node);
	}
}

void simplify_nodeptr(struct tarot_node **node) {
	if (is_constant(*node)) {
		struct tarot_node *result = simplify(*node);
		if (result != *node) {
			tarot_free_node(*node);
			*node = result;
		}
	}
}
