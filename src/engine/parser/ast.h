#ifndef TULA_ENGINE_PARSER_AST_H
#define TULA_ENGINE_PARSER_AST_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "engine/scanner/token.h"


/*
 * def(
 *		identifier,		<-	The identifier used to define the node type
 *		value,			<-	The string of the textual representation of the type
 *	)
 */
#define DEFINE_AST_NODE_TYPES(def)						\
	def(AST_ERROR,				"<error>")				\
	def(AST_BLOCK,				"block")				\
	def(AST_LITERAL,			"literal")				\
	def(AST_IDENTIFIER,			"identifier")			\
	def(AST_BINARY,				"binary")				\
	def(AST_UNARY,				"unary")				\
	def(AST_PRE_INCREMENT,		"pre-increment")		\
	def(AST_POST_INCREMENT,		"post-increment")		\
	def(AST_PRE_DECREMENT,		"pre-decrement")		\
	def(AST_POST_DECREMENT,		"post-decrement")		\
	def(AST_CALL,				"call")					\
	def(AST_MEMBER,				"member-access")		\
	def(AST_INDEX,				"index-access")			\
	def(AST_IS_SET,				"is-set")				\
	def(AST_ARRAY,				"array")				\
	def(AST_TABLE,				"table")				\
	def(AST_TABLE_ENTRY,		"table-entry")			\
	def(AST_FUNCTION,			"function-value")		\
	def(AST_VAR_DEFINE,			"variable-define")		\
	def(AST_VAR_SET,			"variable-set")			\
	def(AST_VAR_UNSET,			"variable-unset")		\
	def(AST_CONST_DEFINE,		"constant-define")		\
	def(AST_FUNC_DEFINE,		"function-define")		\
	def(AST_IF,					"if")					\
	def(AST_WHILE,				"while")				\
	def(AST_DO_WHILE,			"do-while")				\
	def(AST_FOR,				"for")					\
	def(AST_BREAK,				"break")				\
	def(AST_CONTINUE,			"continue")				\
	def(AST_RETURN,				"return")				\
	def(AST_EXPR_STATEMENT,		"expression-statement")	\


/**
 * \brief	An enum representation of every kind of node in the AST.
 */
typedef enum tula_ast_node_type
{
#define AST_NODE_TYPE_ENUM_DEFINER(identifier, _1) identifier,
	DEFINE_AST_NODE_TYPES(AST_NODE_TYPE_ENUM_DEFINER)
#undef AST_NODE_TYPE_ENUM_DEFINER
	TOTAL_AST_NODE_TYPES
} ast_node_type_t;


typedef struct tula_ast_node ast_node_t;


/**
 * \brief	A dynamic array of borrowed pointers to AST nodes. Used wherever a
 *			node owns an ordered, variable-length group of children (block
 *			statements, call arguments, array elements, and so on).
 */
typedef struct tula_arr_node
{
	/**
	 * \brief	The amount of elements currently in the array.
	 */
	size_t count;

	/**
	 * \brief	The amount of elements the array can currently accommodate.
	 */
	size_t capacity;

	/**
	 * \brief	Pointer to the first element of the array.
	 */
	ast_node_t** values;
} arr_node_t;


/**
 * \brief	A single node in the Abstract Syntax Tree.
 *
 *			Every node carries its kind and the source position of the construct
 *			it represents (inherited from the tokens it was built from). The
 *			payload for each kind lives in the \ref as union; only the member
 *			that matches \ref type is valid.
 *
 *			A node owns every child node and string it references. Releasing a
 *			node via \ref ast_node_destroy recursively releases the whole
 *			subtree beneath it.
 */
struct tula_ast_node
{
	/**
	 * \brief	The kind of node, selecting the active \ref as member.
	 */
	ast_node_type_t type;

	/**
	 * \brief	The line the construct starts on.
	 */
	uint32_t line;

	/**
	 * \brief	The column the construct starts at.
	 */
	uint32_t column;

	union
	{
		/**
		 * \brief	AST_ERROR — a syntax error surfaced as data.
		 */
		struct
		{
			/** \brief	The human readable error message (owned). */
			char* message;
		} error;

