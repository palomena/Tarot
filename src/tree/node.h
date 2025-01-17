#ifndef TAROT_NODE_H
#define TAROT_NODE_H
#ifdef TAROT_SOURCE

#include "defines.h"
#include "datatypes/value.h"
#include "system/iostream.h"

/******************************************************************************
 * MARK: Visibility
 *****************************************************************************/

/**
 *
 */
enum tarot_visibility {
	VISIBILITY_NONE,
	VISIBILITY_PRIVATE,
	VISIBILITY_PUBLIC
};

/**
 *
 */
extern const char* visibility_string(enum tarot_visibility kind);

/******************************************************************************
 * MARK: Datatype
 *****************************************************************************/

/**
 *
 */
enum tarot_datatype {
	TYPE_VOID,
	TYPE_BOOLEAN,
	TYPE_INTEGER,
	TYPE_FLOAT,
	TYPE_RATIONAL,
	TYPE_STRING,
	TYPE_LIST,
	TYPE_DICT,
	TYPE_CUSTOM
};

/**
 *
 */
extern const char* datatype_string(enum tarot_datatype kind);

/******************************************************************************
 * MARK: Node Kinds
 *****************************************************************************/

/**
 * Abstract AST node type. The implementation is hidden.
 */
struct tarot_node;

/**
 *
 */
enum tarot_node_class {
	CLASS_ERROR,
	CLASS_BLOCK,
	CLASS_DEFINITION,
	CLASS_EXPRESSION,
	CLASS_STATEMENT
};

/**
 *
 */
extern const char* class_string(enum tarot_node_class kind);

/**
 *
 */
extern enum tarot_node_class class_of(struct tarot_node *node);

/**
 *
 */
enum tarot_node_kind {
	NODE_NULL,
	NODE_ERROR,
	NODE_Module,
	NODE_Block,
	NODE_LogicalExpression,
	NODE_RelationalExpression,
	NODE_ArithmeticExpression,
	NODE_InfixExpression,
	NODE_Not,
	NODE_Neg,
	NODE_Abs,
	NODE_Range,
	NODE_FunctionCall,
	NODE_Relation,
	NODE_Subscript,
	NODE_Pair,
	NODE_Typecast,
	NODE_Literal,
	NODE_List,
	NODE_Dict,
	NODE_FString,
	NODE_FStringString,
	NODE_FStringExpression,
	NODE_Identifier,
	NODE_Enumerator,
	NODE_Type,
	NODE_Import,
	NODE_If,
	NODE_While,
	NODE_For,
	NODE_Match,
	NODE_Case,
	NODE_Assignment,
	NODE_ExpressionStatement,
	NODE_Print,
	NODE_Input,
	NODE_Try,
	NODE_Catch,
	NODE_Raise,
	NODE_Return,
	NODE_Assert,
	NODE_Class,
	NODE_Enum,
	NODE_Constructor,
	NODE_Function,
	NODE_ForeignFunction,
	NODE_Namespace,
	NODE_TypeDefinition,
	NODE_Union,
	NODE_Attribute,
	NODE_Variable,
	NODE_Constant,
	NODE_Parameter,
	NODE_Builtin,
	NODE_Break,
	NODE_Breakpoint
};

/**
 *
 */
extern const char* node_string(enum tarot_node_kind kind);

/**
 *
 */
extern enum tarot_node_kind kind_of(struct tarot_node *node);

/******************************************************************************
 * MARK: Module
 *****************************************************************************/

/**
 *
 */
struct Module {
	char *path;
	struct tarot_node *next_module;
	struct tarot_node *block;
	struct tarot_list *scope;
	size_t num_nodes;
	size_t num_errors;
	bool is_root;
};

/**
 *
 */
extern struct Module* Module(struct tarot_node *node);

/******************************************************************************
 * MARK: Block
 *****************************************************************************/

/**
 *
 */
struct Block {
	struct tarot_node **elements;
	size_t num_elements;
	enum {
		BLOCK_DEFAULT,
		BLOCK_SCOPED,
		BLOCK_LIST
	} kind;
};

