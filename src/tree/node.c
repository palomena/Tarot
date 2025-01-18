#define TAROT_SOURCE
#include "tarot.h"

/******************************************************************************
 * MARK: Node
 *****************************************************************************/

/* Implementation of the general node type */
struct tarot_node {
	union {
		struct Module Module;
		struct Block Block;
		struct LogicalExpression LogicalExpression;
		struct RelationalExpression RelationalExpression;
		struct ArithmeticExpression ArithmeticExpression;
		struct UnaryExpression UnaryExpression;
		struct RangeExpression RangeExpression;
		struct FunctionCall FunctionCall;
		struct Relation Relation;
		struct Subscript Subscript;
		struct Pair Pair;
		struct CastExpression CastExpression;
		struct Literal Literal;
		struct FString FString;
		struct FStringString FStringString;
		struct FStringExpression FStringExpression;
		struct Identifier Identifier;
		struct Enumerator Enumerator;
		struct Type Type;
		struct ImportStatement ImportStatement;
		struct IfStatement IfStatement;
		struct WhileLoop WhileLoop;
		struct ForLoop ForLoop;
		struct MatchStatement MatchStatement;
		struct CaseStatement CaseStatement;
		struct Assignment Assignment;
		struct ExprStatement ExprStatement;
		struct PrintStatement PrintStatement;
		struct InputExpression InputStatement;
		struct TryStatement TryStatement;
		struct CatchStatement CatchStatement;
		struct RaiseStatement RaiseStatement;
		struct ReturnStatement ReturnStatement;
		struct AssertStatement AssertStatement;
		struct ClassDefinition ClassDefinition;
		struct EnumDefinition EnumDefinition;
		struct ClassConstructor ClassConstructor;
		struct FunctionDefinition FunctionDefinition;
		struct ForeignFunction ForeignFunction;
		struct Namespace Namespace;
		struct TypeDefinition TypeDefinition;
		struct UnionDefinition UnionDefinition;
		struct Variable Variable;
		struct Parameter Parameter;
		struct Builtin Builtin;
		struct Break Break;
	} as;
	struct tarot_stream_position position;
	enum tarot_node_kind kind;
};

/******************************************************************************
 * MARK: Visibility
 *****************************************************************************/

const char* visibility_string(enum tarot_visibility kind) {
	switch (kind) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind);
			return NULL;
		case VISIBILITY_NONE:
			return "";
		case VISIBILITY_PRIVATE:
			return tarot_token_string(TAROT_TOK_PRIVATE);
		case VISIBILITY_PUBLIC:
			return tarot_token_string(TAROT_TOK_PUBLIC);
	}
}

/******************************************************************************
 * MARK: Datatype
 *****************************************************************************/

const char* datatype_string(enum tarot_datatype kind) {
	switch (kind) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind);
			return NULL;
		case TYPE_VOID:
			return "Void";
		case TYPE_BOOLEAN:
			return "Boolean";
		case TYPE_FLOAT:
			return "Float";
		case TYPE_INTEGER:
			return "Integer";
		case TYPE_RATIONAL:
			return "Rational";
		case TYPE_STRING:
			return "String";
		case TYPE_LIST:
			return "List";
		case TYPE_DICT:
			return "Dict";
		case TYPE_CUSTOM:
			return "";
	}
}

/******************************************************************************
 * MARK: Node Kinds
 *****************************************************************************/

const char* class_string(enum tarot_node_class kind) {
	switch (kind) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind);
			return NULL;
		case CLASS_ERROR:
			return "Error";
		case CLASS_BLOCK:
			return "Block";
		case CLASS_DEFINITION:
			return "Definition";
		case CLASS_EXPRESSION:
			return "Expression";
		case CLASS_STATEMENT:
			return "Statement";
	}
}

enum tarot_node_class class_of(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			return 128;
		case NODE_NULL:
		case NODE_ERROR:
			return CLASS_ERROR;
		case NODE_Block:
			return CLASS_BLOCK;
		case NODE_LogicalExpression:
		case NODE_RelationalExpression:
		case NODE_ArithmeticExpression:
		case NODE_InfixExpression:
		case NODE_Not:
		case NODE_Neg:
		case NODE_Abs:
		case NODE_FunctionCall:
		case NODE_Relation:
		case NODE_Subscript:
		case NODE_Pair:
		case NODE_Typecast:
		case NODE_Literal:
		case NODE_List:
		case NODE_Dict:
		case NODE_FString:
		case NODE_FStringString:
		case NODE_FStringExpression:
		case NODE_Identifier:
		case NODE_Enumerator:
		case NODE_Type:
			return CLASS_EXPRESSION;
		case NODE_Import:
		case NODE_If:
		case NODE_While:
		case NODE_For:
		case NODE_Match:
		case NODE_Case:
		case NODE_Assignment:
		case NODE_ExpressionStatement:
		case NODE_Print:
		case NODE_Try:
		case NODE_Catch:
		case NODE_Raise:
		case NODE_Assert:
		case NODE_Break:
		case NODE_Breakpoint:
			return CLASS_STATEMENT;
		case NODE_Module:
		case NODE_Class:
		case NODE_Enum:
		case NODE_Constructor:
		case NODE_Function:
		case NODE_Method:
		case NODE_ForeignFunction:
		case NODE_Namespace:
		case NODE_TypeDefinition:
		case NODE_Union:
		case NODE_Variable:
		case NODE_Attribute:
		case NODE_Constant:
		case NODE_Parameter:
		case NODE_Builtin:
			return CLASS_DEFINITION;
	}
}

const char* node_string(enum tarot_node_kind kind) {
	static const char *names[] = {
		"NULL",
		"ERROR",
		"Module",
		"Block",
		"LogicalExpression",
		"RelationalExpression",
		"ArithmeticExpression",
		"InfixExpression",
		"Not",
		"Neg",
		"Abs",
		"Range",
		"FunctionCall",
		"Relation",
		"Subscript",
		"Pair",
		"TypeCast",
		"Literal",
		"List",
		"Dict",
		"FString",
		"FStringString",
		"FStringExpression",
		"Identifier",
		"Enumerator",
		"Type",
		"Import",
		"If",
		"While",
		"For",
		"Match",
		"Case",
		"Assignment",
		"ExpressionStatement",
		"Print",
		"Input",
		"Try",
		"Catch",
		"Raise",
		"Return",
		"Assert",
		"Class",
		"Enum",
		"Constructor",
		"Function",
		"Method",
		"ForeignFunction",
		"Namespace",
		"TypeDefinition",
		"Union",
		"Attribute",
		"Variable",
		"Constant",
		"Parameter",
		"Builtin",
		"Break",
		"Breakpoint"
	};
	if (kind >= 0 and kind < lengthof(names)) {
		return names[kind];
	}
	tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected node kind: %d!", kind);
	return NULL;
}

