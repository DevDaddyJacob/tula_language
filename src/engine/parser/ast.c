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

/**
 * TBD
 */
/* static void example(); */


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

		case AST_PROGRAM:
		{
			arr_ast_node_destroy(&node->as.program.statements);
			break;
		}

		case AST_BLOCK:
		{
			arr_ast_node_destroy(&node->as.block.statements);
			break;
		}

		case AST_STATEMENT:
		{
			if (NULL != node->as.statement.statement)
			{
				ast_node_destroy(node->as.statement.statement);
			}
			break;
		}

		case AST_STMT_RETURN:
		{
			if (NULL != node->as.returnStmt.value)
			{
				ast_node_destroy(node->as.returnStmt.value);
			}

			break;
		}

		case AST_STMT_NUM_ITER:
		{
			if (NULL != node->as.numericIteration.initialization)
			{
				ast_node_destroy(node->as.numericIteration.initialization);
			}

			if (NULL != node->as.numericIteration.condition)
			{
				ast_node_destroy(node->as.numericIteration.condition);
			}

			if (NULL != node->as.numericIteration.block)
			{
				ast_node_destroy(node->as.numericIteration.block);
			}

			break;
		}

		case AST_STMT_COND_ITER:
		{
			if (NULL != node->as.conditionalIteration.condition)
			{
				ast_node_destroy(node->as.conditionalIteration.condition);
			}

			if (NULL != node->as.conditionalIteration.block)
			{
				ast_node_destroy(node->as.conditionalIteration.block);
			}

			break;
		}

		case AST_STMT_COMP:
		{
			if (NULL != node->as.comparison.condition)
			{
				ast_node_destroy(node->as.comparison.condition);
			}

			if (NULL != node->as.comparison.block)
			{
				ast_node_destroy(node->as.comparison.block);
			}

			if (NULL != node->as.comparison.elseNode)
			{
				ast_node_destroy(node->as.comparison.elseNode);
			}

			break;
		}

		case AST_STMT_FUNC_CALL:
		{
			if (NULL != node->as.functionCall.callee)
			{
				ast_node_destroy(node->as.functionCall.callee);
			}

			arr_ast_node_destroy(&node->as.functionCall.elseNode);

			break;
		}

		case AST_STMT_CONST_DEF:
		{
			if (NULL != node->as.constantDef.identifier)
			{
				ast_node_destroy(node->as.constantDef.identifier);
			}

			if (NULL != node->as.constantDef.expression)
			{
				ast_node_destroy(node->as.constantDef.expression);
			}

			break;
		}

		case AST_STMT_VAR_DEF:
		{
			if (NULL != node->as.variableDef.identifier)
			{
				ast_node_destroy(node->as.variableDef.identifier);
			}

			if (NULL != node->as.variableDef.expression)
			{
				ast_node_destroy(node->as.variableDef.expression);
			}

			break;
		}

		case AST_STMT_VAR_SET:
		{
			if (NULL != node->as.variableSet.identifier)
			{
				ast_node_destroy(node->as.variableSet.identifier);
			}

			if (NULL != node->as.variableSet.expression)
			{
				ast_node_destroy(node->as.variableSet.expression);
			}

			break;
		}

		case AST_STMT_VAR_UNSET:
		{
			if (NULL != node->as.variableUnset.identifier)
			{
				ast_node_destroy(node->as.variableUnset.identifier);
			}

			break;
		}

		case AST_STMT_IS_SET:
		{
			if (NULL != node->as.isSet.identifier)
			{
				ast_node_destroy(node->as.isSet.identifier);
			}

			break;
		}

		case AST_CONDITION:
		{
			if (NULL != node->as.condition.expression)
			{
				ast_node_destroy(node->as.condition.expression);
			}

			break;
		}

		case AST_EXPR_IDENT:
		{
			if (NULL != node->as.expressionIdentifier.identifier)
			{
				ast_node_destroy(node->as.expressionIdentifier.identifier);
			}

			arr_ast_node_destroy(&node->as.expressionIdentifier.accessors);
			break;
		}

		case AST_EXPR:
		{
			if (NULL != node->as.expression.lhs)
			{
				ast_node_destroy(node->as.expression.lhs);
			}

			break;
		}

		case AST_EXPR_LOGI_OR:
		{
			if (NULL != node->as.expressionLogicalOr.lhs)
			{
				ast_node_destroy(node->as.expressionLogicalOr.lhs);
			}

			if (NULL != node->as.expressionLogicalOr.rhs)
			{
				ast_node_destroy(node->as.expressionLogicalOr.rhs);
			}

			break;
		}

		case AST_EXPR_LOGI_AND:
		{
			if (NULL != node->as.expressionLogicalAnd.lhs)
			{
				ast_node_destroy(node->as.expressionLogicalAnd.lhs);
			}

			if (NULL != node->as.expressionLogicalAnd.rhs)
			{
				ast_node_destroy(node->as.expressionLogicalAnd.rhs);
			}

			break;
		}

		case AST_EXPR_EQU:
		{
			if (NULL != node->as.expressionEquality.lhs)
			{
				ast_node_destroy(node->as.expressionEquality.lhs);
			}

			if (NULL != node->as.expressionEquality.rhs)
			{
				ast_node_destroy(node->as.expressionEquality.rhs);
			}

			break;
		}

		case AST_EXPR_COMP:
		{
			if (NULL != node->as.expressionComparison.lhs)
			{
				ast_node_destroy(node->as.expressionComparison.lhs);
			}

			if (NULL != node->as.expressionComparison.rhs)
			{
				ast_node_destroy(node->as.expressionComparison.rhs);
			}

			break;
		}

		case AST_EXPR_ADD:
		{
			if (NULL != node->as.expressionAdditive.lhs)
			{
				ast_node_destroy(node->as.expressionAdditive.lhs);
			}

			if (NULL != node->as.expressionAdditive.rhs)
			{
				ast_node_destroy(node->as.expressionAdditive.rhs);
			}

			break;
		}

		case AST_EXPR_MULT:
		{
			if (NULL != node->as.expressionMultiplicative.lhs)
			{
				ast_node_destroy(node->as.expressionMultiplicative.lhs);
			}

			if (NULL != node->as.expressionMultiplicative.rhs)
			{
				ast_node_destroy(node->as.expressionMultiplicative.rhs);
			}

			break;
		}

		case AST_EXPR_EXPO:
		{
			if (NULL != node->as.expressionExponent.lhs)
			{
				ast_node_destroy(node->as.expressionExponent.lhs);
			}

			if (NULL != node->as.expressionExponent.rhs)
			{
				ast_node_destroy(node->as.expressionExponent.rhs);
			}

			break;
		}

		case AST_EXPR_UNARY:
		{
			if (NULL != node->as.expressionUnary.rhs)
			{
				ast_node_destroy(node->as.expressionUnary.rhs);
			}

			break;
		}

		case AST_EXPR_POSTFIX:
		{
			if (NULL != node->as.expressionPostfix.lhs)
			{
				ast_node_destroy(node->as.expressionPostfix.lhs);
			}

			arr_ast_node_destroy(&node->as.expressionPostfix.accessors);

			break;
		}

		case AST_EXPR_PRIMARY:
		{
			if (NULL != node->as.expressionPrimary.expression)
			{
				ast_node_destroy(node->as.expressionPrimary.expression);
			}

			break;
		}

		case AST_FUNC_DEF:
		{
			if (NULL != node->as.functionDef.body)
			{
				ast_node_destroy(node->as.functionDef.body);
			}

			if (NULL != node->as.functionDef.name)
			{
				free(node->as.functionDef.name);
			}

			break;
		}

		case AST_ACCESSOR_MEMBER:
		{
			if (NULL != node->as.accessorMember.identifier)
			{
				ast_node_destroy(node->as.accessorMember.identifier);
			}

			break;
		}

		case AST_ACCESSOR_INDEX:
		{
			if (NULL != node->as.accessorIndex.expression)
			{
				ast_node_destroy(node->as.accessorIndex.expression);
			}

			break;
		}

		case AST_ACCESSOR_CALL:
		{
			arr_ast_node_destroy(&node->as.accessorCall.expressions);
			break;
		}

		case AST_TABLE_DEF:
		{
			arr_ast_node_destroy(&node->as.tableDef.fields);
			break;
		}

		case AST_TABLE_FIELD:
		{
			if (NULL != node->as.tableField.key)
			{
				ast_node_destroy(node->as.tableField.key);
			}

			if (NULL != node->as.tableField.value)
			{
				ast_node_destroy(node->as.tableField.value);
			}

			break;
		}

		case AST_ARRAY_DEF:
		{
			arr_ast_node_destroy(&node->as.arrayDef.elements);
			break;
		}

		case AST_LITERAL:
		{
			if (TOK_STRING == node->as.literal.type)
			{
				if (NULL != node->as.literal.value.t_string)
				{
					free(node->as.literal.value.t_string);
				}
			}

			break;
		}

		case AST_IDENTIFIER:
		{
			if (NULL != node->as.identifier.name)
			{
				free(node->as.identifier.name);
			}

			break;
		}

		DEFAULT_BREAK
	}

	free(node);
}