/**
 *
 */
extern struct Block* Block(struct tarot_node *node);

/**
 *
 */
extern struct Block* List(struct tarot_node *node);

/**
 *
 */
extern struct Block* Dict(struct tarot_node *node);

/******************************************************************************
 * MARK: LogicalExpr
 *****************************************************************************/

/**
 *
 */
enum LogicalExpressionOperator {
	EXPR_AND,
	EXPR_OR,
	EXPR_XOR
};

/**
 *
 */
extern const char* LogicalExpressionOperatorString(enum LogicalExpressionOperator operator);

/**
 *
 */
struct LogicalExpression {
	struct tarot_node *left_operand;
	struct tarot_node *right_operand;
	enum LogicalExpressionOperator operator;
};

/**
 *
 */
extern struct LogicalExpression* LogicalExpression(struct tarot_node *node);

/******************************************************************************
 * MARK: RelationalExpr
 *****************************************************************************/

/**
 *
 */
enum RelationalExpressionOperator {
	EXPR_LESS, EXPR_LESS_EQUAL,
	EXPR_GREATER, EXPR_GREATER_EQUAL,
	EXPR_EQUAL, EXPR_NOT_EQUAL,
	EXPR_IN
};

/**
 *
 */
extern const char* RelationalExpressionOperatorString(enum RelationalExpressionOperator operator);

/**
 *
 */
struct RelationalExpression {
	struct tarot_node *left_operand;
	struct tarot_node *right_operand;
	enum RelationalExpressionOperator operator;
};

/**
 *
 */
extern struct RelationalExpression* RelationalExpression(struct tarot_node *node);

/******************************************************************************
 * MARK: ArithmeticExpr
 *****************************************************************************/

/**
 *
 */
enum ArithmeticExpressionOperator {
	EXPR_ADD,
	EXPR_SUBTRACT,
	EXPR_MULTIPLY,
	EXPR_DIVIDE,
	EXPR_MODULO,
	EXPR_POWER
};

/**
 *
 */
extern const char* ArithmeticExpressionOperatorString(enum ArithmeticExpressionOperator operator);

/**
 *
 */
struct ArithmeticExpression {
	struct tarot_node *left_operand;
	struct tarot_node *right_operand;
	enum ArithmeticExpressionOperator operator;
};

/**
 *
 */
extern struct ArithmeticExpression* ArithmeticExpression(struct tarot_node *node);

/******************************************************************************
 * MARK: UnaryExpression
 *****************************************************************************/

/**
 *
 */
struct UnaryExpression {
	struct tarot_node *expression;
};

/**
 *
 */
extern struct UnaryExpression* InfixExpression(struct tarot_node *node);

/**
 *
 */
extern struct UnaryExpression* NotExpression(struct tarot_node *node);

/**
 *
 */
extern struct UnaryExpression* NegExpression(struct tarot_node *node);

/**
 *
 */
extern struct UnaryExpression* AbsExpression(struct tarot_node *node);

/******************************************************************************
 * MARK: Range
 *****************************************************************************/

/**
 *
 */
struct RangeExpression {
	struct tarot_node* start;
	struct tarot_node* end;
	struct tarot_node* stepsize;
};

/**
 *
 */
extern struct RangeExpression* RangeExpression(struct tarot_node *node);

/******************************************************************************
 * MARK: FunctionCall
 *****************************************************************************/

/**
 *
 */
struct FunctionCall {
	struct tarot_node *identifier;
	struct tarot_node *arguments;
};

/**
 *
 */
extern struct FunctionCall* FunctionCall(struct tarot_node *node);

/******************************************************************************
 * MARK: Relation
 *****************************************************************************/

/**
 *
 */
struct Relation {
	struct tarot_node *parent;
	struct tarot_string *child;
	struct tarot_node *link;
};

/**
 *
 */
extern struct Relation* Relation(struct tarot_node *node);

/******************************************************************************
 * MARK: Subscript
 *****************************************************************************/

/**
 *
 */
struct Subscript {
	struct tarot_node *identifier;
	struct tarot_node *index;
};