enum tarot_node_kind kind_of(struct tarot_node *node) {
	return node != NULL ? node->kind : NODE_NULL;
}

/******************************************************************************
 * MARK: Operators
 *****************************************************************************/

const char* LogicalExpressionOperatorString(enum LogicalExpressionOperator operator) {
	switch (operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", operator);
			return NULL;
		case EXPR_AND:
			return tarot_token_string(TAROT_TOK_AND);
		case EXPR_OR:
			return tarot_token_string(TAROT_TOK_OR);
		case EXPR_XOR:
			return tarot_token_string(TAROT_TOK_XOR);
	}
}

const char* RelationalExpressionOperatorString(enum RelationalExpressionOperator operator) {
	switch (operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", operator);
			return NULL;
		case EXPR_GREATER:
			return tarot_token_string(TAROT_TOK_GREATER_THAN);
		case EXPR_GREATER_EQUAL:
			return tarot_token_string(TAROT_TOK_GREATER_EQUAL);
		case EXPR_LESS:
			return tarot_token_string(TAROT_TOK_LESS_THAN);
		case EXPR_LESS_EQUAL:
			return tarot_token_string(TAROT_TOK_LESS_EQUAL);
		case EXPR_EQUAL:
			return tarot_token_string(TAROT_TOK_EQUAL);
		case EXPR_NOT_EQUAL:
			return tarot_token_string(TAROT_TOK_NOT_EQUAL);
		case EXPR_IN:
			return tarot_token_string(TAROT_TOK_IN);
	}
}

const char* ArithmeticExpressionOperatorString(enum ArithmeticExpressionOperator operator) {
	switch (operator) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", operator);
			return NULL;
		case EXPR_ADD:
			return tarot_token_string(TAROT_TOK_PLUS);
		case EXPR_SUBTRACT:
			return tarot_token_string(TAROT_TOK_MINUS);
		case EXPR_MULTIPLY:
			return tarot_token_string(TAROT_TOK_MULTIPLY);
		case EXPR_DIVIDE:
			return tarot_token_string(TAROT_TOK_DIVIDE);
		case EXPR_MODULO:
			return tarot_token_string(TAROT_TOK_MOD);
		case EXPR_POWER:
			return tarot_token_string(TAROT_TOK_POWER);
	}
}

/******************************************************************************
 * MARK: Node Casts
 *****************************************************************************/

struct Module* Module(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Module);
	return &node->as.Module;
}

struct Block* Block(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Block);
	return &node->as.Block;
}

struct LogicalExpression* LogicalExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_LogicalExpression);
	return &node->as.LogicalExpression;
}

struct RelationalExpression* RelationalExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_RelationalExpression);
	return &node->as.RelationalExpression;
}

struct ArithmeticExpression* ArithmeticExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_ArithmeticExpression);
	return &node->as.ArithmeticExpression;
}

struct UnaryExpression* InfixExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_InfixExpression);
	return &node->as.UnaryExpression;
}

struct UnaryExpression* NotExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Not);
	return &node->as.UnaryExpression;
}

struct UnaryExpression* NegExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Neg);
	return &node->as.UnaryExpression;
}

struct UnaryExpression* AbsExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Abs);
	return &node->as.UnaryExpression;
}

struct RangeExpression* RangeExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Range);
	return &node->as.RangeExpression;
}

struct FunctionCall* FunctionCall(struct tarot_node *node) {
	assert(kind_of(node) == NODE_FunctionCall);
	return &node->as.FunctionCall;
}

struct Relation* Relation(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Relation);
	return &node->as.Relation;
}

struct Subscript* Subscript(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Subscript);
	return &node->as.Subscript;
}

struct Pair* Pair(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Pair);
	return &node->as.Pair;
}

struct CastExpression* CastExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Typecast);
	return &node->as.CastExpression;
}

struct Literal* Literal(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Literal);
	return &node->as.Literal;
}

struct Block* List(struct tarot_node *node) {
	assert(kind_of(node) == NODE_List);
	return &node->as.Block;
}

struct Block* Dict(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Dict);
	return &node->as.Block;
}

struct FString* FString(struct tarot_node *node) {
	assert(kind_of(node) == NODE_FString);
	return &node->as.FString;
}

struct FStringString* FStringString(struct tarot_node *node) {
	assert(kind_of(node) == NODE_FStringString);
	return &node->as.FStringString;
}

struct FStringExpression* FStringExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_FStringExpression);
	return &node->as.FStringExpression;
}

struct Identifier* Identifier(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Identifier);
	return &node->as.Identifier;
}

struct Enumerator* Enumerator(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Enumerator);
	return &node->as.Enumerator;
}

struct Type* Type(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Type);
	return &node->as.Type;
}

struct ImportStatement* ImportStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Import);
	return &node->as.ImportStatement;
}

struct IfStatement* IfStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_If);
	return &node->as.IfStatement;
}

struct WhileLoop* WhileLoop(struct tarot_node *node) {
	assert(kind_of(node) == NODE_While);
	return &node->as.WhileLoop;
}

struct ForLoop* ForLoop(struct tarot_node *node) {
	assert(kind_of(node) == NODE_For);
	return &node->as.ForLoop;
}

struct MatchStatement* MatchStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Match);
	return &node->as.MatchStatement;
}

struct CaseStatement* CaseStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Case);
	return &node->as.CaseStatement;
}

struct Assignment* Assignment(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Assignment);
	return &node->as.Assignment;
}

struct ExprStatement* ExprStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_ExpressionStatement);
	return &node->as.ExprStatement;
}

struct PrintStatement* PrintStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Print);
	return &node->as.PrintStatement;
}

struct InputExpression* InputExpression(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Input);
	return &node->as.InputStatement;
}

struct TryStatement* TryStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Try);
	return &node->as.TryStatement;
}

struct CatchStatement* CatchStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Catch);
	return &node->as.CatchStatement;
}

struct RaiseStatement* RaiseStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Raise);
	return &node->as.RaiseStatement;
}

struct ReturnStatement* ReturnStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Return);
	return &node->as.ReturnStatement;
}

struct AssertStatement* AssertStatement(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Assert);
	return &node->as.AssertStatement;
}

struct ClassDefinition* ClassDefinition(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Class);
	return &node->as.ClassDefinition;
}

struct EnumDefinition* EnumDefinition(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Enum);
	return &node->as.EnumDefinition;
}

struct ClassConstructor* ClassConstructor(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Constructor);
	return &node->as.ClassConstructor;
}

struct FunctionDefinition* FunctionDefinition(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Function);
	return &node->as.FunctionDefinition;
}

struct FunctionDefinition* MethodDefinition(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Method);
	return &node->as.FunctionDefinition;
}

