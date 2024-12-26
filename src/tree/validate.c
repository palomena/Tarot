#define TAROT_SOURCE
#include "tarot.h"

/******************************************************************************
 * MARK: Utils
 *****************************************************************************/

static void print_name(struct tarot_string *name) {
	tarot_format(tarot_stderr, TAROT_COLOR_YELLOW);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_print_string(tarot_stderr, name);
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
}

static void print_node(struct tarot_node *node) {
	tarot_format(tarot_stderr, TAROT_COLOR_YELLOW);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_serialize_node(tarot_stderr, node);
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
}

static void print_text(const char *text) {
	tarot_fputs(tarot_stderr, text);
}

static void print_value(
	struct tarot_iostream *stream,
	const char *format,
	...
) {
	va_list ap;
	tarot_format(stream, TAROT_COLOR_YELLOW);
	tarot_format(stream, TAROT_COLOR_BOLD);
	va_start(ap, format);
	tarot_vfprintf(stream, format, &ap);
	va_end(ap);
	tarot_format(stream, TAROT_COLOR_RESET);
}

static void raise_type_error(
	struct tarot_stream_position *position,
	struct tarot_node *a,
	struct tarot_node *b
) {
	tarot_begin_error(position);
	tarot_fputs(tarot_stderr, "Type mismatch between ");
	print_node(a);
	tarot_fputs(tarot_stderr, " and ");
	print_node(b);
	tarot_fputs(tarot_stderr, "!\n");
	tarot_fputs(tarot_stderr, "Type ");
	print_node(type_of(a));
	tarot_fputs(tarot_stderr, " is incompatible with type ");
	print_node(type_of(b));
	tarot_fputs(tarot_stderr, " !");
	tarot_end_error();
}

static bool compare_types(struct tarot_node *a, struct tarot_node *b) {
	if (a == NULL and b == NULL) {
		return true;
	}
	if (kind_of(a) == NODE_ERROR or kind_of(b) == NODE_ERROR) {
		return false;
	}
	if (Type(a)->type != Type(b)->type) {
		return false;
	}
	if (Type(a)->type == TYPE_CUSTOM) {
		if (
			definition_of(type_of(definition_of(Type(a)->identifier)))
			!=
			definition_of(type_of(definition_of(Type(b)->identifier)))
		) {
			return false;
		}
	}
	return compare_types(Type(a)->subtype, Type(b)->subtype);
}

/******************************************************************************
 * MARK: Functioncall
 *****************************************************************************/

static void raise_invalid_argument_count_error(
	struct tarot_stream_position *position,
	struct tarot_string *function_name,
	size_t num_parameters,
	size_t num_arguments
) {
	tarot_begin_error(position);
	tarot_fputs(tarot_stderr, "Invalid argument count: Function ");
	print_name(function_name);
	tarot_fputs(tarot_stderr, " expects ");
	print_value(tarot_stderr, "%zu", num_parameters);
	tarot_fputs(tarot_stderr, " arguments but received ");
	print_value(tarot_stderr, "%zu", num_arguments);
	tarot_fputs(tarot_stderr, " instead!");
	tarot_end_error();
}

static bool validate_functioncall(struct tarot_node *node) {
	struct tarot_node *function = definition_of(node);
	struct tarot_node *parameters;
	struct tarot_node *arguments = FunctionCall(node)->arguments;
	size_t i;
	if (kind_of(function) == NODE_Function) {
		parameters = FunctionDefinition(function)->parameters;
	} else if (kind_of(function) == NODE_ForeignFunction) {
		parameters = ForeignFunction(function)->parameters;
	} else if (kind_of(function) == NODE_Class) {
		function = lookup_symbol_in_scope(ClassDefinition(function)->scope, tarot_const_string("__init__"));
		if (function != NULL) {
			parameters = FunctionDefinition(function)->parameters;
		} else {
			tarot_error_at(position_of(node), "No constructor found");
			return false;
		}
	} else {
		tarot_error_at(position_of(node), "Identifier is not callable!");
		return false;
	}
	if (Block(parameters)->num_elements != Block(arguments)->num_elements) {
		raise_invalid_argument_count_error(
			position_of(node), name_of(function),
			Block(parameters)->num_elements,
			Block(arguments)->num_elements
		);
		return false;
	}
	for (i = 0; i < Block(parameters)->num_elements; i++) {
		struct tarot_node *parameter = Block(parameters)->elements[i];
		struct tarot_node *argument = Block(arguments)->elements[i];
		if (not compare_types(type_of(parameter), type_of(argument))) {
			raise_type_error(position_of(node), parameter, argument);
			return false;
		}
	}
	return true;
}