/**
 *
 */
extern struct Subscript* Subscript(struct tarot_node *node);

/******************************************************************************
 * MARK: Pair
 *****************************************************************************/

/**
 *
 */
struct Pair {
	struct tarot_node *key;
	struct tarot_node *value;
};

/**
 *
 */
extern struct Pair* Pair(struct tarot_node *node);

/******************************************************************************
 * MARK: TypeCast
 *****************************************************************************/

/**
 *
 */
struct CastExpression {
	struct tarot_node *operand;
	enum tarot_datatype kind;
};

/**
 *
 */
extern struct CastExpression* CastExpression(struct tarot_node *node);

/******************************************************************************
 * MARK: Literal
 *****************************************************************************/

/**
 *
 */
enum tarot_literal_kind {
	VALUE_BOOL,
	VALUE_FLOAT,
	VALUE_INTEGER,
	VALUE_RATIONAL,
	VALUE_STRING,
	VALUE_RAW_STRING
};

/**
 *
 */
struct Literal {
	union tarot_value value;
	enum tarot_datatype type;
	enum tarot_literal_kind kind;
};

/**
 *
 */
extern struct Literal* Literal(struct tarot_node *node);

/**
 *
 */
extern bool Boolean(struct tarot_node *node);
extern tarot_integer Integer(struct tarot_node *node);
extern tarot_rational Rational(struct tarot_node *node);
extern struct tarot_string* String(struct tarot_node *node);

/******************************************************************************
 * MARK: FString
 *****************************************************************************/

/**
 *
 */
struct FString {
	struct tarot_node **elements;
	size_t num_elements;
};

/**
 *
 */
extern struct FString* FString(struct tarot_node *node);

/******************************************************************************
 * MARK: FString String
 *****************************************************************************/

/**
 *
 */
struct FStringString {
	struct tarot_string *value;
};

/**
 *
 */
extern struct FStringString* FStringString(struct tarot_node *node);

/******************************************************************************
 * MARK: FString Expr
 *****************************************************************************/

/**
 *
 */
struct FStringExpression {
	struct tarot_node *expression;
};

/**
 *
 */
extern struct FStringExpression* FStringExpression(struct tarot_node *node);

/******************************************************************************
 * MARK: Identifier
 *****************************************************************************/

/**
 *
 */
struct Identifier {
	struct tarot_string *name;
	struct tarot_node *link;
};

/**
 *
 */
extern struct Identifier* Identifier(struct tarot_node *node);

/******************************************************************************
 * MARK: Enumerator
 *****************************************************************************/

/**
 *
 */
struct Enumerator {
	struct tarot_string *name;
	struct tarot_node *link;
	size_t index;
};

/**
 *
 */
extern struct Enumerator* Enumerator(struct tarot_node *node);

/******************************************************************************
 * MARK: Type
 *****************************************************************************/

/**
 *
 */
struct Type {
	struct tarot_node *identifier;
	struct tarot_node *subtype;
	enum tarot_datatype type;
};

/**
 *
 */
extern struct Type* Type(struct tarot_node *node);

/******************************************************************************
 * MARK: Import
 *****************************************************************************/

/**
 *
 */
struct ImportStatement {
	struct tarot_string *path;
	struct tarot_string *alias;
	struct tarot_node *identifier;
	struct tarot_node *module_link;
};

/**
 *
 */
extern struct ImportStatement* ImportStatement(struct tarot_node *node);

/******************************************************************************
 * MARK: If Statement
 *****************************************************************************/

/**
 *
 */
struct IfStatement {
	struct tarot_node *condition;
	struct tarot_node *block;
	struct tarot_node *elseif;
	uint16_t middle;
	uint16_t end;
};

/**
 *
 */
extern struct IfStatement* IfStatement(struct tarot_node *node);

/******************************************************************************
 * MARK: While Loop
 *****************************************************************************/

/**
 *
 */
struct WhileLoop {
	struct tarot_node *condition;
	struct tarot_node *block;
	size_t start;
	size_t end;
};

