#include "ast.h"

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "common/exit.h"
#include "common/strings.h"

/*
 * ==================================================
 * Macros
 * ==================================================
 */

#define AST_NODE_TYPE_VALUE_DEFINER(identifier, value) \
	[identifier] = value,

#define AST_NODE_TYPE_NAME_DEFINER(identifier, _1) \
	[identifier] = #identifier,


/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

#ifdef TULA_EXE_DEBUG
/**
 * \brief           Recursively prints a node as an indented tree
 * \param[in]       node: The node to print
 * \param[in]       label: A short role label for the node (e.g. "left"), or
 *                  NULL for no label
 * \param[in]       depth: The current indentation depth
 */
static void ast_node_print_indented(
	const ast_node_t* node,
	const char* label,
	uint32_t depth
);
#endif /* TULA_EXE_DEBUG */


/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
 */

const char* AST_NODE_TYPE_VALUE[TOTAL_AST_NODE_TYPES] = {
	DEFINE_AST_NODE_TYPES(AST_NODE_TYPE_VALUE_DEFINER)
};

#ifdef TULA_EXE_DEBUG
const char* AST_NODE_TYPE_NAME[TOTAL_AST_NODE_TYPES] = {
	DEFINE_AST_NODE_TYPES(AST_NODE_TYPE_NAME_DEFINER)
};
#endif /* TULA_EXE_DEBUG */


/*
 * ==================================================
 * Function Definitions
 * ==================================================
 */

ast_node_t* ast_node_new(
	const ast_node_type_t type,
	const uint32_t line,
	const uint32_t column
)
{
	ast_node_t* node = malloc(sizeof(ast_node_t));
	if (NULL == node)
	{
		tula_exit_err_no_mem();
		UNREACHABLE_RETURN(NULL);
	}

	/* Zeroing leaves every child pointer NULL and every array empty */
	memset(node, 0, sizeof(ast_node_t));

	node->type = type;
	node->line = line;
	node->column = column;

	return node;
}


ast_node_t* ast_node_new_error(
	const uint32_t line,
	const uint32_t column,
	const char* message
)
{
	ast_node_t* node = ast_node_new(AST_ERROR, line, column);
	node->as.error.message = NULL;

	const size_t size = strlen(message) + 1;

	node->as.error.message = malloc(sizeof(char) * size);
	if (NULL == node->as.error.message)
	{
		tula_exit_err_no_mem();
		UNREACHABLE_RETURN(NULL);
	}

	str_copy_safe(node->as.error.message, message, size);

	return node;
}


void ast_node_destroy(ast_node_t* node)
{
	if (NULL == node)
	{
		return;
	}

	/* Release every child and string this node owns */
	switch (node->type)
	{
		case AST_ERROR:
		{
			free(node->as.error.message);
			break;
		}

		case AST_BLOCK:
		{
			arr_node_destroy(&node->as.block.statements);
			break;
		}

		case AST_LITERAL:
		{
			free(node->as.literal.value);
			break;
		}

		case AST_IDENTIFIER:
		{
			free(node->as.identifier.name);
			break;
		}

		case AST_BINARY:
		{
			ast_node_destroy(node->as.binary.left);
			ast_node_destroy(node->as.binary.right);
			break;
		}

		case AST_UNARY:
		{
			ast_node_destroy(node->as.unary.operand);
			break;
		}

		case AST_PRE_INCREMENT:
		case AST_POST_INCREMENT:
		case AST_PRE_DECREMENT:
		case AST_POST_DECREMENT:
		{
			ast_node_destroy(node->as.incdec.target);
			break;
		}

		case AST_CALL:
		{
			ast_node_destroy(node->as.call.callee);
			arr_node_destroy(&node->as.call.arguments);
			break;
		}

		case AST_MEMBER:
		{
			ast_node_destroy(node->as.member.object);
			free(node->as.member.name);
			break;
		}

		case AST_INDEX:
		{
			ast_node_destroy(node->as.index.object);
			ast_node_destroy(node->as.index.subscript);
			break;
		}

		case AST_IS_SET:
		{
			ast_node_destroy(node->as.isSet.target);
			break;
		}

		case AST_ARRAY:
		{
			arr_node_destroy(&node->as.array.elements);
			break;
		}

		case AST_TABLE:
		{
			arr_node_destroy(&node->as.table.entries);
			break;
		}

		case AST_TABLE_ENTRY:
		{
			ast_node_destroy(node->as.tableEntry.key);
			ast_node_destroy(node->as.tableEntry.value);
			break;
		}

		case AST_FUNCTION:
		case AST_FUNC_DEFINE:
		{
			free(node->as.function.name);
			arr_node_destroy(&node->as.function.parameters);
			ast_node_destroy(node->as.function.body);
			break;
		}

		case AST_VAR_DEFINE:
		case AST_VAR_SET:
		case AST_CONST_DEFINE:
		{
			free(node->as.variable.name);
			ast_node_destroy(node->as.variable.value);
			break;
		}

		case AST_VAR_UNSET:
		{
			free(node->as.unset.name);
			break;
		}

		case AST_IF:
		{
			arr_node_destroy(&node->as.conditional.conditions);
			arr_node_destroy(&node->as.conditional.bodies);
			ast_node_destroy(node->as.conditional.elseBody);
			break;
		}

		case AST_WHILE:
		case AST_DO_WHILE:
		{
			ast_node_destroy(node->as.loop.condition);
			ast_node_destroy(node->as.loop.body);
			break;
		}

		case AST_FOR:
		{
			ast_node_destroy(node->as.forLoop.initializer);
			ast_node_destroy(node->as.forLoop.condition);
			ast_node_destroy(node->as.forLoop.update);
			ast_node_destroy(node->as.forLoop.body);
			break;
		}

		case AST_RETURN:
		{
			ast_node_destroy(node->as.ret.value);
			break;
		}

		case AST_EXPR_STATEMENT:
		{
			ast_node_destroy(node->as.exprStatement.expression);
			break;
		}

		case AST_BREAK: /* NOLINT(*-branch-clone) */
		case AST_CONTINUE:
		{
			/* No owned children */
			break;
		}

		DEFAULT_BREAK
	}

	free(node);
}


