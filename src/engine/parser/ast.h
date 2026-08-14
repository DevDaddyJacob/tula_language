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
#define DEFINE_AST_NODE_TYPES(def)										\
	def(AST_ERROR,					"<error>")							\
	def(AST_PROGRAM,				"program")							\
	def(AST_BLOCK,					"block")							\
	def(AST_STATEMENT,				"statement")						\
	def(AST_STMT_RETURN,			"return")							\
	def(AST_STMT_NUM_ITER,			"numeric-iteration")				\
	def(AST_STMT_COND_ITER,			"conditional-iteration")			\
	def(AST_STMT_COMP,				"comparison-statement")				\
	def(AST_STMT_FUNC_CALL,			"function-call-statement")			\
	def(AST_STMT_CONST_DEF,			"constant-definition-statement")	\
	def(AST_STMT_VAR_DEF,			"variable-definition-statement")	\
	def(AST_STMT_VAR_SET,			"variable-set-statement")			\
	def(AST_STMT_VAR_UNSET,			"variable-unset-statement")			\
	def(AST_STMT_IS_SET,			"is-set-statement")					\
	def(AST_STMT_BREAK,				"break-statement")					\
	def(AST_STMT_CONTINUE,			"continue-statement")				\
	def(AST_CONDITION,				"condition")						\
	def(AST_EXPR_IDENT,				"expression-identifier")			\
	def(AST_EXPR,					"expression")						\
	def(AST_EXPR_LOGI_OR,			"expression-logical-or")			\
	def(AST_EXPR_LOGI_AND,			"expression-logical-and")			\
	def(AST_EXPR_EQU,				"expression-equality")				\
	def(AST_EXPR_COMP,				"expression-comparison")			\
	def(AST_EXPR_ADD,				"expression-additive")				\
	def(AST_EXPR_MULT,				"expression-multiplicative")		\
	def(AST_EXPR_EXPO,				"expression-exponent")				\
	def(AST_EXPR_UNARY,				"expression-unary")					\
	def(AST_EXPR_POSTFIX,			"expression-postfix")				\
	def(AST_EXPR_PRIMARY,			"expression-primary")				\
	def(AST_FUNC_DEF,				"function-definition")				\
	def(AST_FUNC_DEF_VAL,			"function-definition-value")		\
	def(AST_ACCESSOR_MEMBER,		"accessor-member")					\
	def(AST_ACCESSOR_INDEX,			"accessor-index")					\
	def(AST_ACCESSOR_CALL,			"accessor-call")					\
	def(AST_TABLE_DEF,				"table-definition")					\
	def(AST_TABLE_FIELD,			"table-field")						\
	def(AST_ARRAY_DEF,				"array-definition")					\
	def(AST_LITERAL,				"literal")							\
	def(AST_IDENTIFIER,				"identifier")						\


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
typedef struct tula_arr_ast_node
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
} arr_ast_node_t;


/**
 * \brief	A single node in the Abstract Syntax Tree.
 *
 *			Every node carries its kind and the source position of the construct
 *			it represents (inherited from the tokens it was built from). The
 *			payload for each kind lives in the @c as union; only the member
 *			that matches @c type is valid.
 *
 *			A node owns every child node and string it references. Releasing a
 *			node via @c ast_node_destroy recursively releases the whole
 *			subtree beneath it.
 */
struct tula_ast_node
{
	/**
	 * \brief	The kind of node, selecting the active @c as member.
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
		/** \brief	@c AST_ERROR */
		struct
		{
			/** \brief	The human readable error message (owned). */
			char* message;
		} error;

		/** \brief	@c AST_PROGRAM */
		struct
		{
			/** \brief	The statements making up the program. */
			arr_ast_node_t statements;
		} program;

		/** \brief	@c AST_BLOCK */
		struct
		{
			/** \brief	The statements making up the block. */
			arr_ast_node_t statements;
		} block;

		/** \brief	@c AST_STATEMENT */
		struct
		{
			/** \brief	The statement (@c AST_STMT_*). */
			ast_node_t* statement;
		} statement;

		/** \brief	@c AST_STMT_RETURN */
		struct
		{
			/** \brief	The returned value expression (owned), or @c NULL. */
			ast_node_t* value;
		} returnStmt;

		/** \brief	@c AST_STMT_NUM_ITER */
		struct
		{
			/** \brief	The local definition (@c AST_STMT_VAR_DEF) */
			ast_node_t* initialization;