/******************************************************************************
 * MARK: Assignment
 *****************************************************************************/

static void mark_as_set(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind_of(node));
			break;
		case NODE_Variable:
			Variable(node)->isset = true;
			break;
	}
}

static bool validate_assign(struct tarot_node *node) {
	struct tarot_node *root = definition_of(Assignment(node)->identifier);
	if (kind_of(root) != NODE_Variable) {
		tarot_error_at(position_of(node), "Left operand of assignment statement cannot be written to!");
		/* TODO: State a reason */
		return false;
	}
	if (not compare_types(
		type_of(Assignment(node)->identifier),
		type_of(Assignment(node)->value)
	)) {
		tarot_begin_error(position_of(node));
		tarot_fputs(tarot_stderr, "Invalid assignment! Cannot assign ");
		print_node(Assignment(node)->value);
		tarot_fputs(tarot_stderr, " to ");
		print_node(Assignment(node)->identifier);
		tarot_fputs(tarot_stderr, "!\n");
		tarot_end_error();
		raise_type_error(position_of(node), Assignment(node)->identifier, Assignment(node)->value);
		return false;
	}
	mark_as_set(link_of(Assignment(node)->identifier));
	return true;
}

/******************************************************************************
 * MARK: Identifier
 *****************************************************************************/

static bool is_instanciable(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			return false;
		case NODE_Variable:
		case NODE_Parameter:
		case NODE_Constant:
		case NODE_Enumerator:
			return true;
	}
}

static bool is_variable(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			return false;
		case NODE_Variable:
		case NODE_Parameter:
			return true;
	}
}

static bool validate_identifier(struct scope_stack *stack, struct tarot_node *node) {
	if (Identifier(node)->link == NULL) {
		tarot_begin_error(position_of(node));
		tarot_fputs(tarot_stderr, "Undefined reference to ");
		tarot_format(tarot_stderr, TAROT_COLOR_PURPLE);
		tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
		tarot_print_string(tarot_stderr, name_of(node));
		tarot_format(tarot_stderr, TAROT_COLOR_RESET);
		tarot_fputs(tarot_stderr, "!");
		tarot_end_error();
		return false;
	}
	if (not stack->type_checking and not tarot_match_string(name_of(node), "self")) {
		if (is_variable(link_of(node)) and position_of(node)->line < position_of(link_of(node))->line) {
			tarot_begin_error(position_of(node));
			print_text("Identifier ");
			print_name(name_of(node));
			print_text(" is used before it is defined!");
			tarot_end_error();
			return false;
		}
		if (kind_of(link_of(node)) == NODE_Variable and not Variable(link_of(node))->isset) {
			tarot_begin_error(position_of(node));
			print_text("Variable ");
			print_name(name_of(node));
			print_text(" is used before initialization!");
			tarot_end_error();
			return false;
		}
		return true; /* FIXME */
		/* Prevent users from assigning Class name to a variable of type Class */
		if (not is_instanciable(Identifier(node)->link)) {
			tarot_error_at(
				position_of(node),
				"Identifier %s is not instanciable because it refers to %s!",
				tarot_string_text(name_of(node)),
				tarot_string_text(name_of(Identifier(node)->link))
			);
			return false;
		}
	}
	return true;
}

/******************************************************************************
 * MARK: Relation
 *****************************************************************************/