struct ForeignFunction* ForeignFunction(struct tarot_node *node) {
	assert(kind_of(node) == NODE_ForeignFunction);
	return &node->as.ForeignFunction;
}

struct Namespace* Namespace(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Namespace);
	return &node->as.Namespace;
}

struct TypeDefinition* TypeDefinition(struct tarot_node *node) {
	assert(kind_of(node) == NODE_TypeDefinition);
	return &node->as.TypeDefinition;
}

struct UnionDefinition* UnionDefinition(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Union);
	return &node->as.UnionDefinition;
}

struct Variable* Variable(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Variable);
	return &node->as.Variable;
}

struct Variable* Attribute(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Attribute);
	return &node->as.Variable;
}

struct Variable* Constant(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Constant);
	return &node->as.Variable;
}

struct Parameter* Parameter(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Parameter);
	return &node->as.Parameter;
}

struct Builtin* Builtin(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Builtin);
	return &node->as.Builtin;
}

struct Break* Break(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Break);
	return &node->as.Break;
}

struct UnaryExpression* Breakpoint(struct tarot_node *node) {
	assert(kind_of(node) == NODE_Breakpoint);
	return &node->as.UnaryExpression;
}

/******************************************************************************
 * MARK: Create
 *****************************************************************************/

struct tarot_node* tarot_temp_node(
	enum tarot_node_kind kind,
	struct tarot_stream_position *position
) {
	static struct tarot_node circular_node_buffer[16]; /* smaller values (8) result in error for List[List[Integer]] already */
	static const uint8_t length = lengthof(circular_node_buffer);
	static uint8_t index = 0;
	struct tarot_node *node = &circular_node_buffer[index++ % length];
	memset(node, 0, sizeof(*node));
	node->kind = kind;
	memcpy(&node->position, position, sizeof(*position));
	return node;
}

static size_t num_nodes = 0;

size_t tarot_num_nodes(void) {
	return num_nodes;
}

struct tarot_node* tarot_create_node(
	enum tarot_node_kind kind,
	struct tarot_stream_position *position
) {
	struct tarot_node *node = tarot_malloc(sizeof(*node));
	node->kind = kind;
	memcpy(&node->position, position, sizeof(*position));
	num_nodes++;
	return node;
}

/******************************************************************************
 * MARK: Utils
 *****************************************************************************/

uint16_t index_of(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind_of(node));
			return 0;
		case NODE_Identifier:
		case NODE_Subscript:
			return index_of(link_of(node));
		case NODE_Relation:
			return index_of(Relation(node)->link);
		case NODE_Enumerator:
			return Enumerator(node)->index;
		case NODE_Variable:
			return Variable(node)->index;
		case NODE_Attribute:
			return Attribute(node)->index;
		case NODE_Constant:
			return Constant(node)->index;
		case NODE_Parameter:
			return Parameter(node)->index;
		case NODE_Function:
			return FunctionDefinition(node)->index;
		case NODE_Method:
			return MethodDefinition(node)->index;
		case NODE_Constructor:
			return ClassConstructor(node)->index;
		case NODE_ForeignFunction:
			return ForeignFunction(node)->index;
	}
}

bool is_constant(struct tarot_node *node) {
	switch (kind_of(node)) {
		size_t i;
		default:
			return false;
		case NODE_Literal:
			return true;
		case NODE_List:
			for (i = 0; i < List(node)->num_elements; i++) {
				if (not is_constant(List(node)->elements[i])) {
					return false;
				}
			}
			return true;
		case NODE_ArithmeticExpression:
			return (
				is_constant(ArithmeticExpression(node)->left_operand)
				and is_constant(ArithmeticExpression(node)->right_operand)
			);
		case NODE_LogicalExpression:
			return (
				is_constant(LogicalExpression(node)->left_operand)
				and is_constant(LogicalExpression(node)->right_operand)
			);
		case NODE_RelationalExpression:
			return (
				is_constant(RelationalExpression(node)->left_operand)
				and is_constant(RelationalExpression(node)->right_operand)
			);
		case NODE_InfixExpression:
			return is_constant(InfixExpression(node)->expression);
		case NODE_Typecast:
			return is_constant(CastExpression(node)->operand);
		case NODE_Abs:
			return is_constant(AbsExpression(node)->expression);
		case NODE_Neg:
			return is_constant(NegExpression(node)->expression);
	}
}

struct tarot_stream_position* position_of(struct tarot_node *node) {
	assert(node != NULL);
	return &node->position;
}

struct tarot_string* name_of(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind_of(node));
			return NULL;
		case NODE_Identifier:
			return Identifier(node)->name;
		case NODE_Enumerator:
			return Enumerator(node)->name;
		case NODE_Relation:
			return name_of(Relation(node)->parent);
		case NODE_Import:
			return ImportStatement(node)->alias;
		case NODE_Class:
			return ClassDefinition(node)->name;
		case NODE_Enum:
			return EnumDefinition(node)->name;
		case NODE_Constructor:
			return name_of(link_of(node));
		case NODE_Function:
			return FunctionDefinition(node)->name;
		case NODE_Method:
			return MethodDefinition(node)->name;
		case NODE_ForeignFunction:
			return ForeignFunction(node)->name;
		case NODE_Namespace:
			return Namespace(node)->name;
		case NODE_Type:
			return TypeDefinition(node)->name;
		case NODE_Union:
			return UnionDefinition(node)->name;
		case NODE_Variable:
			return Variable(node)->name;
		case NODE_Attribute:
			return Attribute(node)->name;
		case NODE_Constant:
			return Constant(node)->name;
		case NODE_Parameter:
			return Parameter(node)->name;
	}
}

enum tarot_visibility visibility_of(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind_of(node));
			return VISIBILITY_NONE;
		case NODE_Class:
			return ClassDefinition(node)->visibility;
		case NODE_Enum:
			return EnumDefinition(node)->visibility;
		case NODE_Function:
			return FunctionDefinition(node)->visibility;
		case NODE_Method:
			return MethodDefinition(node)->visibility;
		case NODE_ForeignFunction:
			return ForeignFunction(node)->visibility;
		case NODE_Namespace:
			return Namespace(node)->visibility;
		case NODE_TypeDefinition:
			return TypeDefinition(node)->visibility;
		case NODE_Union:
			return UnionDefinition(node)->visibility;
	}
}

struct tarot_node* link_of(struct tarot_node *node) {
	if (node == NULL) {
		return node;
	}
	switch (kind_of(node)) {
		default:
			return NULL;
		case NODE_Identifier:
			return Identifier(node)->link;
		case NODE_Enumerator:
			return Enumerator(node)->link;
		case NODE_Relation:
			return Relation(node)->link;
		case NODE_Subscript:
			return link_of(Subscript(node)->identifier);
		case NODE_FunctionCall:
			return link_of(FunctionCall(node)->identifier);
		case NODE_Type:
			return link_of(Type(node)->identifier);
		case NODE_Import:
			return link_of(ImportStatement(node)->identifier);
		case NODE_Constructor:
			return ClassConstructor(node)->link;
	}
}