			/** \brief	The condition (@c AST_CONDITION) */
			ast_node_t* condition;

			/** \brief	The increment */
			ast_node_t* increment;

			/** \brief	The code block  (@c AST_BLOCK) */
			ast_node_t* block;
		} numericIteration;

		/** \brief	@c AST_STMT_COND_ITER */
		struct
		{
			/** \brief	If the loop is a do-while */
			bool doMode;

			/** \brief	The condition (@c AST_CONDITION) */
			ast_node_t* condition;

			/** \brief	The code block  (@c AST_BLOCK) */
			ast_node_t* block;
		} conditionalIteration;

		/** \brief	@c AST_STMT_COMP */
		struct
		{
			/** \brief	The condition (@c AST_CONDITION) */
			ast_node_t* condition;

			/** \brief	The code block (@c AST_BLOCK) */
			ast_node_t* block;

			/** \brief	The else node (@c AST_STMT_COMP), or @c NULL */
			ast_node_t* elseNode;
		} comparison;

		/** \brief	@c AST_STMT_FUNC_CALL */
		struct
		{
			/**
			 * \brief	The callee (@c AST_IDENTIFIER, @c AST_ACCESSOR_CALL,
			 *			@c AST_ACCESSOR_INDEX, @c AST_ACCESSOR_MEMBER, or
			 *			@c AST_EXPR_PRIMARY)
			 */
			ast_node_t* callee;

			/** \brief	The arguments (@c AST_EXPR) */
			arr_ast_node_t elseNode;
		} functionCall;

		/** \brief	@c AST_STMT_CONST_DEF */
		struct
		{
			/** \brief	If the constant is a global constant */
			bool isGlobal;

			/** \brief	The constant identifier (@c AST_IDENTIFIER) */
			ast_node_t* identifier;

			/** \brief	The constant expression (@c AST_EXPR) */
			ast_node_t* expression;
		} constantDef;

		/** \brief	@c AST_STMT_VAR_DEF */
		struct
		{
			/** \brief	If the variable is a global variable */
			bool isGlobal;

			/** \brief	The variable identifier (@c AST_IDENTIFIER) */
			ast_node_t* identifier;

			/** \brief	The variable expression (@c AST_EXPR) */
			ast_node_t* expression;
		} variableDef;

		/** \brief	@c AST_STMT_VAR_SET */
		struct
		{
			/** \brief	If the variable is a global variable */
			bool isGlobal;

			/** \brief	The variable identifier (@c AST_IDENTIFIER) */
			ast_node_t* identifier;

			/** \brief	The variable expression (@c AST_EXPR) */
			ast_node_t* expression;
		} variableSet;

		/** \brief	@c AST_STMT_VAR_UNSET */
		struct
		{
			/** \brief	If the variable is a global variable */
			bool isGlobal;

			/** \brief	The variable identifier (@c AST_IDENTIFIER) */
			ast_node_t* identifier;
		} variableUnset;

		/** \brief	@c AST_STMT_IS_SET */
		struct
		{
			/** \brief	The variable identifier (@c AST_IDENTIFIER) */
			ast_node_t* identifier;
		} isSet;

		/** \brief	@c AST_CONDITION */
		struct
		{
			/** \brief	The expression (@c AST_EXPR) */
			ast_node_t* expression;
		} condition;

		/** \brief	@c AST_EXPR_IDENT */
		struct
		{
			/** \brief	The expression identifier (@c AST_IDENTIFIER) */
			ast_node_t* identifier;

			/**
			 *	\brief	The expression accessors (@c AST_ACCESSOR_MEMBER, or
			 *			@c AST_ACCESSOR_INDEX)
			 */
			arr_ast_node_t accessors;
		} expressionIdentifier;

		/** \brief	@c AST_EXPR */
		struct
		{
			/** \brief	The logical or expression (@c AST_EXPR_LOGI_OR) */
			ast_node_t* lhs;
		} expression;

		/** \brief	@c AST_EXPR_LOGI_OR */
		struct
		{
			/**
			 *	\brief	The left logical and expression (@c AST_EXPR_LOGI_AND)
			 */
			ast_node_t* lhs;

			/**
			 *	\brief	The right logical and expression (@c AST_EXPR_LOGI_AND),
			 *			or @c NULL
			 */
			ast_node_t* rhs;
		} expressionLogicalOr;