/**
 *
 */
extern struct WhileLoop* WhileLoop(struct tarot_node *node);

/******************************************************************************
 * MARK: For Loop
 *****************************************************************************/

/**
 *
 */
struct ForLoop {
	struct tarot_node *identifier;
	struct tarot_node *expression;
	struct tarot_node *block;
	size_t start;
	size_t end;
};

/**
 *
 */
extern struct ForLoop* ForLoop(struct tarot_node *node);

/******************************************************************************
 * MARK: Match
 *****************************************************************************/

/**
 *
 */
struct MatchStatement {
	struct tarot_node *pattern;
	struct tarot_node *block;
};

/**
 *
 */
extern struct MatchStatement* MatchStatement(struct tarot_node *node);

/******************************************************************************
 * MARK: Case
 *****************************************************************************/

/**
 *
 */
struct CaseStatement {
	struct tarot_node *condition;
	struct tarot_node *block;
};

/**
 *
 */
extern struct CaseStatement* CaseStatement(struct tarot_node *node);

/******************************************************************************
 * MARK: Assignment
 *****************************************************************************/

/**
 *
 */
struct Assignment {
	struct tarot_node *identifier;
	struct tarot_node *value;
};

/**
 *
 */
extern struct Assignment* Assignment(struct tarot_node *node);

/******************************************************************************
 * MARK: ExprStatement
 *****************************************************************************/

/**
 *
 */
struct ExprStatement {
	struct tarot_node *expression;
};

/**
 *
 */
extern struct ExprStatement* ExprStatement(struct tarot_node *node);

/******************************************************************************
 * MARK: Print
 *****************************************************************************/

/**
 *
 */
struct PrintStatement {
	struct tarot_node *arguments;
	bool newline;
};

/**
 *
 */
extern struct PrintStatement* PrintStatement(struct tarot_node *node);

/******************************************************************************
 * MARK: Input
 *****************************************************************************/

/**
 *
 */
struct InputExpression {
	struct tarot_node *prompt;
};

/**
 *
 */
extern struct InputExpression* InputExpression(struct tarot_node *node);

/******************************************************************************
 * MARK: Try
 *****************************************************************************/

/**
 *
 */
struct TryStatement {
	struct tarot_node *block;
	struct tarot_node *handlers;
	size_t end;
	size_t handlers_start;
	size_t handlers_end;
};

/**
 *
 */
extern struct TryStatement* TryStatement(struct tarot_node *node);

/******************************************************************************
 * MARK: Catch
 *****************************************************************************/

/**
 *
 */
struct CatchStatement {
	struct tarot_node *identifiers;
	struct tarot_node *block;
	struct tarot_node *try;
};

/**
 *
 */
extern struct CatchStatement* CatchStatement(struct tarot_node *node);

/******************************************************************************
 * MARK: Raise
 *****************************************************************************/

/**
 *
 */
struct RaiseStatement {
	struct tarot_node *identififer;
	uint16_t uid;
};

/**
 *
 */
extern struct RaiseStatement* RaiseStatement(struct tarot_node *node);

/******************************************************************************
 * MARK: Return
 *****************************************************************************/

/**
 *
 */
struct ReturnStatement {
	struct tarot_node *expression;
	struct tarot_node *function; /* Link to the function it appears in */
};

/**
 *
 */
extern struct ReturnStatement* ReturnStatement(struct tarot_node *node);


/******************************************************************************
 * MARK: Assert
 *****************************************************************************/

/**
 *
 */
struct AssertStatement {
	struct tarot_node *condition;
	struct tarot_node *function;
};

/**
 *
 */
extern struct AssertStatement* AssertStatement(struct tarot_node *node);

/******************************************************************************
 * MARK: Class
 *****************************************************************************/

/**
 *
 */
struct ClassDefinition {
	struct tarot_string *name;
	struct tarot_node *extends;
	struct tarot_node *block;
	struct tarot_list *scope;
	enum tarot_visibility visibility;
};

/**
 *
 */
extern struct ClassDefinition* ClassDefinition(struct tarot_node *node);