struct tarot_node* definition_of(struct tarot_node *node) {
	if (node == NULL) {
		return node;
	}
	switch (kind_of(node)) {
		default:
			if (class_of(node) != CLASS_DEFINITION) {
				tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind_of(node));
				return NULL;
			}
			return node;
		case NODE_Identifier:
			return definition_of(Identifier(node)->link);
		case NODE_Enumerator:
			return definition_of(Enumerator(node)->link);
		case NODE_Relation:
			return definition_of(Relation(node)->link);
		case NODE_Subscript:
			return definition_of(Subscript(node)->identifier);
		case NODE_FunctionCall:
			return definition_of(FunctionCall(node)->identifier);
		case NODE_Type:
			return definition_of(Type(node)->identifier);
		case NODE_Import:
			return definition_of(ImportStatement(node)->identifier);
	}
}

struct tarot_list* scope_of(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind_of(node));
			return NULL;
		case NODE_Function:
			return FunctionDefinition(node)->scope;
		case NODE_Method:
			return MethodDefinition(node)->scope;
		case NODE_Constructor:
			return ClassConstructor(node)->scope;
	}
}

static struct tarot_node* volatile_type(struct tarot_node *link) {
	struct tarot_node *node = tarot_temp_node(NODE_Type, position_of(link));
	Type(node)->type = TYPE_CUSTOM;
	Type(node)->identifier = tarot_temp_node(NODE_Identifier, position_of(link));
	Identifier(Type(node)->identifier)->name = name_of(link);
	Identifier(Type(node)->identifier)->link = link;
	return node;
}

static struct tarot_node* builtin_type(
	enum tarot_datatype type,
	struct tarot_stream_position *position
) {
	struct tarot_node *node = tarot_temp_node(NODE_Type, position);
	Type(node)->type = type;
	/*Type(node)->identifier = tarot_temp_node(NODE_Identifier, position);
	Identifier(Type(node)->identifier)->name = tarot_const_string(datatype_string(type));*/
	return node;
}

static struct tarot_node* nested_type(
	enum tarot_datatype type,
	struct tarot_node *child,
	struct tarot_stream_position *position
) {
	struct tarot_node *node = tarot_temp_node(NODE_Type, position);
	Type(node)->type = type;
	Type(node)->subtype = child;
	return node;
}

static struct tarot_stream_position* invalid_position(void) {
	static struct tarot_stream_position position = {"invalid", 0, 0};
	return &position;
}

struct tarot_node* type_of(struct tarot_node *node) {
	switch (kind_of(node)) {
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind_of(node));
			return NULL;
		case NODE_NULL:
			return builtin_type(TYPE_VOID, invalid_position());
		case NODE_Type:
			return node;
		case NODE_Identifier:
			return type_of(definition_of(node));
		case NODE_Enumerator:
			return type_of(definition_of(node));
		case NODE_Relation:
			return type_of(definition_of(node));
		case NODE_FunctionCall:
			return type_of(definition_of(node));
		case NODE_Function:
			return type_of(FunctionDefinition(node)->return_value);
		case NODE_Method:
			return type_of(MethodDefinition(node)->return_value);
		case NODE_Constructor:
			return type_of(ClassConstructor(node)->link);
		case NODE_ForeignFunction:
			return type_of(ForeignFunction(node)->return_value);
		case NODE_Class:
		case NODE_Enum:
		case NODE_Namespace:
		case NODE_Union:
			return volatile_type(node);
		case NODE_Variable:
			return type_of(Variable(node)->type);
		case NODE_Attribute:
			return type_of(Attribute(node)->type);
		case NODE_Constant:
			return type_of(Constant(node)->type);
		case NODE_Parameter:
			return type_of(Parameter(node)->type);
		case NODE_Literal:
			return builtin_type(Literal(node)->type, position_of(node));
		case NODE_List:
			if (List(node)->num_elements == 0) {
				return nested_type(TYPE_LIST, NULL, position_of(node));
			}
			return nested_type(TYPE_LIST, type_of(List(node)->elements[0]), position_of(node));
		case NODE_Dict:
			if (Dict(node)->num_elements == 0) {
				return nested_type(TYPE_DICT, NULL, position_of(node));
			}
			return nested_type(TYPE_DICT, type_of(Dict(node)->elements[0]), position_of(node));
		case NODE_Pair:
			return type_of(Pair(node)->value);
		case NODE_FString:
		case NODE_FStringString:
		case NODE_Input:
			return builtin_type(TYPE_STRING, position_of(node));
		case NODE_FStringExpression:
			return type_of(FStringExpression(node)->expression);
		case NODE_LogicalExpression:
		case NODE_RelationalExpression:
		case NODE_Not:
			return builtin_type(TYPE_BOOLEAN, position_of(node));
		case NODE_ArithmeticExpression:
			return type_of(ArithmeticExpression(node)->right_operand);
		case NODE_InfixExpression:
			return type_of(InfixExpression(node)->expression);
		case NODE_Neg:
			return type_of(NegExpression(node)->expression);
		case NODE_Abs:
			return type_of(AbsExpression(node)->expression);
		case NODE_Typecast:
			return builtin_type(CastExpression(node)->kind, position_of(node));
		case NODE_Subscript:
			return Type(type_of(Subscript(node)->identifier))->subtype;
		case NODE_Builtin:
			return Builtin(node)->return_type;
	}
}

/******************************************************************************
 * MARK: Copy
 *****************************************************************************/

static union tarot_value copy_literal_value(
	enum tarot_literal_kind kind,
	union tarot_value value
) {
	union tarot_value result;
	switch (kind) {
		default:
			result = value;
			break;
		case VALUE_INTEGER:
			result.Integer = tarot_copy_integer(value.Integer);
			break;
		case VALUE_RATIONAL:
			result.Rational = tarot_copy_rational(value.Rational);
			break;
		case VALUE_RAW_STRING:
		case VALUE_STRING:
			result.String = tarot_copy_string(value.String);
			break;
	}
	return result;
}