void arr_node_init(arr_ast_node_t* array)
{
	if (NULL == array)
	{
		return;
	}

	array->count = 0;
	array->capacity = 0;
	array->values = NULL;
}


void arr_node_destroy(arr_ast_node_t* array)
{
	if (NULL == array)
	{
		return;
	}

	/* A NULL backing buffer means there is nothing to release */
	if (NULL != array->values)
	{
		for (size_t i = 0; i < array->count; i++)
		{
			ast_node_destroy(array->values[i]);
		}

		tula_array_free(ast_node_t*, array->values, array->capacity);
	}

	array->count = 0;
	array->capacity = 0;
	array->values = NULL;
}


void arr_node_add(arr_ast_node_t* array, ast_node_t* value)
{
	if (NULL == array || NULL == value)
	{
		return;
	}

	/* Check if we need to expand the array size */
	if (array->capacity < array->count + 1)
	{
		const size_t oldCapacity = array->capacity;
		array->capacity = tula_array_grow_capacity(oldCapacity);

		array->values = tula_array_resize(
			ast_node_t*,
			array->values,
			oldCapacity,
			array->capacity
		);
	}

	/* Add the value to the end of the array */
	array->values[array->count] = value;
	array->count++;
}


#ifdef TULA_EXE_DEBUG
static void ast_node_print_indented(
	const ast_node_t* node,
	const char* label,
	const uint32_t depth
)
{
	/* Indent to the node's depth in the tree */
	for (uint32_t i = 0; i < depth; i++)
	{
		printf("  ");
	}

	/* Print the optional role label */
	if (NULL != label)
	{
		printf("%s: ", label);
	}

	/* A null child slot is rendered explicitly so structure stays visible */
	if (NULL == node)
	{
		printf("<none>\n");
		return;
	}

	/* Print the node kind and its source position */
	printf(
		"%s [%u:%u]",
		AST_NODE_TYPE_VALUE[node->type],
		node->line,
		node->column
	);

	/* Print the node's own scalar fields, then recurse into its children */
	switch (node->type)
	{
		case AST_ERROR:
		{
			printf(" message=\"%s\"\n", node->as.error.message);
			break;
		}

		case AST_BLOCK:
		{
			printf("\n");
			for (size_t i = 0; i < node->as.block.statements.count; i++)
			{
				ast_node_print_indented(
					node->as.block.statements.values[i],
					NULL,
					depth + 1
				);
			}
			break;
		}

		case AST_LITERAL:
		{
			printf(
				" type=%s value=\"%s\"\n",
				TOKENS_TYPE_VALUE[node->as.literal.literalType],
				node->as.literal.value
			);
			break;
		}

		case AST_IDENTIFIER:
		{
			printf(" name=\"%s\"\n", node->as.identifier.name);
			break;
		}

		case AST_BINARY:
		{
			printf(" op=%s\n", TOKENS_TYPE_VALUE[node->as.binary.op]);
			ast_node_print_indented(node->as.binary.left, "left", depth + 1);
			ast_node_print_indented(node->as.binary.right, "right", depth + 1);
			break;
		}

		case AST_UNARY:
		{
			printf(" op=%s\n", TOKENS_TYPE_VALUE[node->as.unary.op]);
			ast_node_print_indented(
				node->as.unary.operand,
				"operand",
				depth + 1
			);
			break;
		}

		case AST_PRE_INCREMENT:
		case AST_POST_INCREMENT:
		case AST_PRE_DECREMENT:
		case AST_POST_DECREMENT:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.incdec.target,
				"target",
				depth + 1
			);
			break;
		}

		case AST_CALL:
		{
			printf("\n");
			ast_node_print_indented(node->as.call.callee, "callee", depth + 1);
			for (size_t i = 0; i < node->as.call.arguments.count; i++)
			{
				ast_node_print_indented(
					node->as.call.arguments.values[i],
					"arg",
					depth + 1
				);
			}
			break;
		}

		case AST_MEMBER:
		{
			printf(" name=\"%s\"\n", node->as.member.name);
			ast_node_print_indented(
				node->as.member.object,
				"object",
				depth + 1
			);
			break;
		}

		case AST_INDEX:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.index.object,
				"object",
				depth + 1
			);
			ast_node_print_indented(
				node->as.index.subscript,
				"subscript",
				depth + 1
			);
			break;
		}

		case AST_IS_SET:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.isSet.target,
				"target",
				depth + 1
			);
			break;
		}

		case AST_ARRAY:
		{
			printf("\n");
			for (size_t i = 0; i < node->as.array.elements.count; i++)
			{
				ast_node_print_indented(
					node->as.array.elements.values[i],
					"element",
					depth + 1
				);
			}
			break;
		}

		case AST_TABLE:
		{
			printf("\n");
			for (size_t i = 0; i < node->as.table.entries.count; i++)
			{
				ast_node_print_indented(
					node->as.table.entries.values[i],
					NULL,
					depth + 1
				);
			}
			break;
		}

		case AST_TABLE_ENTRY:
		{
			printf("\n");
			ast_node_print_indented(node->as.tableEntry.key, "key", depth + 1);
			ast_node_print_indented(
				node->as.tableEntry.value,
				"value",
				depth + 1
			);
			break;
		}

		case AST_FUNCTION:
		case AST_FUNC_DEFINE:
		{
			printf(
				" global=%s name=\"%s\"\n",
				node->as.function.isGlobal ? "true" : "false",
				NULL == node->as.function.name
					? "<anonymous>"
					: node->as.function.name
			);
			for (size_t i = 0; i < node->as.function.parameters.count; i++)
			{
				ast_node_print_indented(
					node->as.function.parameters.values[i],
					"param",
					depth + 1
				);
			}
			ast_node_print_indented(node->as.function.body, "body", depth + 1);
			break;
		}

		case AST_VAR_DEFINE:
		case AST_VAR_SET:
		case AST_CONST_DEFINE:
		{
			printf(
				" global=%s name=\"%s\"\n",
				node->as.variable.isGlobal ? "true" : "false",
				node->as.variable.name
			);
			ast_node_print_indented(
				node->as.variable.value,
				"value",
				depth + 1
			);
			break;
		}

		case AST_VAR_UNSET:
		{
			printf(
				" global=%s name=\"%s\"\n",
				node->as.unset.isGlobal ? "true" : "false",
				node->as.unset.name
			);
			break;
		}

		case AST_IF:
		{
			printf("\n");
			for (size_t i = 0; i < node->as.conditional.conditions.count; i++)
			{
				ast_node_print_indented(
					node->as.conditional.conditions.values[i],
					"condition",
					depth + 1
				);
				ast_node_print_indented(
					node->as.conditional.bodies.values[i],
					"then",
					depth + 1
				);
			}
			if (NULL != node->as.conditional.elseBody)
			{
				ast_node_print_indented(
					node->as.conditional.elseBody,
					"else",
					depth + 1
				);
			}
			break;
		}

		case AST_WHILE:
		case AST_DO_WHILE:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.loop.condition,
				"condition",
				depth + 1
			);
			ast_node_print_indented(node->as.loop.body, "body", depth + 1);
			break;
		}

		case AST_FOR:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.forLoop.initializer,
				"init",
				depth + 1
			);
			ast_node_print_indented(
				node->as.forLoop.condition,
				"condition",
				depth + 1
			);
			ast_node_print_indented(
				node->as.forLoop.update,
				"update",
				depth + 1
			);
			ast_node_print_indented(node->as.forLoop.body, "body", depth + 1);
			break;
		}

		case AST_RETURN:
		{
			printf("\n");
			if (NULL != node->as.ret.value)
			{
				ast_node_print_indented(
					node->as.ret.value,
					"value",
					depth + 1
				);
			}
			break;
		}

		case AST_EXPR_STATEMENT:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.exprStatement.expression,
				NULL,
				depth + 1
			);
			break;
		}

		case AST_BREAK: /* NOLINT(*-branch-clone) */
		case AST_CONTINUE:
		{
			printf("\n");
			break;
		}

		default:
		{
			printf("\n");
			break;
		}
	}
}


void ast_node_print(const ast_node_t* node)
{
	ast_node_print_indented(node, NULL, 0);
}
#endif /* TULA_EXE_DEBUG */