static bool validate_relation(struct tarot_node *node) {
	if (definition_of(node) == NULL) {
		tarot_begin_error(position_of(node));
		tarot_fputs(tarot_stderr, "Undefined reference to ");
		tarot_format(tarot_stderr, TAROT_COLOR_PURPLE);
		tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
		tarot_serialize_node(tarot_stderr, node);
		tarot_format(tarot_stderr, TAROT_COLOR_RESET);
		tarot_fputs(tarot_stderr, "!");
		tarot_end_error();
		return false;
	}
	return true;
}

/******************************************************************************
 * MARK: Import
 *****************************************************************************/

static bool validate_import(struct tarot_node *node) {
	if (link_of(node) == NULL) {
		tarot_begin_error(position_of(node));
		tarot_fputs(tarot_stderr, "Failed to import ");
		tarot_format(tarot_stderr, TAROT_COLOR_PURPLE);
		tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
		tarot_print_string(tarot_stderr, name_of(node));
		tarot_format(tarot_stderr, TAROT_COLOR_RESET);
		tarot_fputs(tarot_stderr, " from module ");
		tarot_print_string(tarot_stderr, ImportStatement(node)->path);
		tarot_fputs(tarot_stderr, "!");
		tarot_end_error();
		return false;
	}
	return true;
}

/******************************************************************************
 * MARK: Function
 *****************************************************************************/

static bool validate_function(struct tarot_node *node) {
	if (Block(FunctionDefinition(node)->block)->num_elements == 0) {
		tarot_error_at(position_of(node), "Function %s is empty!", tarot_string_text(name_of(node)));
		return false;
	}
	if (FunctionDefinition(node)->return_value) {
		size_t i;
		struct tarot_node **elements = Block(FunctionDefinition(node)->block)->elements;
		struct tarot_node *it = elements;
		size_t length = Block(FunctionDefinition(node)->block)->num_elements;
		unsigned int num_returns = 0;
		for (i = 0; i < length; i++) {
			if (kind_of(elements[i]) == NODE_Return) {
				num_returns++;
			}
		}
		if (num_returns == 0) {
			tarot_error_at(position_of(node), "Function does not contain a return statement!");
			return false;
		}
	}
	if (Block(FunctionDefinition(node)->parameters)->num_elements >= 16) {
		tarot_error_at(position_of(node), "Too many parameters");
	}
	return true;
}

/******************************************************************************
 * MARK: If
 *****************************************************************************/

static bool validate_if_statement(struct tarot_node *node) {
	if (Type(type_of(IfStatement(node)->condition))->type != TYPE_BOOLEAN) {
		tarot_begin_error(position_of(node));
		tarot_fputs(tarot_stderr, "Condition ");
		tarot_format(tarot_stderr, TAROT_COLOR_PURPLE);
		tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
		tarot_serialize_node(tarot_stderr, IfStatement(node)->condition);
		tarot_format(tarot_stderr, TAROT_COLOR_RESET);
		tarot_fputs(tarot_stderr, " in if-statement evaluates to type ");
		tarot_format(tarot_stderr, TAROT_COLOR_PURPLE);
		tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
		tarot_serialize_node(tarot_stderr, type_of(IfStatement(node)->condition));
		tarot_format(tarot_stderr, TAROT_COLOR_RESET);
		tarot_fputs(tarot_stderr, "! Expected a logical value instead!");
		tarot_end_error();
		return false;
	}
	return true;
}

/******************************************************************************
 * MARK: WhileLoop
 *****************************************************************************/

static bool validate_while_statement(struct tarot_node *node) {
	if (Type(type_of(WhileLoop(node)->condition))->type != TYPE_BOOLEAN) {
		tarot_begin_error(position_of(node));
		tarot_fputs(tarot_stderr, "Condition ");
		tarot_format(tarot_stderr, TAROT_COLOR_PURPLE);
		tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
		tarot_serialize_node(tarot_stderr, WhileLoop(node)->condition);
		tarot_format(tarot_stderr, TAROT_COLOR_RESET);
		tarot_fputs(tarot_stderr, " in while-statement evaluates to type ");
		tarot_format(tarot_stderr, TAROT_COLOR_PURPLE);
		tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
		tarot_serialize_node(tarot_stderr, type_of(WhileLoop(node)->condition));
		tarot_format(tarot_stderr, TAROT_COLOR_RESET);
		tarot_fputs(tarot_stderr, "! Expected a logical value instead!");
		tarot_end_error();
		return false;
	}
	return true;
}