		/**
		 * \brief	AST_BLOCK — an ordered group of statements.
		 */
		struct
		{
			/** \brief	The statements making up the block. */
			arr_node_t statements;
		} block;

		/**
		 * \brief	AST_LITERAL — a primitive literal value.
		 */
		struct
		{
			/** \brief	The primitive token type (e.g. TOK_INT32, TOK_STRING). */
			token_type_t literalType;

			/** \brief	The raw textual content of the literal (owned). */
			char* value;
		} literal;

		/**
		 * \brief	AST_IDENTIFIER — a reference to a named entity.
		 */
		struct
		{
			/** \brief	The identifier's name (owned). */
			char* name;
		} identifier;

		/**
		 * \brief	AST_BINARY — an operator combining two sub-expressions.
		 */
		struct
		{
			/** \brief	The operator token type (e.g. TOK_PLUS, TOK_AND). */
			token_type_t op;

			/** \brief	The left-hand operand (owned). */
			ast_node_t* left;

			/** \brief	The right-hand operand (owned). */
			ast_node_t* right;
		} binary;

		/**
		 * \brief	AST_UNARY — a prefix operator applied to one operand.
		 */
		struct
		{
			/** \brief	The operator token type (e.g. TOK_NOT). */
			token_type_t op;

			/** \brief	The operand (owned). */
			ast_node_t* operand;
		} unary;

		/**
		 * \brief	AST_PRE_INCREMENT / AST_POST_INCREMENT /
		 *			AST_PRE_DECREMENT / AST_POST_DECREMENT — an increment or
		 *			decrement of an identifier.
		 */
		struct
		{
			/** \brief	The identifier being incremented or decremented. */
			ast_node_t* target;
		} incdec;

		/**
		 * \brief	AST_CALL — a function call.
		 */
		struct
		{
			/** \brief	The identifier naming the callee (owned). */
			ast_node_t* callee;

			/** \brief	The argument expressions. */
			arr_node_t arguments;
		} call;

		/**
		 * \brief	AST_MEMBER — a "object.name" member access.
		 */
		struct
		{
			/** \brief	The expression being accessed (owned). */
			ast_node_t* object;

			/** \brief	The accessed member's name (owned). */
			char* name;
		} member;

		/**
		 * \brief	AST_INDEX — a "object[subscript]" index access.
		 */
		struct
		{
			/** \brief	The expression being indexed (owned). */
			ast_node_t* object;

			/** \brief	The index expression (owned). */
			ast_node_t* subscript;
		} index;

		/**
		 * \brief	AST_IS_SET — an "isSet(identifier)" query.
		 */
		struct
		{
			/** \brief	The identifier being queried (owned). */
			ast_node_t* target;
		} isSet;

		/**
		 * \brief	AST_ARRAY — an array literal.
		 */
		struct
		{
			/** \brief	The element expressions. */
			arr_node_t elements;
		} array;

		/**
		 * \brief	AST_TABLE — a table literal.
		 */
		struct
		{
			/** \brief	The AST_TABLE_ENTRY nodes making up the table. */
			arr_node_t entries;
		} table;

		/**
		 * \brief	AST_TABLE_ENTRY — a single "[key] = value" pair.
		 */
		struct
		{
			/** \brief	The key expression (owned). */
			ast_node_t* key;

			/** \brief	The value expression (owned). */
			ast_node_t* value;
		} tableEntry;

		/**
		 * \brief	AST_FUNCTION — an anonymous function value.
		 *			AST_FUNC_DEFINE — a named function definition.
		 */
		struct
		{
			/** \brief	Whether the definition targets the global scope. */
			bool isGlobal;

			/** \brief	The function's name (owned), or NULL when anonymous. */
			char* name;

			/** \brief	The parameter AST_IDENTIFIER nodes. */
			arr_node_t parameters;

			/** \brief	The function body block (owned). */
			ast_node_t* body;
		} function;

		/**
		 * \brief	AST_VAR_DEFINE / AST_VAR_SET / AST_CONST_DEFINE — a
		 *			definition or assignment of a variable or constant.
		 */
		struct
		{
			/** \brief	Whether the definition targets the global scope. */
			bool isGlobal;

			/** \brief	The variable or constant name (owned). */
			char* name;