		/** \brief	@c AST_EXPR_LOGI_AND */
		struct
		{
			/** \brief	The left equality expression (@c AST_EXPR_EQU) */
			ast_node_t* lhs;

			/**
			 *	\brief	The right equality expression (@c AST_EXPR_EQU),
			 *			or @c NULL
			 */
			ast_node_t* rhs;
		} expressionLogicalAnd;

		/** \brief	@c AST_EXPR_EQU */
		struct
		{
			/** \brief	The left comparison expression (@c AST_EXPR_COMP) */
			ast_node_t* lhs;

			/**
			 *	\brief	The operator token type (@c TOK_EQUAL_EQUAL or
			 *			@c TOK_EXCLAM_EQUAL), or @c TOK_NONE
			 */
			token_type_t op;

			/**
			 *	\brief	The right comparison expression (@c AST_EXPR_COMP),
			 *			or @c NULL
			 */
			ast_node_t* rhs;
		} expressionEquality;

		/** \brief	@c AST_EXPR_COMP */
		struct
		{
			/** \brief	The left additive expression (@c AST_EXPR_ADD) */
			ast_node_t* lhs;

			/**
			 *	\brief	The operator token type (@c TOK_GREATER_THAN,
			 *			@c TOK_LESS_THAN, @c TOK_GT_EQUAL, or @c TOK_LT_EQUAL),
			 *			or @c TOK_NONE
			 */
			token_type_t op;

			/**
			 *	\brief	The right additive expression (@c AST_EXPR_ADD),
			 *			or @c NULL
			 */
			ast_node_t* rhs;
		} expressionComparison;

		/** \brief	@c AST_EXPR_ADD */
		struct
		{
			/**
			 *	\brief	The left multiplicative expression (@c AST_EXPR_MULT)
			 */
			ast_node_t* lhs;

			/**
			 *	\brief	The operator token type (@c TOK_PLUS, or @c TOK_MINUS),
			 *			or @c TOK_NONE
			 */
			token_type_t op;

			/**
			 *	\brief	The right multiplicative expression (@c AST_EXPR_MULT),
			 *			or @c NULL
			 */
			ast_node_t* rhs;
		} expressionAdditive;

		/** \brief	@c AST_EXPR_MULT */
		struct
		{
			/**
			 *	\brief	The left exponent expression (@c AST_EXPR_EXPO)
			 */
			ast_node_t* lhs;

			/**
			 *	\brief	The operator token type (@c TOK_STAR, or
			 *			@c TOK_SLASH_FWD), or @c TOK_NONE
			 */
			token_type_t op;

			/**
			 *	\brief	The right exponent expression (@c AST_EXPR_EXPO),
			 *			or @c NULL
			 */
			ast_node_t* rhs;
		} expressionMultiplicative;

		/** \brief	@c AST_EXPR_EXPO */
		struct
		{
			/**
			 *	\brief	The left unary expression (@c AST_EXPR_UNARY)
			 */
			ast_node_t* lhs;

			/**
			 *	\brief	The right exponent expression (@c AST_EXPR_EXPO),
			 *			or @c NULL
			 */
			ast_node_t* rhs;
		} expressionExponent;

		/** \brief	@c AST_EXPR_UNARY */
		struct
		{
			/**
			 *	\brief	The operator token type (@c TOK_NOT, @c TOK_PLUS_PLUS,
			 *			or @c TOK_MINUS_MINUS), or @c TOK_NONE
			 */
			token_type_t op;

			/**
			 *	\brief	The right expression (@c AST_EXPR_UNARY,
			 *			@c AST_EXPR_IDENT, or @c AST_EXPR_POSTFIX)
			 */
			ast_node_t* rhs;
		} expressionUnary;

		/** \brief	@c AST_EXPR_POSTFIX */
		struct
		{
			/**
			 *	\brief	The left primary expression (@c AST_EXPR_PRIMARY)
			 */
			ast_node_t* lhs;

			/**
			 *	\brief	The operator token type (@c TOK_PLUS_PLUS, or
			 *			@c TOK_MINUS_MINUS), or @c TOK_NONE
			 */
			token_type_t op;

			/**
			 *	\brief	The right accessor expressions (@c AST_ACCESSOR_MEMBER,
			 *			@c AST_ACCESSOR_INDEX, or @c AST_ACCESSOR_CALL)
			*/
			arr_ast_node_t accessors;
		} expressionPostfix;