struct tarot_node* tarot_copy_node(struct tarot_node *original) {
	struct tarot_node *node = NULL;
	if (original == NULL) {
		return node;
	}
	node = tarot_create_node(kind_of(original), position_of(original));
	memcpy(node, original, sizeof(*node));
	switch (kind_of(original)) {
		size_t i;
		default:
			tarot_sourcecode_error(__FILE__, __LINE__, "Unexpected switchcase: %d", kind_of(node));
			break;
		case NODE_ERROR:
			break;
		case NODE_Module:
			Module(node)->block = tarot_copy_node(Module(original)->block);
			Module(node)->scope = tarot_copy_list(Module(original)->scope);
			break;
		case NODE_Block:
			Block(node)->elements = tarot_malloc(sizeof(node) * Block(node)->num_elements);
			for (i = 0; i < Block(node)->num_elements; i++) {
				Block(node)->elements[i] = tarot_copy_node(Block(original)->elements[i]);
			}
			break;
		case NODE_LogicalExpression:
			LogicalExpression(node)->left_operand = tarot_copy_node(LogicalExpression(original)->left_operand);
			LogicalExpression(node)->right_operand = tarot_copy_node(LogicalExpression(original)->right_operand);
			break;
		case NODE_RelationalExpression:
			RelationalExpression(node)->left_operand = tarot_copy_node(RelationalExpression(original)->left_operand);
			RelationalExpression(node)->right_operand = tarot_copy_node(RelationalExpression(original)->right_operand);
			break;
		case NODE_ArithmeticExpression:
			ArithmeticExpression(node)->left_operand = tarot_copy_node(ArithmeticExpression(original)->left_operand);
			ArithmeticExpression(node)->right_operand = tarot_copy_node(ArithmeticExpression(original)->right_operand);
			break;
		case NODE_InfixExpression:
			InfixExpression(node)->expression = tarot_copy_node(InfixExpression(original)->expression);
			break;
		case NODE_Not:
			NotExpression(node)->expression = tarot_copy_node(NotExpression(original)->expression);
			break;
		case NODE_Neg:
			NegExpression(node)->expression = tarot_copy_node(NegExpression(original)->expression);
			break;
		case NODE_Abs:
			AbsExpression(node)->expression = tarot_copy_node(AbsExpression(original)->expression);
			break;
		case NODE_FunctionCall:
			FunctionCall(node)->identifier = tarot_copy_node(FunctionCall(original)->identifier);
			FunctionCall(node)->arguments = tarot_copy_node(FunctionCall(original)->arguments);
			break;
		case NODE_Relation:
			Relation(node)->parent = tarot_copy_node(Relation(original)->parent);
			Relation(node)->child = tarot_copy_string(Relation(original)->child);
			break;
		case NODE_Subscript:
			Subscript(node)->identifier = tarot_copy_node(Subscript(original)->identifier);
			Subscript(node)->index = tarot_copy_node(Subscript(original)->index);
			break;
		case NODE_Pair:
			Pair(node)->key = tarot_copy_node(Pair(original)->key);
			Pair(node)->value = tarot_copy_node(Pair(original)->value);
			break;
		case NODE_Typecast:
			CastExpression(node)->operand = tarot_copy_node(CastExpression(original)->operand);
			break;
		case NODE_Literal:
			Literal(node)->value = copy_literal_value(Literal(original)->kind, Literal(original)->value);
			break;
		case NODE_List:
			List(node)->elements = tarot_malloc(sizeof(node) * List(node)->num_elements);
			for (i = 0; i < List(node)->num_elements; i++) {
				List(node)->elements[i] = tarot_copy_node(List(original)->elements[i]);
			}
			break;
		case NODE_Dict:
			Dict(node)->elements = tarot_malloc(sizeof(node) * Dict(node)->num_elements);
			for (i = 0; i < List(node)->num_elements; i++) {
				Dict(node)->elements[i] = tarot_copy_node(Dict(original)->elements[i]);
			}
			break;
		case NODE_FString:
			FString(node)->elements = tarot_malloc(sizeof(node) * FString(node)->num_elements);
			for (i = 0; i < FString(node)->num_elements; i++) {
				FString(node)->elements[i] = tarot_copy_node(FString(original)->elements[i]);
			}
			break;
		case NODE_FStringString:
			FStringString(node)->value = tarot_copy_string(FStringString(original)->value);
			break;
		case NODE_FStringExpression:
			FStringExpression(node)->expression = tarot_copy_node(FStringExpression(original)->expression);
			break;
		case NODE_Identifier:
			Identifier(node)->name = tarot_copy_string(Identifier(original)->name);
			break;
		case NODE_Enumerator:
			Enumerator(node)->name = tarot_copy_string(Identifier(original)->name);
			break;
		case NODE_Type:
			Type(node)->identifier = tarot_copy_node(Type(original)->identifier);
			Type(node)->subtype = tarot_copy_node(Type(original)->subtype);
			break;
		case NODE_Import:
			ImportStatement(node)->path = tarot_copy_string(ImportStatement(original)->path);
			ImportStatement(node)->identifier = tarot_copy_node(ImportStatement(original)->identifier);
			ImportStatement(node)->alias = tarot_copy_string(ImportStatement(original)->alias);
			break;
		case NODE_If:
			IfStatement(node)->condition = tarot_copy_node(IfStatement(original)->condition);
			IfStatement(node)->block = tarot_copy_node(IfStatement(original)->block);
			IfStatement(node)->elseif = tarot_copy_node(IfStatement(original)->elseif);
			break;
		case NODE_While:
			WhileLoop(node)->condition = tarot_copy_node(WhileLoop(original)->condition);
			WhileLoop(node)->block = tarot_copy_node(WhileLoop(original)->block);
			break;
		case NODE_For:
			ForLoop(node)->identifier = tarot_copy_node(ForLoop(original)->identifier);
			ForLoop(node)->expression = tarot_copy_node(ForLoop(original)->expression);
			ForLoop(node)->block = tarot_copy_node(ForLoop(original)->block);
			break;
		case NODE_Match:
			MatchStatement(node)->pattern = tarot_copy_node(MatchStatement(original)->pattern);
			MatchStatement(node)->block = tarot_copy_node(MatchStatement(original)->block);
			break;
		case NODE_Case:
			CaseStatement(node)->condition = tarot_copy_node(CaseStatement(original)->condition);
			CaseStatement(node)->block = tarot_copy_node(CaseStatement(original)->block);
			break;
		case NODE_Assignment:
			Assignment(node)->identifier = tarot_copy_node(Assignment(original)->identifier);
			Assignment(node)->value = tarot_copy_node(Assignment(original)->value);
			break;
		case NODE_ExpressionStatement:
			ExprStatement(node)->expression = tarot_copy_node(ExprStatement(original)->expression);
			break;
		case NODE_Print:
			PrintStatement(node)->arguments = tarot_copy_node(PrintStatement(original)->arguments);
			break;
		case NODE_Input:
			InputExpression(node)->prompt = tarot_copy_node(InputExpression(original)->prompt);
			break;
		case NODE_Try:
			TryStatement(node)->block = tarot_copy_node(TryStatement(original)->block);
			TryStatement(node)->handlers = tarot_copy_node(TryStatement(original)->handlers);
			break;
		case NODE_Catch:
			CatchStatement(node)->identifiers = tarot_copy_node(CatchStatement(original)->identifiers);
			CatchStatement(node)->block = tarot_copy_node(CatchStatement(original)->block);
			break;
		case NODE_Raise:
			RaiseStatement(node)->identififer = tarot_copy_node(RaiseStatement(original)->identififer);
			break;
		case NODE_Return:
			ReturnStatement(node)->expression = tarot_copy_node(ReturnStatement(original)->expression);
			break;
		case NODE_Assert:
			AssertStatement(node)->condition = tarot_copy_node(AssertStatement(original)->condition);
			break;
		case NODE_Class:
			ClassDefinition(node)->name = tarot_copy_string(ClassDefinition(original)->name);
			ClassDefinition(node)->extends = tarot_copy_node(ClassDefinition(original)->extends);
			ClassDefinition(node)->block = tarot_copy_node(ClassDefinition(original)->block);
			break;
		case NODE_Enum:
			EnumDefinition(node)->name = tarot_copy_string(EnumDefinition(original)->name);
			EnumDefinition(node)->block = tarot_copy_node(EnumDefinition(original)->block);
			break;
		case NODE_Constructor:
			ClassConstructor(node)->parameters = tarot_copy_node(ClassConstructor(original)->parameters);
			ClassConstructor(node)->block = tarot_copy_node(ClassConstructor(original)->block);
			ClassConstructor(node)->scope = tarot_copy_list(ClassConstructor(original)->scope);
			break;
		case NODE_Function:
			FunctionDefinition(node)->name = tarot_copy_string(FunctionDefinition(original)->name);
			FunctionDefinition(node)->parameters = tarot_copy_node(FunctionDefinition(original)->parameters);
			FunctionDefinition(node)->return_value = tarot_copy_node(FunctionDefinition(original)->return_value);
			FunctionDefinition(node)->block = tarot_copy_node(FunctionDefinition(original)->block);
			FunctionDefinition(node)->scope = tarot_copy_list(FunctionDefinition(original)->scope);
			break;
		case NODE_Method:
			MethodDefinition(node)->name = tarot_copy_string(MethodDefinition(original)->name);
			MethodDefinition(node)->parameters = tarot_copy_node(MethodDefinition(original)->parameters);
			MethodDefinition(node)->return_value = tarot_copy_node(MethodDefinition(original)->return_value);
			MethodDefinition(node)->block = tarot_copy_node(MethodDefinition(original)->block);
			MethodDefinition(node)->scope = tarot_copy_list(MethodDefinition(original)->scope);
			break;
		case NODE_ForeignFunction:
			ForeignFunction(node)->name = tarot_copy_string(ForeignFunction(original)->name);
			ForeignFunction(node)->parameters = tarot_copy_node(ForeignFunction(original)->parameters);
			ForeignFunction(node)->return_value = tarot_copy_node(ForeignFunction(original)->return_value);
			break;
		case NODE_Namespace:
			Namespace(node)->name = tarot_copy_string(Namespace(original)->name);
			Namespace(node)->block = tarot_copy_node(Namespace(original)->block);
			Namespace(node)->scope = tarot_copy_list(Namespace(original)->scope);
			break;
		case NODE_TypeDefinition:
			TypeDefinition(node)->name = tarot_copy_string(TypeDefinition(original)->name);
			TypeDefinition(node)->original = tarot_copy_node(TypeDefinition(original)->original);
			break;
		case NODE_Union:
			UnionDefinition(node)->name = tarot_copy_string(UnionDefinition(original)->name);
			UnionDefinition(node)->block = tarot_copy_node(UnionDefinition(original)->block);
			break;
		case NODE_Variable:
			Variable(node)->name = tarot_copy_string(Variable(original)->name);
			Variable(node)->type = tarot_copy_node(Variable(original)->type);
			Variable(node)->value = tarot_copy_node(Variable(original)->value);
			break;
		case NODE_Attribute:
			Attribute(node)->name = tarot_copy_string(Attribute(original)->name);
			Attribute(node)->type = tarot_copy_node(Attribute(original)->type);
			Attribute(node)->value = tarot_copy_node(Attribute(original)->value);
			break;
		case NODE_Constant:
			Constant(node)->name = tarot_copy_string(Constant(original)->name);
			Constant(node)->type = tarot_copy_node(Constant(original)->type);
			Constant(node)->value = tarot_copy_node(Constant(original)->value);
			break;
		case NODE_Parameter:
			Parameter(node)->name = tarot_copy_string(Parameter(original)->name);
			Parameter(node)->type = tarot_copy_node(Parameter(original)->type);
			Parameter(node)->value = tarot_copy_node(Parameter(original)->value);
			break;
	}
	return node;
}