			/** \brief	The assigned value expression (owned). */
			ast_node_t* value;
		} variable;

		/**
		 * \brief	AST_VAR_UNSET — an "unset" of a variable.
		 */
		struct
		{
			/** \brief	Whether the unset targets the global scope. */
			bool isGlobal;

			/** \brief	The variable name (owned). */
			char* name;
		} unset;

		/**
		 * \brief	AST_IF — an if / else-if / else chain.
		 */
		struct
		{
			/** \brief	The branch condition expressions. */
			arr_node_t conditions;

			/** \brief	The branch body blocks, parallel to \ref conditions. */
			arr_node_t bodies;

			/** \brief	The trailing else block (owned), or NULL if absent. */
			ast_node_t* elseBody;
		} conditional;

		/**
		 * \brief	AST_WHILE / AST_DO_WHILE — a conditional loop.
		 */
		struct
		{
			/** \brief	The loop condition expression (owned). */
			ast_node_t* condition;

			/** \brief	The loop body block (owned). */
			ast_node_t* body;
		} loop;

		/**
		 * \brief	AST_FOR — a numeric for loop.
		 */
		struct
		{
			/** \brief	The initializer statement (owned), or NULL. */
			ast_node_t* initializer;

			/** \brief	The condition expression (owned), or NULL. */
			ast_node_t* condition;

			/** \brief	The update statement (owned), or NULL. */
			ast_node_t* update;

			/** \brief	The loop body block (owned). */
			ast_node_t* body;
		} forLoop;

		/**
		 * \brief	AST_RETURN — a return statement.
		 */
		struct
		{
			/** \brief	The returned value expression (owned), or NULL. */
			ast_node_t* value;
		} ret;

		/**
		 * \brief	AST_EXPR_STATEMENT — an expression used as a statement.
		 */
		struct
		{
			/** \brief	The wrapped expression (owned). */
			ast_node_t* expression;
		} exprStatement;
	} as;
};


/**
 * \brief	An array which contains the string of the textual representation of
 *			each AST node type.
 */
extern const char* AST_NODE_TYPE_VALUE[TOTAL_AST_NODE_TYPES];


/**
 * \brief           Allocates a new, zero-initialized AST node of the given kind
 * \param[in]       type: The kind of node to create
 * \param[in]       line: The source line the construct starts on
 * \param[in]       column: The source column the construct starts at
 * \return          Returns a pointer to the new node
 * \note            Every embedded child pointer is NULL and every embedded
 *                  \ref arr_node_t is empty; the caller fills in the members
 *                  relevant to \p type. Exits fatally on allocation failure.
 */
ast_node_t* ast_node_new(ast_node_type_t type, uint32_t line, uint32_t column);


/**
 * \brief           Allocates a new AST_ERROR node carrying a message
 * \param[in]       line: The source line the error was detected on
 * \param[in]       column: The source column the error was detected at
 * \param[in]       message: The error message to copy into the node
 * \return          Returns a pointer to the new error node
 * \note            Exits fatally on allocation failure.
 */
ast_node_t* ast_node_new_error(
	uint32_t line,
	uint32_t column,
	const char* message
);


/**
 * \brief           Recursively releases a node and every child it owns
 * \param[in]       node: Pointer to the node to free (a NULL is a safe no-op)
 */
void ast_node_destroy(ast_node_t* node);


/**
 * \brief           Initializes the provided array
 * \param[in]       array: Pointer to the array to initialize
 */
void arr_node_init(arr_node_t* array);


/**
 * \brief           Releases the provided array and every node it owns
 * \param[in]       array: Pointer to the array whose contents to free
 */
void arr_node_destroy(arr_node_t* array);


/**
 * \brief           Appends a node pointer to the array, growing it as needed
 * \param[in]       array: Pointer to the array to write to
 * \param[in]       value: The node pointer to write to the array
 */
void arr_node_add(arr_node_t* array, ast_node_t* value);


#ifdef TULA_EXE_DEBUGGING
extern const char* AST_NODE_TYPE_NAME[TOTAL_AST_NODE_TYPES];

void ast_node_print(const ast_node_t* node);
#endif /* TULA_EXE_DEBUGGING */


#endif /* TULA_ENGINE_PARSER_AST_H */