		/** \brief	@c AST_EXPR_PRIMARY */
		struct
		{
			/**
			 *	\brief	The expression (@c AST_LITERAL, @c AST_IDENTIFIER,
			 *			@c AST_STMT_IS_SET, @c AST_FUNC_DEF_VAL, @c AST_ARRAY_DEF,
			 *			@c AST_TABLE_FIELD, or @c AST_EXPR)
			*/
			ast_node_t* expression;
		} expressionPrimary;

		/** \brief	@c AST_FUNC_DEF */
		struct
		{
			/** \brief	Whether the definition targets the global scope */
			bool isGlobal;

			/** \brief	The function's name (owned) */
			char* name;

			/** \brief	The function def value node (@c AST_FUNC_DEF_VAL) */
			ast_node_t* body;
		} functionDef;

		/** \brief	@c AST_FUNC_DEF_VAL */
		struct
		{
			/** \brief	The parameter nodes (@c AST_IDENTIFIER) */
			arr_ast_node_t parameters;

			/** \brief	The function body block node (@c AST_BLOCK) */
			ast_node_t* body;
		} functionDefValue;

		/** \brief	@c AST_ACCESSOR_MEMBER */
		struct
		{
			/** \brief	The identifier (@c AST_IDENTIFIER) */
			ast_node_t* identifier;
		} accessorMember;

		/** \brief	@c AST_ACCESSOR_INDEX */
		struct
		{
			/** \brief	The expression (@c AST_EXPR) */
			ast_node_t* expression;
		} accessorIndex;

		/** \brief	@c AST_ACCESSOR_CALL */
		struct
		{
			/** \brief	The expressions (@c AST_EXPR) */
			arr_ast_node_t expressions;
		} accessorCall;

		/** \brief	@c AST_TABLE_DEF */
		struct
		{
			/** \brief	The table fields (@c AST_TABLE_FIELD) */
			arr_ast_node_t fields;
		} tableDef;

		/** \brief	@c AST_TABLE_FIELD */
		struct
		{
			/** \brief	The field key (@c AST_EXPR) */
			ast_node_t* key;

			/** \brief	The field value (@c AST_EXPR) */
			ast_node_t* value;
		} tableField;

		/** \brief	@c AST_ARRAY_DEF */
		struct
		{
			/** \brief	The array elements (@c AST_EXPR) */
			arr_ast_node_t elements;
		} arrayDef;

		/** \brief	@c AST_LITERAL */
		struct
		{
			/**
			 *	\brief	The token type (@c TOK_INT8, @c TOK_UINT8, @c TOK_INT16,
			 *			@c TOK_UINT16, @c TOK_INT32, @c TOK_UINT32,
			 *			@c TOK_INT64, @c TOK_UINT64, @c TOK_FLOAT,
			 *			@c TOK_DOUBLE, @c TOK_CHAR, @c TOK_STRING,
			 *			@c TOK_TRUE, or @c TOK_FALSE)
			 */
			token_type_t type;

			union
			{
				char _dummy;
				int8_t t_int8;
				uint8_t t_uint8;
				int16_t t_int16;
				uint16_t t_uint16;
				int32_t t_int32;
				uint32_t t_uint32;
				int64_t t_int64;
				uint64_t t_uint64;
				float t_float;
				double t_double;
				char t_char;
				char* t_string;
			} value;
		} literal;

		/** \brief	@c AST_IDENTIFIER */
		struct
		{
			/** \brief	name of the identifier (owned) */
			char* name;
		} identifier;
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
 *                  @c arr_node_t is empty; the caller fills in the members
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
void arr_ast_node_init(arr_ast_node_t* array);


/**
 * \brief           Releases the provided array and every node it owns
 * \param[in]       array: Pointer to the array whose contents to free
 */
void arr_ast_node_destroy(arr_ast_node_t* array);


/**
 * \brief           Appends a node pointer to the array, growing it as needed
 * \param[in]       array: Pointer to the array to write to
 * \param[in]       value: The node pointer to write to the array
 */
void arr_ast_node_add(arr_ast_node_t* array, ast_node_t* value);


#ifdef TULA_EXE_DEBUG
extern const char* AST_NODE_TYPE_NAME[TOTAL_AST_NODE_TYPES];

void ast_node_print(const ast_node_t* node);
#endif /* TULA_EXE_DEBUG */


#endif /* TULA_ENGINE_PARSER_AST_H */