/******************************************************************************
 * MARK: Free
 *****************************************************************************/

static void free_literal_value(struct Literal *literal) {
	switch (literal->kind) {
		default:
			break;
		case VALUE_INTEGER:
			tarot_free_integer(literal->value.Integer);
			break;
		case VALUE_RATIONAL:
			tarot_free_rational(literal->value.Rational);
			break;
		case VALUE_STRING:
			tarot_free_string(literal->value.String);
			break;
	}
}

static void free_node(
	struct tarot_node **nodeptr,
	struct scope_stack *stack,
	void *data
) {
	struct tarot_node *node = *nodeptr;
	unused(stack);
	unused(data);
	switch (kind_of(node)) {
		default:
			break;
		case NODE_Module:
			tarot_free(Module(node)->path);
			destroy_scope(Module(node)->scope);
			break;
		case NODE_Block:
			tarot_free(Block(node)->elements);
			break;
		case NODE_Literal:
			free_literal_value(Literal(node));
			break;
		case NODE_List:
			tarot_free(List(node)->elements);
			break;
		case NODE_Dict:
			tarot_free(Dict(node)->elements);
			break;
		case NODE_FString:
			tarot_free(FString(node)->elements);
			break;
		case NODE_FStringString:
			tarot_free_string(FStringString(node)->value);
			break;
		case NODE_Relation:
			tarot_free_string(Relation(node)->child);
			break;
		case NODE_Identifier:
			tarot_free_string(Identifier(node)->name);
			break;
		case NODE_Enumerator:
			tarot_free_string(Enumerator(node)->name);
			break;
		case NODE_Import:
			tarot_free_string(ImportStatement(node)->path);
			tarot_free_string(ImportStatement(node)->alias);
			break;
		case NODE_Class:
			tarot_free_string(ClassDefinition(node)->name);
			destroy_scope(ClassDefinition(node)->scope);
			break;
		case NODE_Enum:
			tarot_free_string(EnumDefinition(node)->name);
			break;
		case NODE_Constructor:
			destroy_scope(ClassConstructor(node)->scope);
			break;
		case NODE_Function:
			tarot_free_string(FunctionDefinition(node)->name);
			destroy_scope(FunctionDefinition(node)->scope);
			break;
		case NODE_Method:
			tarot_free_string(MethodDefinition(node)->name);
			destroy_scope(MethodDefinition(node)->scope);
			break;
		case NODE_ForeignFunction:
			tarot_free_string(ForeignFunction(node)->name);
			break;
		case NODE_Namespace:
			tarot_free_string(Namespace(node)->name);
			destroy_scope(Namespace(node)->scope);
			break;
		case NODE_TypeDefinition:
			tarot_free_string(TypeDefinition(node)->name);
			break;
		case NODE_Union:
			tarot_free_string(UnionDefinition(node)->name);
			break;
		case NODE_Variable:
			tarot_free_string(Variable(node)->name);
			break;
		case NODE_Attribute:
			tarot_free_string(Attribute(node)->name);
			break;
		case NODE_Constant:
			tarot_free_string(Constant(node)->name);
			break;
		case NODE_Parameter:
			tarot_free_string(Parameter(node)->name);
			break;
	}
	tarot_free(node);
	*nodeptr = NULL;
}

