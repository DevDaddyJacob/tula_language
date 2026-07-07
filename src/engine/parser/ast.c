#include "ast.h"

/*
 * ==================================================
 * Macros
 * ==================================================
 */

/* #define XYZ "ABC" */

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

/* int magicNumber = 420; */


/*
 * ==================================================
 * Function Definitions
 * ==================================================
 */

void arr_ast_node_init(arr_ast_node_t* array)
{
	if (NULL == array)
	{
		return;
	}

	array->count = 0;
	array->capacity = 0;
	array->nodes = NULL;
}


void arr_ast_node_destroy(arr_ast_node_t* array)
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


void arr_ast_node_add(arr_ast_node_t* array, const arr_ast_node_t* value)
{

}