/******************************************************************************
 * MARK: Enumeration
 *****************************************************************************/

/**
 *
 */
struct EnumDefinition {
	struct tarot_string *name;
	struct tarot_node *block;
	enum tarot_visibility visibility;
};

/**
 *
 */
extern struct EnumDefinition* EnumDefinition(struct tarot_node *node);

/******************************************************************************
 * MARK: Constructor
 *****************************************************************************/

/**
 *
 */
struct ClassConstructor {
	struct tarot_node *parameters;
	struct tarot_node *return_value;
	struct tarot_node *block;
	struct tarot_node *link;
	struct tarot_list *scope;
	uint16_t index;
};

/**
 *
 */
extern struct ClassConstructor* ClassConstructor(struct tarot_node *node);


/******************************************************************************
 * MARK: Function
 *****************************************************************************/

/**
 *
 */
struct FunctionDefinition {
	struct tarot_string *name;
	struct tarot_node *parameters;
	struct tarot_node *return_value;
	struct tarot_node *block;
	struct tarot_list *scope;
	enum tarot_visibility visibility;
	uint16_t index;
};

/**
 *
 */
extern struct FunctionDefinition* FunctionDefinition(struct tarot_node *node);

/******************************************************************************
 * MARK: ForeignFunction
 *****************************************************************************/

/**
 *
 */
struct ForeignFunction {
	struct tarot_string *name;
	struct tarot_node *parameters;
	struct tarot_node *return_value;
	enum tarot_visibility visibility;
	uint16_t index;
};

/**
 *
 */
extern struct ForeignFunction* ForeignFunction(struct tarot_node *node);

/******************************************************************************
 * MARK: Namespace
 *****************************************************************************/

/**
 *
 */
struct Namespace {
	struct tarot_string *name;
	struct tarot_node *block;
	struct tarot_list *scope;
	enum tarot_visibility visibility;
};

/**
 *
 */
extern struct Namespace* Namespace(struct tarot_node *node);

/******************************************************************************
 * MARK: TypeDefinition
 *****************************************************************************/

/**
 *
 */
struct TypeDefinition {
	struct tarot_string *name;
	struct tarot_node *original;
	enum tarot_visibility visibility;
};

/**
 *
 */
extern struct TypeDefinition* TypeDefinition(struct tarot_node *node);

/******************************************************************************
 * MARK: Union
 *****************************************************************************/

/**
 *
 */
struct UnionDefinition {
	struct tarot_string *name;
	struct tarot_node *block;
	enum tarot_visibility visibility;
};

/**
 *
 */
extern struct UnionDefinition* UnionDefinition(struct tarot_node *node);

/******************************************************************************
 * MARK: Variable
 *****************************************************************************/

/**
 *
 */
struct Variable {
	struct tarot_string *name;
	struct tarot_node *type;
	struct tarot_node *value;
	uint16_t index;
	bool is_constant;
	bool is_set;
};

/**
 *
 */
extern struct Variable* Variable(struct tarot_node *node);

/**
 *
 */
extern struct Variable* Attribute(struct tarot_node *node);

/**
 *
 */
extern struct Variable* Constant(struct tarot_node *node);

/******************************************************************************
 * MARK: Parameter
 *****************************************************************************/

/**
 *
 */
struct Parameter {
	struct tarot_string *name;
	struct tarot_node *type;
	struct tarot_node *value;
	uint16_t index;
};

/**
 *
 */
extern struct Parameter* Parameter(struct tarot_node *node);

/******************************************************************************
 * MARK: Builtin
 *****************************************************************************/

/**
 *
 */
struct Builtin {
	struct tarot_string *name;
	struct tarot_node *builtin_type;
	struct tarot_node *return_type;
};

/**
 *
 */
extern struct Builtin* Builtin(struct tarot_node *node);

/******************************************************************************
 * MARK: Break
 *****************************************************************************/

/**
 *
 */
struct Break {
	struct tarot_node *loop;
};

/**
 *
 */
extern struct Break* Break(struct tarot_node *node);