void tarot_free_node(struct tarot_node *node) {
	tarot_traverse_postorder(node, free_node, NULL);
}

/******************************************************************************
 * MARK: Traverse
 *****************************************************************************/

struct traverse_state {
	tarot_visitor_function *enter;
	tarot_visitor_function *leave;
	void *data;
	struct scope_stack scopes;
};

static void traverse_node(
	struct tarot_node **nodeptr,
	struct traverse_state *state
) {
	struct tarot_node *node = *nodeptr;
	if (node == NULL) {
		return;
	}

	if (state->enter) {
		state->enter(nodeptr, &state->scopes, state->data);
	}

	enter_node(&state->scopes, node);

	switch (kind_of(node)) {
		size_t i;
		default:
			break;
		case NODE_Module:
			traverse_node(&Module(node)->block, state);
			break;
		case NODE_Block:
			for (i = 0; i < Block(node)->num_elements; i++) {
				traverse_node(&Block(node)->elements[i], state);
			}
			break;
		case NODE_LogicalExpression:
			traverse_node(&LogicalExpression(node)->left_operand, state);
			traverse_node(&LogicalExpression(node)->right_operand, state);
			break;
		case NODE_RelationalExpression:
			traverse_node(&RelationalExpression(node)->left_operand, state);
			traverse_node(&RelationalExpression(node)->right_operand, state);
			break;
		case NODE_ArithmeticExpression:
			traverse_node(&ArithmeticExpression(node)->left_operand, state);
			traverse_node(&ArithmeticExpression(node)->right_operand, state);
			break;
		case NODE_InfixExpression:
			traverse_node(&InfixExpression(node)->expression, state);
			break;
		case NODE_Not:
			traverse_node(&NotExpression(node)->expression, state);
			break;
		case NODE_Neg:
			traverse_node(&NegExpression(node)->expression, state);
			break;
		case NODE_Abs:
			traverse_node(&AbsExpression(node)->expression, state);
			break;
		case NODE_Range:
			traverse_node(&RangeExpression(node)->start, state);
			traverse_node(&RangeExpression(node)->end, state);
			traverse_node(&RangeExpression(node)->stepsize, state);
			break;
		case NODE_FunctionCall:
			traverse_node(&FunctionCall(node)->identifier, state);
			traverse_node(&FunctionCall(node)->arguments, state);
			break;
		case NODE_Relation:
			traverse_node(&Relation(node)->parent, state);
			/* error what if class is freed *
			if (kind_of(Relation(node)->link) == NODE_Builtin) {
				traverse_node(&Relation(node)->link, state);
			}*/
			break;
		case NODE_Subscript:
			traverse_node(&Subscript(node)->identifier, state);
			traverse_node(&Subscript(node)->index, state);
			break;
		case NODE_Pair:
			traverse_node(&Pair(node)->key, state);
			traverse_node(&Pair(node)->value, state);
			break;
		case NODE_Typecast:
			traverse_node(&CastExpression(node)->operand, state);
			break;
		case NODE_List:
			for (i = 0; i < List(node)->num_elements; i++) {
				traverse_node(&List(node)->elements[i], state);
			}
			break;
		case NODE_Dict:
			for (i = 0; i < Dict(node)->num_elements; i++) {
				traverse_node(&Dict(node)->elements[i], state);
			}
			break;
		case NODE_FString:
			for (i = 0; i < FString(node)->num_elements; i++) {
				traverse_node(&FString(node)->elements[i], state);
			}
			break;
		case NODE_FStringExpression:
			traverse_node(&FStringExpression(node)->expression, state);
			break;
		case NODE_Type:
			traverse_node(&Type(node)->identifier, state);
			traverse_node(&Type(node)->subtype, state);
			break;
		case NODE_If:
			traverse_node(&IfStatement(node)->condition, state);
			traverse_node(&IfStatement(node)->block, state);
			traverse_node(&IfStatement(node)->elseif, state);
			break;
		case NODE_While:
			traverse_node(&WhileLoop(node)->condition, state);
			traverse_node(&WhileLoop(node)->block, state);
			break;
		case NODE_For:
			traverse_node(&ForLoop(node)->identifier, state);
			traverse_node(&ForLoop(node)->expression, state);
			traverse_node(&ForLoop(node)->block, state);
			break;
		case NODE_Match:
			traverse_node(&MatchStatement(node)->pattern, state);
			traverse_node(&MatchStatement(node)->block, state);
			break;
		case NODE_Case:
			traverse_node(&CaseStatement(node)->condition, state);
			traverse_node(&CaseStatement(node)->block, state);
			break;
		case NODE_Assignment:
			traverse_node(&Assignment(node)->identifier, state);
			traverse_node(&Assignment(node)->value, state);
			break;
		case NODE_ExpressionStatement:
			traverse_node(&ExprStatement(node)->expression, state);
			break;
		case NODE_Import:
			traverse_node(&ImportStatement(node)->identifier, state);
			break;
		case NODE_Print:
			traverse_node(&PrintStatement(node)->arguments, state);
			break;
		case NODE_Input:
			traverse_node(&InputExpression(node)->prompt, state);
			break;
		case NODE_Try:
			traverse_node(&TryStatement(node)->block, state);
			traverse_node(&TryStatement(node)->handlers, state);
			break;
		case NODE_Catch:
			traverse_node(&CatchStatement(node)->identifiers, state);
			traverse_node(&CatchStatement(node)->block, state);
			break;
		case NODE_Raise:
			traverse_node(&RaiseStatement(node)->identififer, state);
			break;
		case NODE_Return:
			traverse_node(&ReturnStatement(node)->expression, state);
			break;
		case NODE_Assert:
			traverse_node(&AssertStatement(node)->condition, state);
			break;
		case NODE_Class:
			traverse_node(&ClassDefinition(node)->block, state);
			traverse_node(&ClassDefinition(node)->extends, state);
			break;
		case NODE_Enum:
			traverse_node(&EnumDefinition(node)->block, state);
			break;
		case NODE_Constructor:
			traverse_node(&ClassConstructor(node)->parameters, state);
			traverse_node(&ClassConstructor(node)->block, state);
			break;
		case NODE_Function:
			traverse_node(&FunctionDefinition(node)->parameters, state);
			traverse_node(&FunctionDefinition(node)->return_value, state);
			traverse_node(&FunctionDefinition(node)->block, state);
			break;
		case NODE_Method:
			traverse_node(&MethodDefinition(node)->parameters, state);
			traverse_node(&MethodDefinition(node)->return_value, state);
			traverse_node(&MethodDefinition(node)->block, state);
			break;
		case NODE_ForeignFunction:
			traverse_node(&ForeignFunction(node)->parameters, state);
			traverse_node(&ForeignFunction(node)->return_value, state);
			break;
		case NODE_Namespace:
			traverse_node(&Namespace(node)->block, state);
			break;
		case NODE_Union:
			traverse_node(&UnionDefinition(node)->block, state);
			break;
		case NODE_Variable:
			traverse_node(&Variable(node)->type, state);
			traverse_node(&Variable(node)->value, state);
			break;
		case NODE_Attribute:
			traverse_node(&Attribute(node)->type, state);
			traverse_node(&Attribute(node)->value, state);
			break;
		case NODE_Constant:
			traverse_node(&Constant(node)->type, state);
			traverse_node(&Constant(node)->value, state);
			break;
		case NODE_Parameter:
			traverse_node(&Parameter(node)->type, state);
			traverse_node(&Parameter(node)->value, state);
			break;
		case NODE_Builtin:
			traverse_node(&Builtin(node)->return_type, state);
			break;
	}

	leave_node(&state->scopes, node);

	if (state->leave) {
		state->leave(nodeptr, &state->scopes, state->data);
	}
}

