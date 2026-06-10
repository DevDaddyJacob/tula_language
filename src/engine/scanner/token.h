#ifndef TULA_ENGINE_SCANNER_TOKEN_H
#define TULA_ENGINE_SCANNER_TOKEN_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "../../config.h"


/**
 * \brief	Represents the maximum number of characters that a token may have.
 */
#define TOKEN_MAX_LENGTH 128


/**
 * \brief	Represents the maximum number of characters that a string token may
 *			have.
 */
#define TOKEN_STRING_MAX_LENGTH 2048

/*
 * def(
 *		tokenIdentifier,	<-	The identifier used to define the token
 *		value,				<-	The string of the textual representation of the token
 *		valueAlt,			<-	The alternate string of the textual representation of the token
 *		isMeta,				<-	If the token is a meta token
 *		isPrimitive,		<-	If the token represents a primitive type value
 *		isKeyword,			<-	If the token represents a keyword
 *		isOperator,			<-	If the token represents an operator
 *	)
 */
#define DEFINE_TOKENS(def)																\
	def(TOK_ERROR,			"<error>",		NULL,		true,	false,	false,	false)	\
	def(TOK_EOS,			"<eos>",		NULL,		true,	false,	false,	false)	\
	def(TOK_IDENT,			"<identifier>",	NULL,		false,	false,	false,	false)	\
	def(TOK_INT8,			"<int8>",		NULL,		false,	true,	false,	false)	\
	def(TOK_UINT8,			"<uint8>",		NULL,		false,	true,	false,	false)	\
	def(TOK_INT16,			"<int16>",		NULL,		false,	true,	false,	false)	\
	def(TOK_UINT16,			"<uint16>",		NULL,		false,	true,	false,	false)	\
	def(TOK_INT32,			"<int32>",		NULL,		false,	true,	false,	false)	\
	def(TOK_UINT32,			"<uint32>",		NULL,		false,	true,	false,	false)	\
	def(TOK_INT64,			"<int64>",		NULL,		false,	true,	false,	false)	\
	def(TOK_UINT64,			"<uint64>",		NULL,		false,	true,	false,	false)	\
	def(TOK_FLOAT,			"<float>",		NULL,		false,	true,	false,	false)	\
	def(TOK_DOUBLE,			"<double>",		NULL,		false,	true,	false,	false)	\
	def(TOK_CHAR,			"<char>",		NULL,		false,	true,	false,	false)	\
	def(TOK_STRING,			"<string>",		NULL,		false,	true,	false,	false)	\
	def(TOK_TRUE,			"true",			NULL,		false,	true,	true,	false)	\
	def(TOK_FALSE,			"false",		NULL,		false,	true,	true,	false)	\
	def(TOK_DEFINE,			"define",		"def",		false,	false,	true,	false)	\
	def(TOK_VARIABLE,		"variable",		"var",		false,	false,	true,	false)	\
	def(TOK_CONSTANT,		"constant",		"const",	false,	false,	true,	false)	\
	def(TOK_FUNC,			"function",		"func",		false,	false,	true,	false)	\
	def(TOK_GLOBAL,			"global",		NULL,		false,	false,	true,	false)	\
	def(TOK_SET,			"set",			NULL,		false,	false,	true,	false)	\
	def(TOK_UNSET,			"unset",		NULL,		false,	false,	true,	false)	\
	def(TOK_IS_SET,			"isSet",		NULL,		false,	false,	true,	false)	\
	def(TOK_EMIT,			"emit",			NULL,		false,	false,	true,	false)	\
	def(TOK_IF,				"if",			NULL,		false,	false,	true,	false)	\
	def(TOK_ELSE,			"else",			NULL,		false,	false,	true,	false)	\
	def(TOK_WHILE,			"while",		NULL,		false,	false,	true,	false)	\
	def(TOK_DO,				"do",			NULL,		false,	false,	true,	false)	\
	def(TOK_FOR,			"for",			NULL,		false,	false,	true,	false)	\
	def(TOK_BREAK,			"break",		NULL,		false,	false,	true,	false)	\
	def(TOK_CONTINUE,		"continue",		NULL,		false,	false,	true,	false)	\
	def(TOK_RETURN,			"return",		NULL,		false,	false,	true,	false)	\
	def(TOK_AND,			"and",			NULL,		false,	false,	true,	false)	\
	def(TOK_OR,				"or",			NULL,		false,	false,	true,	false)	\
	def(TOK_NOT,			"not",			NULL,		false,	false,	true,	false)	\
	def(TOK_PAREN_LEFT,		"(",			NULL,		false,	false,	false,	true)	\
	def(TOK_PAREN_RIGHT,	")",			NULL,		false,	false,	false,	true)	\
	def(TOK_BRACKET_LEFT,	"[",			NULL,		false,	false,	false,	true)	\
	def(TOK_BRACKET_RIGHT,	"]",			NULL,		false,	false,	false,	true)	\
	def(TOK_BRACE_LEFT,		"{",			NULL,		false,	false,	false,	true)	\
	def(TOK_BRACE_RIGHT,	"}",			NULL,		false,	false,	false,	true)	\
	def(TOK_DOT,			".",			NULL,		false,	false,	false,	true)	\
	def(TOK_COMMA,			",",			NULL,		false,	false,	false,	true)	\
	def(TOK_EQUAL,			"=",			NULL,		false,	false,	false,	true)	\
	def(TOK_PLUS,			"+",			NULL,		false,	false,	false,	true)	\
	def(TOK_MINUS,			"-",			NULL,		false,	false,	false,	true)	\
	def(TOK_STAR,			"*",			NULL,		false,	false,	false,	true)	\
	def(TOK_CARET,			"^",			NULL,		false,	false,	false,	true)	\
	def(TOK_SLASH_FWD,		"/",			NULL,		false,	false,	false,	true)	\
	def(TOK_PERCENT,		"%",			NULL,		false,	false,	false,	true)	\
	/* def(TOK_QUOTE_SINGLE,	"'",			NULL,		false,	false,	false,	true) 	*/ \
	/* def(TOK_QUOTE_DOUBLE,	"\"",			NULL,		false,	false,	false,	true)	*/ \
	def(TOK_GREATER_THAN,	">",			NULL,		false,	false,	false,	true)	\
	def(TOK_LESS_THAN,		"<",			NULL,		false,	false,	false,	true)	\
	def(TOK_PLUS_PLUS,		"++",			NULL,		false,	false,	false,	true)	\
	def(TOK_MINUS_MINUS,	"--",			NULL,		false,	false,	false,	true)	\
	def(TOK_EQUAL_EQUAL,	"==",			NULL,		false,	false,	false,	true)	\
	def(TOK_EXCLAM_EQUAL,	"!=",			NULL,		false,	false,	false,	true)	\
	def(TOK_GT_EQUAL,		">=",			NULL,		false,	false,	false,	true)	\
	def(TOK_LT_EQUAL,		"<=",			NULL,		false,	false,	false,	true)	\