/******************************************************************************
 * MARK: Breakpoint
 *****************************************************************************/

/**
 *
 */
extern struct UnaryExpression* Breakpoint(struct tarot_node *node);

/******************************************************************************
 * MARK: Node
 *****************************************************************************/

/**
 * Returns the index of the node. For example for a variable
 * it would return the stack position. For a function the function
 * index.
 */
extern uint16_t index_of(struct tarot_node *node);

/**
 * Returns true, if the value of node is computable at compile time.
 */
extern bool is_constant(struct tarot_node *node);

/**
 * Returns the position of the node within sourcecode.
 */
extern struct tarot_stream_position* position_of(struct tarot_node *node);

/**
 * Returns the name of the node.
 */
extern struct tarot_string* name_of(struct tarot_node *node);

/**
 * Returns the visibility of the definition node.
 */
extern enum tarot_visibility visibility_of(struct tarot_node *node);

/**
 * Returns the link of the node of NULL if the node does not have a link property.
 */
extern struct tarot_node* link_of(struct tarot_node *node);

/**
 * Returns the definition of the node.
 */
extern struct tarot_node* definition_of(struct tarot_node *node);

/**
 * Returns the type of the node (castable to struct Type).
 */
extern struct tarot_node* type_of(struct tarot_node *node);

extern struct tarot_list* scope_of(struct tarot_node *node);

/**
 * Creates a new node of the specified node kind.
 */
extern struct tarot_node* tarot_create_node(
	enum tarot_node_kind kind,
	struct tarot_stream_position *position
);

/**
 * Returns the number of nodes that have been created up until that point.
 */
extern size_t tarot_num_nodes(void);

/**
 * Performs a deep copy of the node and its children.
 */
extern struct tarot_node* tarot_copy_node(struct tarot_node *node);

/**
 * Frees the node and all of its children recursively via postorder traversal.
 */
extern void tarot_free_node(struct tarot_node *node);

/* Forward declaration */
struct scope_stack;

/**
 * The visitor function is called on each node when traversing
 * the abstract syntax tree. The first parameter of the visitor function
 * is a pointer to the currently visited node, allowing for in-place
 * replacement of deletion of the node.
 */
typedef void tarot_visitor_function(
	struct tarot_node **node,
	struct scope_stack *stack,
	void *data
);

/**
 * Traverses the abstract syntax tree recursively, starting at node.
 */
extern void tarot_traverse(
	struct tarot_node **node,
	tarot_visitor_function enter,
	tarot_visitor_function leave,
	void *data
);

/**
 * Traverses the abstract syntax tree recursively, starting at node.
 */
extern void tarot_traverse_preorder(
	struct tarot_node *node,
	tarot_visitor_function visitor,
	void *data
);

/**
 * Traverses the abstract syntax tree recursively, starting at node.
 */
extern void tarot_traverse_postorder(
	struct tarot_node *node,
	tarot_visitor_function visitor,
	void *data
);

/**
 * Prints the node and its children recursively to the supplied iostream.
 * The printout is of technical nature and does not represent
 * a serialization to sourcecode.
 */
extern void tarot_print_node(
	struct tarot_iostream *stream,
	struct tarot_node *node
);

/**
 * Serializes the node and its children recursively and prints
 * the result to the supplied iostream.
 */
extern void tarot_serialize_node(
	struct tarot_iostream *stream,
	struct tarot_node *node
);

/**
 * Serializes the node and its children recursively and returns
 * the result as a string.
 */
extern struct tarot_string* tarot_stringize_node(struct tarot_node *node);

/**
 * Performs type-checking, linking, etc
 */
extern void tarot_analyze_ast(struct tarot_node *node);

/**
 * Simplifies constant expressions by calculating them at compile time
 * and replacing them with the result.
 */
extern void simplify_nodeptr(struct tarot_node **nodeptr);

/**
 * Validates the node
 */
extern bool validate_nodeptr(struct tarot_node **nodeptr, struct scope_stack *stack);

#endif /* TAROT_SOURCE */
#endif /* TAROT_NODE_H */