void tarot_traverse(
	struct tarot_node **nodeptr,
	tarot_visitor_function enter,
	tarot_visitor_function leave,
	void *data
) {
	struct tarot_node *node = *nodeptr;
	struct traverse_state state;
	memset(&state, 0, sizeof(state));
	state.enter = enter;
	state.leave = leave;
	state.data = data;
	if (kind_of(node) == NODE_Module) {
		while (node != NULL) {
			struct tarot_node *next_module = Module(node)->next_module;
			traverse_node(&node, &state);
			node = next_module;
		}
	} else {
		traverse_node(nodeptr, &state);
	}
}

void tarot_traverse_preorder(
	struct tarot_node *node,
	tarot_visitor_function visitor,
	void *data
) {
	tarot_traverse(&node, visitor, NULL, data);
}

void tarot_traverse_postorder(
	struct tarot_node *node,
	tarot_visitor_function visitor,
	void *data
) {
	tarot_traverse(&node, NULL, visitor, data);
}

/******************************************************************************
 * MARK: Print
 *****************************************************************************/

static void print_node_kind(
	struct tarot_iostream *stream,
	struct tarot_node *node
) {
	tarot_fprintf(stream, "[%p] ", node);
	switch (class_of(node)) {
		default:
		case CLASS_BLOCK:
			tarot_format(stream, TAROT_COLOR_BLUE);
			break;
		case CLASS_DEFINITION:
			tarot_format(stream, TAROT_COLOR_PURPLE);
			break;
		case CLASS_EXPRESSION:
			tarot_format(stream, TAROT_COLOR_YELLOW);
			break;
		case CLASS_STATEMENT:
			tarot_format(stream, TAROT_COLOR_CYAN);
			break;
	}
	tarot_format(stream, TAROT_COLOR_BOLD);
	tarot_fprintf(stream, "%s ", node_string(kind_of(node)));
	tarot_format(stream, TAROT_COLOR_RESET);
}

static void print_node(
	struct tarot_iostream *stream,
	struct tarot_node *node
) {
	print_node_kind(stream, node);
	switch (kind_of(node)) {
		default:
			break;
		case NODE_Block:
			tarot_fprintf(stream, "num_elements:%zu", Block(node)->num_elements);
			break;
		case NODE_Module:
			tarot_fprintf(stream, "num_errors:%zu ", Module(node)->num_errors);
			tarot_fprintf(stream, "num_nodes:%zu ", Module(node)->num_nodes);
			tarot_fprintf(stream, "is_root:%s ", tarot_bool_string(Module(node)->is_root));
			break;
		case NODE_Identifier:
			tarot_fputs(stream, "name:");
			tarot_print_string(stream, name_of(node));
			tarot_fprintf(stream, " link:%p definition:%p ", Identifier(node)->link, definition_of(node));
			break;
		case NODE_Enumerator:
			tarot_print_string(stream, name_of(node));
			break;
		case NODE_Class:
			tarot_fputs(stream, "name:");
			tarot_print_string(stream, name_of(node));
			break;
		case NODE_Enum:
		case NODE_Function:
		case NODE_Method:
		case NODE_Variable:
		case NODE_Attribute:
		case NODE_Parameter:
		case NODE_Constant:
			tarot_fputs(stream, "name:");
			tarot_print_string(stream, name_of(node));
			tarot_fprintf(stream, " index:%d", index_of(node));
			break;
		case NODE_Constructor:
			tarot_fprintf(stream, "link:%p", link_of(node));
			tarot_fprintf(stream, " index:%d", index_of(node));
			break;
		case NODE_Relation:
			tarot_fputs(stream, "attribute:");
			tarot_print_string(stream, Relation(node)->child);
			tarot_fprintf(stream, " link:%p ", Relation(node)->link);
			break;
		case NODE_Literal:
			tarot_fprintf(stream, "type:%s ", datatype_string(Literal(node)->type));
			tarot_serialize_node(stream, node);
			break;
		case NODE_Type:
			tarot_fprintf(stream, "type:%s ", datatype_string(Type(node)->type));
			break;
	}
	tarot_newline(stream);
}

static void enter_print(
	struct tarot_node **nodeptr,
	struct scope_stack *stack,
	void *data
) {
	struct tarot_iostream *stream = data;
	unused(stack);
	print_node(stream, *nodeptr);
	tarot_indent(stream, 1);
}

static void leave_print(
	struct tarot_node **nodeptr,
	struct scope_stack *stack,
	void *data
) {
	struct tarot_iostream *stream = data;
	unused(nodeptr);
	unused(stack);
	tarot_indent(stream, -1);
}

void tarot_print_node(
	struct tarot_iostream *stream,
	struct tarot_node *node
) {
	tarot_set_indent_width(stream, 2);
	tarot_traverse(&node, enter_print, leave_print, stream);
}