/******************************************************************************
 * MARK: TypeCasts
 *****************************************************************************/

static void raise_invalid_cast_error(
	enum tarot_datatype type,
	struct tarot_node *operand
) {
	tarot_begin_error(position_of(operand));
	tarot_fputs(tarot_stderr, "Invalid cast expression: Cannot convert type ");
	tarot_format(tarot_stderr, TAROT_COLOR_YELLOW);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_serialize_node(tarot_stderr, type_of(operand));
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
	tarot_fputs(tarot_stderr, " to type ");
	tarot_format(tarot_stderr, TAROT_COLOR_YELLOW);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_fputs(tarot_stderr, datatype_string(type));
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
	tarot_end_error();
}

static bool validate_boolean_typecast(struct tarot_node *operand) {
	switch (Type(type_of(operand))->type) {
		case TYPE_BOOLEAN:
		case TYPE_STRING:
			return true;
		default:
			raise_invalid_cast_error(TYPE_BOOLEAN, operand);
			return false;
	}
}

static bool validate_float_typecast(struct tarot_node *operand) {
	switch (Type(type_of(operand))->type) {
		case TYPE_FLOAT:
		case TYPE_INTEGER:
		case TYPE_RATIONAL:
		case TYPE_STRING:
			return true;
		default:
			raise_invalid_cast_error(TYPE_FLOAT, operand);
			return false;
	}
}

static bool validate_integer_typecast(struct tarot_node *operand) {
	switch (Type(type_of(operand))->type) {
		case TYPE_FLOAT:
		case TYPE_INTEGER:
		case TYPE_RATIONAL:
		case TYPE_STRING:
			return true;
		default:
			raise_invalid_cast_error(TYPE_INTEGER, operand);
			return false;
	}
}

static bool validate_rational_typecast(struct tarot_node *operand) {
	switch (Type(type_of(operand))->type) {
		case TYPE_FLOAT:
		case TYPE_INTEGER:
		case TYPE_RATIONAL:
		case TYPE_STRING:
			return true;
		default:
			raise_invalid_cast_error(TYPE_RATIONAL, operand);
			return false;
	}
}

static bool validate_string_typecast(struct tarot_node *operand) {
	switch (Type(type_of(operand))->type) {
		case TYPE_BOOLEAN:
		case TYPE_FLOAT:
		case TYPE_INTEGER:
		case TYPE_RATIONAL:
		case TYPE_STRING:
			return true;
		default:
			raise_invalid_cast_error(TYPE_STRING, operand);
			return false;
	}
}

static bool validate_typecast(struct tarot_node *node) {
	switch (CastExpression(node)->kind) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", CastExpression(node)->kind);
			return false;
		case TYPE_BOOLEAN:
			return validate_boolean_typecast(CastExpression(node)->operand);
		case TYPE_FLOAT:
			return validate_float_typecast(CastExpression(node)->operand);
		case TYPE_INTEGER:
			return validate_integer_typecast(CastExpression(node)->operand);
		case TYPE_RATIONAL:
			return validate_rational_typecast(CastExpression(node)->operand);
		case TYPE_STRING:
			return validate_string_typecast(CastExpression(node)->operand);
	}
}

/******************************************************************************
 * MARK: LogicalExpression
 *****************************************************************************/

static void raise_logical_error(
	struct tarot_node *node,
	struct tarot_node *type
) {
	tarot_begin_error(position_of(node));
	tarot_fputs(tarot_stderr, "Logical operation [x ");
	tarot_format(tarot_stderr, TAROT_COLOR_GREEN);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_fputs(tarot_stderr, LogicalExpressionOperatorString(LogicalExpression(node)->operator));
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
	tarot_fputs(tarot_stderr, " y] is only defined for operands of type ");
	tarot_format(tarot_stderr, TAROT_COLOR_CYAN);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_fputs(tarot_stderr, datatype_string(TYPE_BOOLEAN));
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
	tarot_fputs(tarot_stderr, ", but here operands are of typey ");
	tarot_format(tarot_stderr, TAROT_COLOR_CYAN);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_serialize_node(tarot_stderr, type);
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
	tarot_fputs(tarot_stderr, "!");
	tarot_end_error();
}

