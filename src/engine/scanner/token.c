// ReSharper disable CppPrintfBadFormat
#include "token.h"

#include <stdio.h>

#include "util.h"

/*
 * ==================================================
 * Macros
 * ==================================================
*/

#define TOKEN_VALUE_DEFINER(identifier, value, _2, _3, _4, _5, _6) \
	[identifier] = value,

#define TOKEN_VALUE_ALT_DEFINER(identifier, _1, valueAlt, _3, _4, _5, _6) \
	[identifier] = valueAlt,

#define TOKEN_IS_META_DEFINER(identifier, _1, _2, isMeta, _4, _5, _6) \
	[identifier] = isMeta,

#define TOKEN_IS_PRIMITIVE_DEFINER(identifier, _1, _2, _3, isPrimitive, _5, _6) \
	[identifier] = isPrimitive,

#define TOKEN_IS_KEYWORD_DEFINER(identifier, _1, _2, _3, _4, isKeyword, _6) \
	[identifier] = isKeyword,

#define TOKEN_IS_OPERATOR_DEFINER(identifier, _1, _2, _3, _4, _5, isOperator) \
	[identifier] = isOperator,


/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

/**
 * TBD
 */
/* static void example(); */


/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
 */

const char* TOKENS_VALUE[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_VALUE_DEFINER)
};

const char* TOKENS_VALUE_ALT[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_VALUE_ALT_DEFINER)
};

const bool TOKENS_IS_META[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_IS_META_DEFINER)
};

const bool TOKENS_IS_PRIMITIVE[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_IS_PRIMITIVE_DEFINER)
};

const bool TOKENS_IS_KEYWORD[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_IS_KEYWORD_DEFINER)
};

const bool TOKENS_IS_OPERATOR[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_IS_OPERATOR_DEFINER)
};


/*
 * ==================================================
 * Function Definitions
 * ==================================================
 */

void token_destroy(token_t* token)
{
	if (NULL == token)
	{
		return;
	}

	if (NULL != token->content)
	{
		free(token->content);
	}

	free(token);
}


void arr_token_init(arr_token_t* array)
{
	if (NULL == array)
	{
		return;
	}

	array->count = 0;
	array->capacity = 0;
	array->values = NULL;
}


void arr_token_destroy(arr_token_t* array)
{
	if (NULL == array)
	{
		return;
	}

	for (size_t i = 0; i < array->count; i++)
	{
		token_destroy(array->values + i);
	}

	free(array);
}


void arr_token_add(arr_token_t* array, const token_t* value)
{
	if (NULL == array || NULL == value)
	{
		return;
	}

	/* Check if we need to expand the array size */
	if (array->capacity < array->count + 1) {
		const size_t oldCapacity = array->capacity;
		array->capacity = tula_array_grow_capacity(oldCapacity);

		array->values = tula_array_resize(
			token_t,
			array->values,
			oldCapacity,
			array->capacity
		);
	}

	/* Add the value to the end of the array */
	array->values[array->count] = *value;
	array->count++;
}


#ifdef TULA_DEBUGGING
void token_print(const token_t* token)
{
	printf(
		"<token_t> {" \
			"type: \"%s\", " \
			"line: %d, " \
			"column: %d, " \
			"content: \"%s\", " \
			"contentLength: %d" \
		"}",
		TOKENS_VALUE[token->type],
		token->line,
		token->column,
		token->content,
		token->contentLength
	);
}


void arr_token_print(const arr_token_t* array)
{
	if (NULL == array->values)
	{
		printf("<arr_token_t> []");
		return;
	}

	printf("<arr_token_t> [\n");

	for (size_t i = 0; i < array->capacity; i++)
	{
		printf("\t%llu: ", i);

		if (i < array->count)
		{
			token_print(array->values + i);
		}
		else
		{
			// ReSharper disable once CppPrintfBadFormat
			printf("%s", NULL);
		}

		if (i + 1 < array->capacity)
		{
			printf(",");
		}

		printf("\n");
	}

	printf("]");
}
#endif