void arr_ast_node_init(arr_ast_node_t* array)
{
	if (NULL == array)
	{
		return;
	}

	array->count = 0;
	array->capacity = 0;
	array->values = NULL;
}


void arr_ast_node_destroy(arr_ast_node_t* array)
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


void arr_ast_node_add(arr_ast_node_t* array, ast_node_t* value)
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
		case AST_STMT_BREAK:
		case AST_STMT_CONTINUE:
		{
			printf("\n");
			break;
		}

		case AST_ERROR:
		{
			printf(" message=\"%s\"\n", node->as.error.message);
			break;
		}

		case AST_PROGRAM:
		{
			printf(" count=%lld\n", node->as.program.statements.count);

			for (size_t i = 0; i < node->as.program.statements.count; i++)
			{
				ast_node_print_indented(
					node->as.program.statements.values[i],
					NULL,
					depth + 1
				);
			}

			break;
		}

		case AST_BLOCK:
		{
			printf(" count=%lld\n", node->as.block.statements.count);

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

		case AST_STATEMENT:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.statement.statement,
				NULL,
				depth + 1
			);

			break;
		}

		case AST_LITERAL:
		{
			printf(" type=%s ", TOKENS_TYPE_VALUE[node->as.literal.type]);

			switch (node->as.literal.type)
			{
				case TOK_CHAR:
				{
					printf(
						"value='%c'\n",
						node->as.literal.value.t_char
					);
					break;
				}

				case TOK_INT8:
				{
					printf(
						"value=%i\n",
						node->as.literal.value.t_int8
					);
					break;
				}

				case TOK_UINT8:
				{
					printf(
						"value=%u\n",
						node->as.literal.value.t_uint8
					);
					break;
				}

				case TOK_INT16:
				{
					printf(
						"value=%i\n",
						node->as.literal.value.t_int16
					);
					break;
				}

				case TOK_UINT16:
				{
					printf(
						"value=%u\n",
						node->as.literal.value.t_uint16
					);
					break;
				}

				case TOK_INT32:
				{
					printf(
						"value=%i\n",
						node->as.literal.value.t_int32
					);
					break;
				}

				case TOK_UINT32:
				{
					printf(
						"value=%u\n",
						node->as.literal.value.t_uint32
					);
					break;
				}

				case TOK_INT64:
				{
					printf(
						"value=%lli\n",
						node->as.literal.value.t_int64
					);
					break;
				}

				case TOK_UINT64:
				{
					printf(
						"value=%llu\n",
						node->as.literal.value.t_uint64
					);
					break;
				}

				case TOK_FLOAT:
				{
					printf(
						"value=%f\n",
						node->as.literal.value.t_float
					);
					break;
				}

				case TOK_DOUBLE:
				{
					printf(
						"value=%lf\n",
						node->as.literal.value.t_double
					);
					break;
				}

				case TOK_STRING:
				{
					printf(
						"value=\"%s\"\n",
						node->as.literal.value.t_string
					);
					break;
				}

				case TOK_TRUE:
				{
					printf("value=true\n");
					break;
				}

				case TOK_FALSE:
				{
					printf("value=false\n");
					break;
				}

				UNREACHABLE_DEFAULT();
			}

			break;
		}

		case AST_IDENTIFIER:
		{
			printf(" name=\"%s\"\n", node->as.identifier.name);
			break;
		}

		case AST_STMT_RETURN:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.returnStmt.value,
				"value",
				depth + 1
			);
			break;
		}

		case AST_STMT_NUM_ITER:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.numericIteration.initialization,
				"init",
				depth + 1
			);
			ast_node_print_indented(
				node->as.numericIteration.condition,
				"condition",
				depth + 1
			);
			ast_node_print_indented(
				node->as.numericIteration.block,
				"block",
				depth + 1
			);
			break;
		}

		case AST_STMT_COND_ITER:
		{
			printf(
				" do=%s\n",
				node->as.conditionalIteration.doMode ? "true" : "false"
			);
			ast_node_print_indented(
				node->as.conditionalIteration.condition,
				"condition",
				depth + 1
			);
			ast_node_print_indented(
				node->as.conditionalIteration.block,
				"block",
				depth + 1
			);
			break;
		}

		case AST_STMT_COMP:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.comparison.condition,
				"condition",
				depth + 1
			);
			ast_node_print_indented(
				node->as.comparison.block,
				"block",
				depth + 1
			);
			ast_node_print_indented(
				node->as.comparison.elseNode,
				"else",
				depth + 1
			);
			break;
		}

		case AST_STMT_FUNC_CALL:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.functionCall.callee,
				"callee",
				depth + 1
			);

			for (size_t i = 0; i < node->as.functionCall.elseNode.count; i++)
			{
				ast_node_print_indented(
					node->as.functionCall.elseNode.values[i],
					NULL,
					depth + 1
				);
			}

			break;
		}

		case AST_STMT_CONST_DEF:
		{
			printf(
				" global=%s\n",
				node->as.constantDef.isGlobal ? "true" : "false"
			);
			ast_node_print_indented(
				node->as.constantDef.identifier,
				"identifier",
				depth + 1
			);
			ast_node_print_indented(
				node->as.constantDef.expression,
				"expression",
				depth + 1
			);
			break;
		}

		case AST_STMT_VAR_DEF:
		{
			printf(
				" global=%s\n",
				node->as.variableDef.isGlobal ? "true" : "false"
			);
			ast_node_print_indented(
				node->as.variableDef.identifier,
				"identifier",
				depth + 1
			);
			ast_node_print_indented(
				node->as.variableDef.expression,
				"expression",
				depth + 1
			);
			break;
		}

		case AST_STMT_VAR_SET:
		{
			printf(
				" global=%s\n",
				node->as.variableSet.isGlobal ? "true" : "false"
			);
			ast_node_print_indented(
				node->as.variableSet.identifier,
				"identifier",
				depth + 1
			);
			ast_node_print_indented(
				node->as.variableSet.expression,
				"expression",
				depth + 1
			);
			break;
		}

		case AST_STMT_VAR_UNSET:
		{
			printf(
				" global=%s\n",
				node->as.variableUnset.isGlobal ? "true" : "false"
			);
			ast_node_print_indented(
				node->as.variableUnset.identifier,
				"identifier",
				depth + 1
			);
			break;
		}

		case AST_STMT_IS_SET:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.isSet.identifier,
				"identifier",
				depth + 1
			);
			break;
		}

		case AST_CONDITION:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.condition.expression,
				"expression",
				depth + 1
			);
			break;
		}

		case AST_EXPR_IDENT:
		{
			printf(
				" accessor_count=%lld\n",
				node->as.expressionIdentifier.accessors.count
			);
			ast_node_print_indented(
				node->as.expressionIdentifier.identifier,
				"identifier",
				depth + 1
			);

			for (
				size_t i = 0;
				i < node->as.expressionIdentifier.accessors.count;
				i++
			)
			{
				ast_node_print_indented(
					node->as.expressionIdentifier.accessors.values[i],
					"accessor",
					depth + 1
				);
			}

			break;
		}

		case AST_EXPR:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.expression.lhs,
				"lhs",
				depth + 1
			);
			break;
		}

		case AST_EXPR_LOGI_OR:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.expressionLogicalOr.lhs,
				"lhs",
				depth + 1
			);
			ast_node_print_indented(
				node->as.expressionLogicalOr.rhs,
				"rhs",
				depth + 1
			);
			break;
		}

		case AST_EXPR_LOGI_AND:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.expressionLogicalAnd.lhs,
				"lhs",
				depth + 1
			);
			ast_node_print_indented(
				node->as.expressionLogicalAnd.rhs,
				"rhs",
				depth + 1
			);
			break;
		}

		case AST_EXPR_EQU:
		{
			printf(
				" op=%s\n",
				TOKENS_TYPE_VALUE[node->as.expressionEquality.op]
			);
			ast_node_print_indented(
				node->as.expressionEquality.lhs,
				"lhs",
				depth + 1
			);
			ast_node_print_indented(
				node->as.expressionEquality.rhs,
				"rhs",
				depth + 1
			);
			break;
		}

		case AST_EXPR_COMP:
		{
			printf(
				" op=%s\n",
				TOKENS_TYPE_VALUE[node->as.expressionComparison.op]
			);
			ast_node_print_indented(
				node->as.expressionComparison.lhs,
				"lhs",
				depth + 1
			);
			ast_node_print_indented(
				node->as.expressionComparison.rhs,
				"rhs",
				depth + 1
			);
			break;
		}

		case AST_EXPR_ADD:
		{
			printf(
				" op=%s\n",
				TOKENS_TYPE_VALUE[node->as.expressionAdditive.op]
			);
			ast_node_print_indented(
				node->as.expressionAdditive.lhs,
				"lhs",
				depth + 1
			);
			ast_node_print_indented(
				node->as.expressionAdditive.rhs,
				"rhs",
				depth + 1
			);
			break;
		}

		case AST_EXPR_MULT:
		{
			printf(
				" op=%s\n",
				TOKENS_TYPE_VALUE[node->as.expressionMultiplicative.op]
			);
			ast_node_print_indented(
				node->as.expressionMultiplicative.lhs,
				"lhs",
				depth + 1
			);
			ast_node_print_indented(
				node->as.expressionMultiplicative.rhs,
				"rhs",
				depth + 1
			);
			break;
		}

		case AST_EXPR_EXPO:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.expressionExponent.lhs,
				"lhs",
				depth + 1
			);
			ast_node_print_indented(
				node->as.expressionExponent.rhs,
				"rhs",
				depth + 1
			);
			break;
		}

		case AST_EXPR_UNARY:
		{
			printf(
				" op=%s\n",
				TOKENS_TYPE_VALUE[node->as.expressionUnary.op]
			);
			ast_node_print_indented(
				node->as.expressionUnary.rhs,
				"rhs",
				depth + 1
			);
			break;
		}

		case AST_EXPR_POSTFIX:
		{
			printf(
				" op=%s\n",
				TOKENS_TYPE_VALUE[node->as.expressionPostfix.op]
			);
			ast_node_print_indented(
				node->as.expressionPostfix.lhs,
				"lhs",
				depth + 1
			);

			for (
				size_t i = 0;
				i < node->as.expressionPostfix.accessors.count;
				i++
			)
			{
				ast_node_print_indented(
					node->as.expressionPostfix.accessors.values[i],
					"accessor",
					depth + 1
				);
			}

			break;
		}

		case AST_EXPR_PRIMARY:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.expressionPrimary.expression,
				"expression",
				depth + 1
			);
			break;
		}

		case AST_FUNC_DEF:
		{
			printf(
				" global=%s name=\"%s\"\n",
				node->as.functionDef.isGlobal ? "true" : "false",
				NULL == node->as.functionDef.name
					? "<anonymous>"
					: node->as.functionDef.name
			);
			ast_node_print_indented(
				node->as.functionDef.body,
				"body",
				depth + 1
			);
			break;
		}

		case AST_FUNC_DEF_VAL:
		{
			printf(
				" params=%lld\n",
				node->as.functionDefValue.parameters.count
			);

			for (
				size_t i = 0;
				i < node->as.functionDefValue.parameters.count;
				i++
			)
			{
				ast_node_print_indented(
					node->as.functionDefValue.parameters.values[i],
					"param",
					depth + 1
				);
			}

			ast_node_print_indented(
				node->as.functionDefValue.body,
				"body",
				depth + 1
			);
			break;
		}

		case AST_ACCESSOR_MEMBER:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.accessorMember.identifier,
				"identifier",
				depth + 1
			);
			break;
		}

		case AST_ACCESSOR_INDEX:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.accessorIndex.expression,
				"expression",
				depth + 1
			);
			break;
		}

		case AST_ACCESSOR_CALL:
		{
			printf("\n");
			for (size_t i = 0; i < node->as.accessorCall.expressions.count; i++)
			{
				ast_node_print_indented(
					node->as.accessorCall.expressions.values[i],
					NULL,
					depth + 1
				);
			}
			break;
		}

		case AST_TABLE_DEF:
		{
			printf(" count=%lld\n", node->as.tableDef.fields.count);

			for (size_t i = 0; i < node->as.tableDef.fields.count; i++)
			{
				ast_node_print_indented(
					node->as.tableDef.fields.values[i],
					NULL,
					depth + 1
				);
			}

			break;
		}

		case AST_TABLE_FIELD:
		{
			printf("\n");
			ast_node_print_indented(
				node->as.tableField.key,
				"key",
				depth + 1
			);
			ast_node_print_indented(
				node->as.tableField.value,
				"value",
				depth + 1
			);
			break;
		}

		case AST_ARRAY_DEF:
		{
			printf(" count=%lld\n", node->as.arrayDef.elements.count);

			for (size_t i = 0; i < node->as.arrayDef.elements.count; i++)
			{
				ast_node_print_indented(
					node->as.arrayDef.elements.values[i],
					"element",
					depth + 1
				);
			}

			break;
		}

		UNREACHABLE_DEFAULT();
	}
}


void ast_node_print(const ast_node_t* node)
{
	ast_node_print_indented(node, NULL, 0);
}
#endif /* TULA_EXE_DEBUG */