static bool validate_logical_expression(struct tarot_node *node) {
	struct tarot_node *a = type_of(LogicalExpression(node)->right_operand);
	struct tarot_node *b = type_of(LogicalExpression(node)->left_operand);
	if (not compare_types(a, b)) {
		raise_type_error(position_of(node), a, b);
		return false;
	}
	switch (Type(b)->type) {
		case TYPE_BOOLEAN:
			return true;
		default:
			raise_logical_error(node, a);
			return false;
	}
}

/******************************************************************************
 * MARK: RelationalExpression
 *****************************************************************************/

/* TODO: More extensive message like 2 funcs above */
static void raise_relational_error(
	struct tarot_node *node,
	struct tarot_node *type
) {
	tarot_begin_error(position_of(node));
	tarot_fputs(tarot_stderr, "Relation operation [x ");
	tarot_format(tarot_stderr, TAROT_COLOR_GREEN);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_fputs(tarot_stderr, RelationalExpressionOperatorString(RelationalExpression(node)->operator));
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
	tarot_fputs(tarot_stderr, " y] is not permitted on instances of type ");
	tarot_format(tarot_stderr, TAROT_COLOR_PURPLE);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_serialize_node(tarot_stderr, type);
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
	tarot_fputs(tarot_stderr, " !");
	tarot_end_error();
}

static bool validate_restricted_relational_expression(
	struct tarot_node *node,
	struct tarot_node *type
) {
	switch (RelationalExpression(node)->operator) {
		case EXPR_EQUAL:
		case EXPR_NOT_EQUAL:
			return true;
		default:
			raise_relational_error(node, type);
			return false;
	}
}

static bool validate_relational_expression(struct tarot_node *node) {
	struct tarot_node *a = type_of(RelationalExpression(node)->right_operand);
	struct tarot_node *b = type_of(RelationalExpression(node)->left_operand);
	if (not compare_types(a, b)) {
		raise_type_error(position_of(node), a, b);
		return false;
	}
	switch (Type(b)->type) {
		case TYPE_BOOLEAN:
		case TYPE_STRING:
			return validate_restricted_relational_expression(node, a);
		default:
			return true;
	}
}

/******************************************************************************
 * MARK: ArithmeticExpression
 *****************************************************************************/

static void raise_arithmetic_error(
	struct tarot_node *node,
	struct tarot_node *type
) {
	tarot_begin_error(position_of(node));
	tarot_fputs(tarot_stderr, "Arithmetic operation [x ");
	tarot_format(tarot_stderr, TAROT_COLOR_GREEN);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_fputs(tarot_stderr, ArithmeticExpressionOperatorString(ArithmeticExpression(node)->operator));
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
	tarot_fputs(tarot_stderr, " y] is not permitted on instances of type ");
	tarot_format(tarot_stderr, TAROT_COLOR_PURPLE);
	tarot_format(tarot_stderr, TAROT_COLOR_BOLD);
	tarot_serialize_node(tarot_stderr, type);
	tarot_format(tarot_stderr, TAROT_COLOR_RESET);
	tarot_fputs(tarot_stderr, "!");
	tarot_end_error();
}

static bool validate_arithmetic_expression(struct tarot_node *node) {
	struct tarot_node *a = type_of(ArithmeticExpression(node)->right_operand);
	struct tarot_node *b = type_of(ArithmeticExpression(node)->left_operand);
	if (not compare_types(a, b)) {
		raise_type_error(position_of(node), a, b);
		return false;
	}
	switch (Type(b)->type) {
		case TYPE_FLOAT:
		case TYPE_INTEGER:
		case TYPE_RATIONAL:
			return true;
		default:
			raise_arithmetic_error(node, a);
			return false;
	}
	return true;
}