/**
 * \brief	An enum representation of all the tokens in the language
 */
typedef enum tula_token_type
{
#define TOKEN_ENUM_DEFINER(identifier, _1, _2, _3, _4, _5, _6) identifier,
	DEFINE_TOKENS(TOKEN_ENUM_DEFINER)
#undef TOKEN_ENUM_DEFINER
	TOTAL_TOKENS
} token_type_t;


typedef struct tula_token
{
	/**
	 * \brief	The token's type
	 */
	token_type_t type;

	/**
	 * \brief	The line the token is found on
	 */
	uint32_t line;

	/**
	 * \brief	The column the token is starts at
	 */
	uint32_t column;

	/**
	 * \brief	Pointer to the start of the token
	 */
	char* content;

	/**
	 * \brief	The length of the token
	 */
	uint32_t contentLength;
} token_t;


typedef struct tula_arr_token
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
	token_t* values;
} arr_token_t;


/**
 * \brief	An array which contains the string of the textual representation of
 *			each token.
 */
extern const char* TOKENS_VALUE[TOTAL_TOKENS];


/**
 * \brief	An array which contains the string of the alternate textual
 *			representation of each token.
 */
extern const char* TOKENS_VALUE_ALT[TOTAL_TOKENS];


/**
 * \brief	An array which contains the booleans indicating if each token is a
 *			meta token or not.
 */
extern const bool TOKENS_IS_META[TOTAL_TOKENS];


/**
 * \brief	An array which contains the booleans indicating if each token
 *			represents a primitive type value.
 */
extern const bool TOKENS_IS_PRIMITIVE[TOTAL_TOKENS];


/**
 * \brief	An array which contains the booleans indicating if each token
 *			represents a keyword.
 */
extern const bool TOKENS_IS_KEYWORD[TOTAL_TOKENS];


/**
 * \brief	An array which contains the booleans indicating if each token
 *			represents an operator.
 */
extern const bool TOKENS_IS_OPERATOR[TOTAL_TOKENS];


token_t* token_new(
	token_type_t type,
	uint32_t line,
	uint32_t column,
	const char* content,
	uint32_t contentLength
);


token_t* token_new_error(uint32_t line, uint32_t column, const char* content);


token_t* token_new_eos(uint32_t line, uint32_t column);


/**
 * \brief           Releases the provided token
 * \param[in]       token: Pointer to the token to free
 */
void token_destroy(token_t* token);


/**
 * \brief           Initializes the provided array
 * \param[in]       array: Pointer to the array to initialize
 */
void arr_token_init(arr_token_t* array);


/**
 * \brief           Releases the provided array
 * \param[in]       array: Pointer to the array to free
 */
void arr_token_destroy(arr_token_t* array);


/**
 * \brief           Writes the provided value to the array
 * \param[in]       array: Pointer to the array to write to
 * \param[in]       value: The value to write to the array
 */
void arr_token_add(arr_token_t* array, const token_t* value);


#ifdef TULA_DEBUGGING
void token_print(const token_t* token);

void arr_token_print(const arr_token_t* array);
#else
#define token_print(_0) ((void)_0)

#define arr_token_print(_0) ((void)_0)
#endif


#endif /* TULA_ENGINE_SCANNER_TOKEN_H */