/******************************************************************************
 * MARK: Not
 *****************************************************************************/

static bool validate_not_expression(struct tarot_node *node) {
	if (Type(type_of(NotExpression(node)->expression))->type != TYPE_BOOLEAN) {
		tarot_error_at(position_of(node), "Expected Boolean type!");
		return false;
	}
	return true;
}

/******************************************************************************
 * MARK: Neg
 *****************************************************************************/

static bool validate_neg_expression(struct tarot_node *node) {
	switch (Type(type_of(NegExpression(node)->expression))->type) {
		default:
			tarot_error_at(position_of(node), "Expected numeric type!");
			return false;
		case TYPE_FLOAT:
		case TYPE_INTEGER:
		case TYPE_RATIONAL:
			return true;
	}
}

/******************************************************************************
 * MARK: Abs
 *****************************************************************************/

static bool validate_abs_expression(struct tarot_node *node) {
	switch (Type(type_of(AbsExpression(node)->expression))->type) {
		default:
			tarot_error_at(position_of(node), "Expected numeric type!");
			return false;
		case TYPE_FLOAT:
		case TYPE_INTEGER:
		case TYPE_RATIONAL:
			return true;
	}
}

/******************************************************************************
 * MARK: ExprStmt
 *****************************************************************************/

static bool validate_exprstmt(struct tarot_node *node) {
	if (class_of(ExprStatement(node)->expression) != CLASS_EXPRESSION) {
		tarot_error_at(position_of(node), "Expression statement is not an expression!");
		return false;
	}
	switch (kind_of(ExprStatement(node)->expression)) {
		default:
			tarot_error_at(position_of(node), "Invalid expression statement!");
			return false;
		case NODE_FunctionCall:
			break;
	}
	return true;
}

/******************************************************************************
 * MARK: Assert
 *****************************************************************************/

static bool validate_assert(struct tarot_node *node) {
	if (Type(type_of(AssertStatement(node)->condition))->type != TYPE_BOOLEAN) {
		tarot_error_at(position_of(node), "Expected boolean type in assertion!");
		return false;
	}
	return true;
}

/******************************************************************************
 * MARK: Return
 *****************************************************************************/

static bool validate_return(struct tarot_node *node) {
	if (FunctionDefinition(ReturnStatement(node)->function)->return_value == NULL) {
		tarot_error_at(position_of(node),
			"Invalid return statement: "
			"The function does not expect a return value!"
		);
		return false;
	}
	if (not compare_types(
		type_of(ReturnStatement(node)->expression),
		type_of(FunctionDefinition(ReturnStatement(node)->function)->return_value))
	) {
		tarot_error_at(position_of(node), "Type of return value does not match the function signature!");
		return false;
	}
	return true;
}

/******************************************************************************
 * MARK: Print
 *****************************************************************************/

static bool validate_print(struct tarot_node *node) {
	if (Type(type_of(PrintStatement(node)->arguments))->type == TYPE_VOID) {
		tarot_error_at(position_of(node), "Cannot print void type!");
		return false;
	}
	if (Type(type_of(PrintStatement(node)->arguments))->type == TYPE_CUSTOM) {
		if (kind_of(definition_of(PrintStatement(node)->arguments)) != NODE_Enum) {
			tarot_error_at(position_of(node), "Cannot print non-builtin type!");
			return false;
		}
	}
	return true;
}

/******************************************************************************
 * MARK: Variable
 *****************************************************************************/

static bool validate_variable(struct tarot_node *node) {
	if (Variable(node)->value == NULL) {
		return true;
	}
	if (not compare_types(Variable(node)->type, type_of(Variable(node)->value))) {
		raise_type_error(position_of(node), Variable(node)->type, type_of(Variable(node)->value));
		return false;
	}
	mark_as_set(node);
	return true;
}

/******************************************************************************
 * MARK: Constant
 *****************************************************************************/

static bool validate_constant(struct tarot_node *node) {
	if (Constant(node)->value == NULL) {
		tarot_error_at(position_of(node), "Constant is not assigned a value!");
		return false;
	}
	if (not compare_types(Constant(node)->type, type_of(Constant(node)->value))) {
		raise_type_error(position_of(node), Constant(node)->type, type_of(Constant(node)->value));
		return false;
	}
	if (not is_constant(Constant(node)->value)) {
		tarot_error_at(position_of(node), "Value is not constant!");
		return false;
	}
	return true;
}

/******************************************************************************
 * MARK: Parameter
 *****************************************************************************/

static bool validate_parameter(struct tarot_node *node) {
	if (Parameter(node)->value != NULL) {
		if (not compare_types(Parameter(node)->type, type_of(Parameter(node)->value))) {
			raise_type_error(position_of(node), Parameter(node)->type, type_of(Parameter(node)->value));
			return false;
		}
		if (not is_constant(Parameter(node)->value)) {
			tarot_error_at(position_of(node), "Value is not constant!");
			return false;
		}
	}
	return true;
}

/******************************************************************************
 * MARK: Class
 *****************************************************************************/

static bool validate_class(struct tarot_node *node) {
	struct tarot_node *constructor = lookup_symbol_in_scope(
		ClassDefinition(node)->scope,
		tarot_const_string("__init__")
	);
	if (constructor == NULL) {
		tarot_begin_error(position_of(node));
		print_text("Class ");
		print_name(name_of(node));
		print_text(" does not have a constructor!");
		tarot_end_error();
		return false;
	}
	if (FunctionDefinition(constructor)->return_value != NULL) {
		tarot_begin_error(position_of(node));
		print_text("Constructor of class ");
		print_name(name_of(node));
		print_text(" returns a value!");
		tarot_end_error();
		return false;
	}
	if (FunctionDefinition(constructor)->visibility == VISIBILITY_PRIVATE) {
		tarot_begin_error(position_of(node));
		print_text("Constructor of class ");
		print_name(name_of(node));
		print_text(" is private!");
		tarot_end_error();
		return false;
	}
	/* TODO: Compare whether the assignments in constructor block set all class member variables (assert no members are left uninitialized) */
	return true;
}

/******************************************************************************
 * MARK: List
 *****************************************************************************/

static bool validate_list(struct tarot_node *node) {
	/* All elements must have the same type*/
	struct tarot_node *type = type_of(node);
	struct tarot_node *reftype = Type(type)->subtype;
	size_t i;
	for (i = 0; i < List(node)->num_elements; i++) {
		struct tarot_node *element = List(node)->elements[i];
		struct tarot_node *subtype = type_of(element);
		if (not compare_types(reftype, subtype)) {
			tarot_begin_error(position_of(element));
			print_text("List element ");
			print_node(element);
			print_text(" does not fit list type ");
			print_node(type);
			print_text("!");
			tarot_end_error();
			return false;
		}
	}
	return true;
}

/******************************************************************************
 * MARK: Dict
 *****************************************************************************/

static bool validate_dict(struct tarot_node *node) {
	/* All values must have the same type*/
	struct tarot_node *type = type_of(node);
	size_t i;
	for (i = 0; i < Dict(node)->num_elements; i++) {
		struct tarot_node *pair = Dict(node)->elements[i];
		struct tarot_node *subtype = type_of(Pair(pair)->value);
		if (not compare_types(Type(type)->subtype, subtype)) {
			tarot_begin_error(position_of(Pair(pair)->value));
			print_text("Dict value ");
			print_node(Pair(pair)->value);
			print_text(" does not fit dict type ");
			print_node(type);
			print_text("!");
			tarot_end_error();
			return false;
		}
	}
	return true;
}

/******************************************************************************
 * MARK: Subscript
 *****************************************************************************/

static bool validate_subscript(struct tarot_node *node) {
	struct tarot_node *identifier_type = type_of(Subscript(node)->identifier);
	struct tarot_node *index_type = type_of(Subscript(node)->index);
	switch (Type(identifier_type)->type) {
		default:
			tarot_error_at(position_of(node), "Instance is not subscriptable!");
			return false;
		case TYPE_LIST:
			if (Type(index_type)->type != TYPE_INTEGER) {
				tarot_error_at(position_of(node), "List index is not integral");
				return false;
			}
			break;
		case TYPE_DICT:
			if (Type(index_type)->type != TYPE_STRING) {
				tarot_error_at(position_of(node), "Dict index is not a string");
				return false;
			}
			break;
	}
	return true;
}

/******************************************************************************
 * MARK: Type
 *****************************************************************************/

static bool validate_type(struct tarot_node *node) {
	switch (Type(node)->type) {
		default:
			return true;
		case TYPE_LIST:
		case TYPE_DICT:
			if (Type(node)->subtype == NULL) {
				tarot_error_at(position_of(node), "Complex type %s requires subtype!", datatype_string(Type(node)->type));
				return false;
			}
			return true;
	}
}

/******************************************************************************
 * MARK: General
 *****************************************************************************/

bool validate_nodeptr(
	struct tarot_node **nodeptr,
	struct scope_stack *stack
) {
	bool is_valid = true;
	struct tarot_node *node = *nodeptr;
	switch (kind_of(node)) {
		default:
			break;
		case NODE_Import:
			is_valid = validate_import(node);
			if (not is_valid) {
				remove_symbol(stack, node);
			}
			break;
		case NODE_Type:
			is_valid = validate_type(node);
			break;
		case NODE_LogicalExpression:
			is_valid = validate_logical_expression(node);
			break;
		case NODE_RelationalExpression:
			is_valid = validate_relational_expression(node);
			break;
		case NODE_ArithmeticExpression:
			is_valid = validate_arithmetic_expression(node);
			break;
		case NODE_Not:
			is_valid = validate_not_expression(node);
			break;
		case NODE_Neg:
			is_valid = validate_neg_expression(node);
			break;
		case NODE_Abs:
			is_valid = validate_abs_expression(node);
			break;
		case NODE_Identifier:
			is_valid = validate_identifier(stack, node);
			break;
		case NODE_Enumerator:
			is_valid = Enumerator(node)->link != NULL;
			break;
		case NODE_ExpressionStatement:
			is_valid = validate_exprstmt(node);
			break;
		case NODE_Print:
			is_valid = validate_print(node);
			break;
		case NODE_Assert:
			is_valid = validate_assert(node);
			break;
		case NODE_Return:
			is_valid = validate_return(node);
			break;
		case NODE_Relation:
			is_valid = validate_relation(node);
			break;
		case NODE_Assignment:
			is_valid = validate_assign(node);
			break;
		case NODE_FunctionCall:
			is_valid = validate_functioncall(node);
			break;
		case NODE_Function:
			is_valid = validate_function(node);
			if (not is_valid) {
				remove_symbol(stack, node);
			}
			break;
		case NODE_If:
			is_valid = validate_if_statement(node);
			break;
		case NODE_While:
			is_valid = validate_while_statement(node);
			break;
		case NODE_Typecast:
			is_valid = validate_typecast(node);
			break;
		case NODE_Variable:
			is_valid = validate_variable(node);
			break;
		case NODE_Constant:
			is_valid = validate_constant(node);
			break;
		case NODE_Parameter:
			is_valid = validate_parameter(node);
			break;
		case NODE_Class:
			is_valid = validate_class(node);
			break;
		case NODE_List:
			is_valid = validate_list(node);
			break;
		case NODE_Dict:
			is_valid = validate_dict(node);
			break;
		case NODE_Subscript:
			is_valid = validate_subscript(node);
			break;
	}
	if (not is_valid) {
		/*
		* This breaks links. For example if a variable is turned into an error.
		* Then all references (links) to that variable in assignments are
		* pointing to free'd memory and not the error node!
		*nodeptr = tarot_create_node(NODE_ERROR, position_of(node));
		tarot_free_node(node);
		*/
	}
	return is_valid;
}

bool tarot_validate(struct tarot_node *node) {
	bool is_valid = true;
	if (node == NULL) {
		return false;
	}
	while (node != NULL) {
		is_valid = is_valid and Module(node)->num_errors == 0;
		node = Module(node)->next_module;
	}
	return is_valid;
}
