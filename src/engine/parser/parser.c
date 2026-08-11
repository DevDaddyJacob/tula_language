#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "common/exit.h"
#include "common/numeric.h"
#include "common/strings.h"

/*
 * ==================================================
 * Macros
 * ==================================================
*/

#define IS_AST_ERR(node) (NULL != (node) && AST_ERROR == (node)->type)


#define parser_err_unimplemented(parser) \
	parser_make_error( \
		(parser), \
		"Branch Unimplemented: " __FILE__ ":" TO_STRING(__LINE__) \
	)


#define parser_err_internal(parser, msg) \
	parser_make_error((parser), "Internal Parser Failure: " msg)


#define parser_err_syntax(parser, msg) \
	parser_make_error((parser), "Syntax Error: " msg)


#define parser_err_numeric_range(parser, msg) \
	parser_make_error((parser), "Numeric Range Error: " msg)


/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

/* Token cursor helpers */

/**
 * \brief           Advances the cursor by pulling the next token from the
 *                  scanner into the parser's current slot
 * \param[in]       parser: The parser whose cursor to advance
 */
static void parser_advance(parser_t* parser);


/**
 * \brief           Tests whether the current token is of the given type
 * \param[in]       parser: The parser to inspect
 * \param[in]       type: The token type to test against
 * \return          Returns true when a current token exists and its type
 *                  matches \p type
 */
static bool parser_check(const parser_t* parser, token_type_t type);


/**
 * \brief           Consumes the current token if it matches the given type
 * \param[in]       parser: The parser to advance on a match
 * \param[in]       type: The token type to test against
 * \return          Returns true and advances the cursor on a match, otherwise
 *                  returns false and leaves the cursor untouched
 */
static bool parser_check_and_advance(parser_t* parser, token_type_t type);


/**
 * \brief           Tests whether the token stream has been exhausted
 * \param[in]       parser: The parser to inspect
 * \return          Returns true at the end-of-stream token, on a scanner error,
 *                  or when no current token exists
 */
static bool parser_is_at_end(const parser_t* parser);


/**
 * \brief           Duplicates the current token's content into a fresh heap
 *                  string
 * \param[in]       parser: The parser whose current token to copy
 * \return          Returns a newly allocated, null-terminated copy of the
 *                  content, or an empty string when the token has no content
 */
static char* parser_dup_current(const parser_t* parser);


/**
 * \brief           Builds an AST_ERROR node for the current position and flags
 *                  the parser as having errored
 * \param[in]       parser: The parser to flag; its current token supplies the
 *                  reported position
 * \param[in]       message: The fallback error message, used unless the current
 *                  token is itself a scanner error carrying its own message
 * \return          Returns a newly allocated AST_ERROR node
 */
static ast_node_t* parser_make_error(parser_t* parser, const char* message);


static bool ast_program(parser_t* parser, ast_node_t** out);


static bool ast_block(parser_t* parser, ast_node_t** out);


static bool ast_statement(parser_t* parser, ast_node_t** out);


static bool ast_stmt_return(parser_t* parser, ast_node_t** out);


static bool ast_stmt_num_iter(parser_t* parser, ast_node_t** out);


static bool ast_stmt_cond_iter(parser_t* parser, ast_node_t** out);


static bool ast_stmt_comp(parser_t* parser, ast_node_t** out);


static bool ast_stmt_func_call(parser_t* parser, ast_node_t** out);


static bool ast_stmt_var_const_func_def(parser_t* parser, ast_node_t** out);


static bool ast_stmt_const_def_internal(
	parser_t* parser,
	ast_node_t** out,
	uint32_t line,
	uint32_t column,
	bool global
);


static bool ast_stmt_var_def_internal(
	parser_t* parser,
	ast_node_t** out,
	uint32_t line,
	uint32_t column,
	bool global
);


static bool ast_stmt_var_set(parser_t* parser, ast_node_t** out);


static bool ast_stmt_var_unset(parser_t* parser, ast_node_t** out);


static bool ast_stmt_is_set(parser_t* parser, ast_node_t** out);


static bool ast_stmt_break(parser_t* parser, ast_node_t** out);


static bool ast_stmt_continue(parser_t* parser, ast_node_t** out);


static bool ast_condition(parser_t* parser, ast_node_t** out);


static bool ast_expr_ident(parser_t* parser, ast_node_t** out);


static bool ast_expr(parser_t* parser, ast_node_t** out);


static bool ast_expr_logi_or(parser_t* parser, ast_node_t** out);


static bool ast_expr_logi_and(parser_t* parser, ast_node_t** out);


static bool ast_expr_equ(parser_t* parser, ast_node_t** out);


static bool ast_expr_comp(parser_t* parser, ast_node_t** out);


static bool ast_expr_add(parser_t* parser, ast_node_t** out);


static bool ast_expr_mult(parser_t* parser, ast_node_t** out);


static bool ast_expr_expo(parser_t* parser, ast_node_t** out);


static bool ast_expr_unary(parser_t* parser, ast_node_t** out);


static bool ast_expr_postfix(parser_t* parser, ast_node_t** out);


static bool ast_expr_primary(parser_t* parser, ast_node_t** out);


static bool ast_func_def_internal(
	parser_t* parser,
	ast_node_t** out,
	uint32_t line,
	uint32_t column,
	bool global
);


static bool ast_func_def_value(parser_t* parser, ast_node_t** out);


static bool ast_accessor_member(parser_t* parser, ast_node_t** out);


static bool ast_accessor_index(parser_t* parser, ast_node_t** out);


static bool ast_accessor_call(parser_t* parser, ast_node_t** out);


static bool ast_table_def(parser_t* parser, ast_node_t** out);


static bool ast_table_field(parser_t* parser, ast_node_t** out);


static bool ast_array_def(parser_t* parser, ast_node_t** out);


static bool ast_literal(parser_t* parser, ast_node_t** out);


static bool ast_identifier(parser_t* parser, ast_node_t** out);



/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
 */


/*
 * ==================================================
 * Function Definitions
 * ==================================================
 */

static void parser_advance(parser_t* parser)
{
	parser->current = scanner_read_next(parser->scanner);
}


static bool parser_check(const parser_t* parser, const token_type_t type)
{
	return NULL != parser->current && type == parser->current->type;
}


static bool parser_check_and_advance(parser_t* parser, const token_type_t type)
{
	if (!parser_check(parser, type))
	{
		return false;
	}

	parser_advance(parser);
	return true;
}


static bool parser_is_at_end(const parser_t* parser)
{
	return NULL == parser->current
		|| TOK_EOS == parser->current->type
		|| TOK_ERROR == parser->current->type;
}


static bool parser_starts_expression(const token_type_t type)
{
	if (TOKENS_IS_PRIMITIVE[type])
	{
		return true;
	}

	switch (type)
	{
		case TOK_IDENT:
		case TOK_IS_SET:
		case TOK_PAREN_LEFT:
		case TOK_BRACKET_LEFT:
		case TOK_BRACE_LEFT:
		case TOK_FUNC:
		case TOK_NOT:
		case TOK_PLUS_PLUS:
		case TOK_MINUS_MINUS:
		{
			return true;
		}

		DEFAULT_BREAK
	}

	return false;
}


static char* parser_dup_current(const parser_t* parser)
{
	const char* content = parser->current->content;

	/* A content-less token still duplicates to a valid empty string */
	size_t contentLength = 0;
	if (NULL != content)
	{
		contentLength = strlen(content);
	}

	const size_t size = contentLength + 1;

	char* copy = malloc(sizeof(char) * size);
	if (NULL == copy)
	{
		tula_exit_err_no_mem();
		UNREACHABLE_RETURN(NULL);
	}

	const char* source = content;
	if (NULL == source)
	{
		source = "";
	}

	str_copy_safe(copy, source, size);

	return copy;
}


static ast_node_t* parser_make_error(parser_t* parser, const char* message)
{
	parser->hadError = true;

	uint32_t line = 0;
	uint32_t column = 0;
	const char* text = message;

	if (NULL != parser->current)
	{
		line = parser->current->line;
		column = parser->current->column;

		/*
		 * When the offending token is itself a scanner error, surface the
		 * scanner's own message rather than the parser's generic one.
		 */
		if (
			TOK_ERROR == parser->current->type
			&& NULL != parser->current->content
		)
		{
			text = parser->current->content;
		}
	}

	return ast_node_new_error(line, column, text);
}


static ast_node_t* parser_make_identifier(parser_t* parser)
{
	ast_node_t* node = ast_node_new(
		AST_IDENTIFIER,
		parser->current->line,
		parser->current->column
	);

	node->as.identifier.name = parser_dup_current(parser);
	parser_advance(parser);

	return node;
}


static bool ast_program(parser_t* parser, ast_node_t** out)
{
	uint32_t line = 1;
	uint32_t column = 1;
	if (NULL != parser->current)
	{
		line = parser->current->line;
		column = parser->current->column;
	}


	/* Create / init the output node */
	*out = ast_node_new(AST_PROGRAM, line, column);
	arr_ast_node_init(&(*out)->as.program.statements);


	/* Consume statements */
	bool hadError = false;
	ast_node_t* statement = NULL;
	while (!parser_is_at_end(parser))
	{
		if (ast_statement(parser, &statement))
		{
			arr_ast_node_add(&(*out)->as.program.statements, statement);
			statement = NULL;
			continue;
		}


		if (!IS_AST_ERR(statement))
		{
			statement = parser_err_internal(
				parser,
				"program statements parse failed"
			);
		}

		arr_ast_node_add(&(*out)->as.program.statements, statement);

		hadError = true;

		break;
	}


	/* Ensure we didn't return an internal error */
	if (AST_ERROR == (*out)->type)
	{
		return false;
	}


	/* Surface a trailing scanner error as data in the tree */
	if (!parser->hadError && parser_check(parser, TOK_ERROR))
	{
		arr_ast_node_add(
			&(*out)->as.block.statements,
			parser_err_syntax(parser, "unexpected token.")
		);
	}

	return !hadError;
}


static bool ast_block(parser_t* parser, ast_node_t** out)
{
	/* Find the opening { */
	if (!parser_check(parser, TOK_BRACE_LEFT))
	{
		*out = parser_err_syntax(parser, "expected '{' while parsing block.");

		return false;
	}


	/* Create / init the output node */
	*out = ast_node_new(
		AST_BLOCK,
		parser->current->line,
		parser->current->column
	);
	arr_ast_node_init(&(*out)->as.block.statements);


	/* Consume the { token */
	parser_advance(parser);


	/* Consume statements */
	bool hadError = false;
	ast_node_t* statement = NULL;
	while (!parser_is_at_end(parser))
	{
		if (ast_statement(parser, &statement))
		{
			arr_ast_node_add(&(*out)->as.block.statements, statement);
			statement = NULL;
			continue;
		}

		/*
		 * If the statement ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR(statement))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"block statements parse failed"
			);
		}

		hadError = true;

		break;
	}


	/* Ensure we didn't return an internal error */
	if (AST_ERROR == (*out)->type)
	{
		return false;
	}


	/* Surface a trailing scanner error as data in the tree */
	if (!parser->hadError && parser_check(parser, TOK_ERROR))
	{
		arr_ast_node_add(
			&(*out)->as.block.statements,
			parser_err_syntax(parser, "unexpected token.")
		);
	}


	/* Find the closing } */
	if (!parser_check_and_advance(parser, TOK_BRACE_RIGHT))
	{
		ast_node_destroy(*out);

		*out = parser_err_syntax(parser, "expected '}' while parsing block.");

		return false;
	}

	return hadError;
}


static bool ast_statement(parser_t* parser, ast_node_t** out)
{
	/* TODO: Add from EBNF "function_call_statement" */
	ast_node_t* statement = NULL;
	switch (parser->current->type)
	{
		case TOK_BREAK:
		{
			*out = ast_node_new(
				AST_STMT_BREAK,
				parser->current->line,
				parser->current->column
			);

			/* consume the break token */
			parser_advance(parser);

			return true;
		}

		case TOK_CONTINUE:
		{
			*out = ast_node_new(
				AST_STMT_CONTINUE,
				parser->current->line,
				parser->current->column
			);

			/* consume the continue token */
			parser_advance(parser);

			return true;
		}

		case TOK_IS_SET:
		{
			if (ast_stmt_is_set(parser, &statement))
			{
				goto fold_and_exit;
			}

			goto fold_failure_and_exit;;
		}

		case TOK_DEFINE:
		{
			if (
				ast_stmt_var_const_func_def(
					parser,
					&statement
				)
			)
			{
				goto fold_and_exit;
			}

			goto fold_failure_and_exit;;
		}

		case TOK_SET:
		{
			if (ast_stmt_var_set(parser, &statement))
			{
				goto fold_and_exit;
			}

			goto fold_failure_and_exit;;
		}

		case TOK_UNSET:
		{
			if (ast_stmt_var_unset(parser, &statement))
			{
				goto fold_and_exit;
			}

			goto fold_failure_and_exit;;
		}

		case TOK_IF:
		{
			if (ast_stmt_comp(parser, &statement))
			{
				goto fold_and_exit;
			}

			goto fold_failure_and_exit;;
		}

		case TOK_DO:
		case TOK_WHILE:
		{
			if (ast_stmt_cond_iter(parser, &statement))
			{
				goto fold_and_exit;
			}

			goto fold_failure_and_exit;;
		}

		case TOK_FOR:
		{
			if (ast_stmt_num_iter(parser, &statement))
			{
				goto fold_and_exit;
			}

			goto fold_failure_and_exit;;
		}

		case TOK_RETURN:
		{
			if (ast_stmt_return(parser, &statement))
			{
				goto fold_and_exit;
			}

			goto fold_failure_and_exit;;
		}

		default:
		{
			*out = parser_err_syntax(
				parser,
				"unexpected token."
			);

			return false;
		}
	}

fold_and_exit:
	*out = statement;
	return true;


fold_failure_and_exit:
	if (!IS_AST_ERR(statement))
	{
		ast_node_destroy(*out);
		*out = parser_err_internal(parser, "statement parse failed");
	}
	else
	{
		*out = statement;
	}

	return false;
}


static bool ast_stmt_return(parser_t* parser, ast_node_t** out)
{
	/* Find the opening return */
	if (!parser_check(parser, TOK_RETURN))
	{
		*out =  parser_err_internal(
			parser,
			"expected 'return' while parsing return statement"
		);

		return false;
	}


	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;
	*out = ast_node_new(AST_STMT_RETURN, line, column);


	/* consume 'return' */
	parser_advance(parser);


	/* Try to parse an expression */
	if (!ast_expr(parser, &(*out)->as.returnStmt.value))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR((*out)->as.returnStmt.value))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"return statement expression parse failed"
			);

			return false;
		}

		(*out)->as.returnStmt.value = NULL;
	}

	return true;
}


static bool ast_stmt_num_iter(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_stmt_cond_iter(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_stmt_comp(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_stmt_func_call(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_stmt_var_const_func_def(parser_t* parser, ast_node_t** out)
{
	/* Find the opening define */
	if (!parser_check(parser, TOK_DEFINE))
	{
		*out = parser_err_internal(
			parser,
			"expected 'define' while parsing "
				"variable/constant/function definition statement"
		);

		return false;
	}


	/* Store the statement line and column at the start */
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;


	/* consume 'define' */
	parser_advance(parser);


	/* Determine if this is a global or not */
	const bool isGlobal = parser_check_and_advance(parser, TOK_GLOBAL);


	/* Check if this is a constant */
	if (parser_check_and_advance(parser, TOK_CONSTANT))
	{
		return ast_stmt_const_def_internal(parser, out, line, column, isGlobal);
	}


	/* Check if this is a variable */
	if (parser_check_and_advance(parser, TOK_VARIABLE))
	{
		return ast_stmt_var_def_internal(parser, out, line, column, isGlobal);
	}


	/* Check if this is a function */
	if (parser_check_and_advance(parser, TOK_FUNC))
	{
		return ast_func_def_internal(parser, out, line, column, isGlobal);
	}


	*out = parser_err_syntax(parser, "unexpected token.");
	return false;
}


static bool ast_stmt_const_def_internal(
	parser_t* parser,
	ast_node_t** out,
	const uint32_t line,
	const uint32_t column,
	const bool global
)
{
	*out = ast_node_new(AST_STMT_CONST_DEF, line, column);
	(*out)->as.constantDef.isGlobal = global;


	/* Get the identifier */
	if (!ast_identifier(parser, &(*out)->as.constantDef.identifier))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR((*out)->as.constantDef.identifier))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"constant definition statement identifier parse failed"
			);
		}

		return false;
	}


	/* Expect a = */
	if (!parser_check_and_advance(parser, TOK_EQUAL))
	{
		(*out)->as.constantDef.expression = parser_err_syntax(
			parser,
			"expected '=' while parsing constant definition."
		);

		return false;
	}


	/* Get the expression */
	if (!ast_expr(parser, &(*out)->as.constantDef.expression))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR((*out)->as.constantDef.expression))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"constant definition statement expression parse failed"
			);
		}

		return false;
	}

	return true;
}


static bool ast_stmt_var_def_internal(
	parser_t* parser,
	ast_node_t** out,
	const uint32_t line,
	const uint32_t column,
	const bool global
)
{
	*out = ast_node_new(AST_STMT_VAR_DEF, line, column);
	(*out)->as.variableDef.isGlobal = global;


	/* Get the identifier */
	if (!ast_identifier(parser, &(*out)->as.variableDef.identifier))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR((*out)->as.variableDef.identifier))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"constant definition statement identifier parse failed"
			);
		}

		return false;
	}


	/* Check for a = */
	if (!parser_check(parser, TOK_EQUAL))
	{
		(*out)->as.variableDef.expression = NULL;
		return true;
	}


	/* consume '=' */
	parser_advance(parser);


	/* Get the expression */
	if (!ast_expr(parser, &(*out)->as.variableDef.expression))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR((*out)->as.variableDef.expression))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"constant definition statement expression parse failed"
			);
		}

		return false;
	}

	return true;
}


static bool ast_stmt_var_set(parser_t* parser, ast_node_t** out)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;

	/* Find the opening set */
	if (!parser_check_and_advance(parser, TOK_SET))
	{
		*out = parser_err_internal(
			parser,
			"expected 'set' while parsing variable set statement"
		);

		return false;
	}

	/* Check for the global keyword */
	const bool isGlobal = parser_check_and_advance(parser, TOK_GLOBAL);


	/* Find the following var */
	if (!parser_check_and_advance(parser, TOK_VARIABLE))
	{
		*out = parser_err_syntax(
			parser,
			"expected 'variable'/'var' while parsing variable set statement"
		);

		return false;
	}


	/* Make the node */
	*out = ast_node_new(AST_STMT_VAR_SET, line, column);
	(*out)->as.variableSet.isGlobal = isGlobal;


	/* Find the identifier */
	if (!ast_identifier(parser, &(*out)->as.variableSet.identifier))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR((*out)->as.variableSet.identifier))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"variable set statement identifier parse failed"
			);
		}

		return false;
	}


	/* Find the = */
	if (!parser_check_and_advance(parser, TOK_EQUAL))
	{
		ast_node_destroy(*out);

		*out = parser_err_syntax(
			parser,
			"expected '=' while parsing variable set statement"
		);

		return false;
	}


	/* Find the expression */
	if (!ast_expr(parser, &(*out)->as.variableSet.expression))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR((*out)->as.variableSet.expression))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"variable set statement expression parse failed"
			);
		}

		return false;
	}

	return true;
}


static bool ast_stmt_var_unset(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_stmt_is_set(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_stmt_break(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_stmt_continue(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_condition(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_expr_ident(parser_t* parser, ast_node_t** out)
{
    *out = ast_node_new(
        AST_EXPR_IDENT,
        parser->current->line,
        parser->current->column
    );

    arr_ast_node_init(&(*out)->as.expressionIdentifier.accessors);


    /* Handle the identifier */
    if (!ast_identifier(parser, &(*out)->as.expressionIdentifier.identifier))
    {
        /*
         * If the parsed ast is not an error but failed, this ast
         * becomes an error
         */
        if (!IS_AST_ERR((*out)->as.expressionIdentifier.identifier))
        {
            ast_node_destroy(*out);

            *out = parser_err_internal(
                parser,
                "expression identifier parse failed"
            );
        }

        return false;
    }


    /*
     * Handle any member/index accessors. A call accessor is intentionally not
     * consumed here: expression_identifier is the assignable-location form used
     * by pre/post inc-dec and isSet, and a call yields a value, not a location.
     */
    while (true)
    {
        ast_node_t* accessor = NULL;
        bool accessorParseStatus;

        if (parser_check(parser, TOK_DOT))
        {
            accessorParseStatus = ast_accessor_member(parser, &accessor);
        }
        else if (parser_check(parser, TOK_BRACKET_LEFT))
        {
            accessorParseStatus = ast_accessor_index(parser, &accessor);
        }
        else
        {
            break;
        }


        /* Handle accessor parse failure */
        if (!accessorParseStatus)
        {
            /*
             * If the parsed ast is not an error but failed, this ast
             * becomes an error
             */
			if (!IS_AST_ERR(accessor))
            {
                ast_node_destroy(*out);

                *out = parser_err_internal(
                    parser,
                    "expression identifier accessor parse failed"
                );
	        }
			else
	        {
                ast_node_destroy(*out);

                *out = accessor;
	        }

		    return false;
		}


        arr_ast_node_add(&(*out)->as.expressionIdentifier.accessors, accessor);
    }

    return true;
  }


static bool ast_expr(parser_t* parser, ast_node_t** out)
{
	*out = ast_node_new(
		AST_EXPR,
		parser->current->line,
		parser->current->column
	);


	/* Handle left hand side */
	if (!ast_expr_logi_or(parser, &(*out)->as.expression.lhs))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR((*out)->as.expression.lhs))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"expression left-hand side parse failed"
			);
		}

		return false;
	}

	return true;
}


static bool ast_expr_logi_or(parser_t* parser, ast_node_t** out)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;

	/* Handle left hand side (one tighter operand) */
	ast_node_t* lhs = NULL;
	if (!ast_expr_logi_and(parser, &lhs))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR(lhs))
		{
			ast_node_destroy(lhs);

			*out = parser_err_internal(
				parser,
				"logical or expression left-hand side parse failed"
			);
		}
		else
		{
			*out = lhs;
		}

		return false;
	}


	/* As long as we can keep going, fold left */
	while (true)
	{
		if (!parser_check_and_advance(parser, TOK_OR))
		{
			break;
		}


		/* Handle right hand side (another tighter operand) */
		ast_node_t* rhs = NULL;
		if (!ast_expr_logi_and(parser, &rhs))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR(rhs))
			{
				ast_node_destroy(lhs);
				ast_node_destroy(rhs);

				*out = parser_err_internal(
					parser,
					"logical or expression right-hand side parse failed"
				);
			}
			else
			{
				*out = rhs;
			}

			return false;
		}


		/* fold the accumulated tree */
		ast_node_t* foldedNode = ast_node_new(AST_EXPR_LOGI_OR, line, column);
		foldedNode->as.expressionLogicalOr.lhs = lhs;
		foldedNode->as.expressionLogicalOr.rhs = rhs;

		lhs = foldedNode;
	}


	*out = lhs;

	return true;
}


static bool ast_expr_logi_and(parser_t* parser, ast_node_t** out)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;

	/* Handle left hand side (one tighter operand) */
	ast_node_t* lhs = NULL;
	if (!ast_expr_equ(parser, &lhs))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR(lhs))
		{
			ast_node_destroy(lhs);

			*out = parser_err_internal(
				parser,
				"logical and expression left-hand side parse failed"
			);
		}
		else
		{
			*out = lhs;
		}

		return false;
	}


	/* As long as we can keep going, fold left */
	while (true)
	{
		if (!parser_check_and_advance(parser, TOK_AND))
		{
			break;
		}


		/* Handle right hand side (another tighter operand) */
		ast_node_t* rhs = NULL;
		if (!ast_expr_equ(parser, &rhs))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR(rhs))
			{
				ast_node_destroy(lhs);
				ast_node_destroy(rhs);

				*out = parser_err_internal(
					parser,
					"logical and expression right-hand side parse failed"
				);
			}
			else
			{
				*out = rhs;
			}

			return false;
		}


		/* fold the accumulated tree */
		ast_node_t* foldedNode = ast_node_new(AST_EXPR_LOGI_AND, line, column);
		foldedNode->as.expressionLogicalAnd.lhs = lhs;
		foldedNode->as.expressionLogicalAnd.rhs = rhs;

		lhs = foldedNode;
	}


	*out = lhs;

	return true;
}


static bool ast_expr_equ(parser_t* parser, ast_node_t** out)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;

	/* Handle left hand side (one tighter operand) */
	ast_node_t* lhs = NULL;
	if (!ast_expr_comp(parser, &lhs))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR(lhs))
		{
			ast_node_destroy(lhs);

			*out = parser_err_internal(
				parser,
				"equality expression left-hand side parse failed"
			);
		}
		else
		{
			*out = lhs;
		}

		return false;
	}


	/* As long as we can keep going, fold left */
	while (true)
	{
		token_type_t op = TOK_NONE;
		if (parser_check_and_advance(parser, TOK_EQUAL_EQUAL))
		{
			op = TOK_EQUAL_EQUAL;
		}
		else if (parser_check_and_advance(parser, TOK_EXCLAM_EQUAL))
		{
			op = TOK_EXCLAM_EQUAL;
		}
		else
		{
			break;
		}


		/* Handle right hand side (another tighter operand) */
		ast_node_t* rhs = NULL;
		if (!ast_expr_comp(parser, &rhs))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR(rhs))
			{
				ast_node_destroy(lhs);
				ast_node_destroy(rhs);

				*out = parser_err_internal(
					parser,
					"equality expression right-hand side parse failed"
				);
			}
			else
			{
				*out = rhs;
			}

			return false;
		}


		/* fold the accumulated tree */
		ast_node_t* foldedNode = ast_node_new(AST_EXPR_EQU, line, column);
		foldedNode->as.expressionEquality.lhs = lhs;
		foldedNode->as.expressionEquality.op = op;
		foldedNode->as.expressionEquality.rhs = rhs;

		lhs = foldedNode;
	}


	*out = lhs;

	return true;
}


static bool ast_expr_comp(parser_t* parser, ast_node_t** out)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;

	/* Handle left hand side (one tighter operand) */
	ast_node_t* lhs = NULL;
	if (!ast_expr_add(parser, &lhs))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR(lhs))
		{
			ast_node_destroy(lhs);

			*out = parser_err_internal(
				parser,
				"comparison expression left-hand side parse failed"
			);
		}
		else
		{
			*out = lhs;
		}

		return false;
	}


	/* As long as we can keep going, fold left */
	while (true)
	{
		token_type_t op = TOK_NONE;
		if (parser_check_and_advance(parser, TOK_GREATER_THAN))
		{
			op = TOK_GREATER_THAN;
		}
		else if (parser_check_and_advance(parser, TOK_LESS_THAN))
		{
			op = TOK_LESS_THAN;
		}
		else if (parser_check_and_advance(parser, TOK_GT_EQUAL))
		{
			op = TOK_GT_EQUAL;
		}
		else if (parser_check_and_advance(parser, TOK_LT_EQUAL))
		{
			op = TOK_LT_EQUAL;
		}
		else
		{
			break;
		}


		/* Handle right hand side (another tighter operand) */
		ast_node_t* rhs = NULL;
		if (!ast_expr_add(parser, &rhs))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR(rhs))
			{
				ast_node_destroy(lhs);
				ast_node_destroy(rhs);

				*out = parser_err_internal(
					parser,
					"comparison expression right-hand side parse failed"
				);
			}
			else
			{
				*out = rhs;
			}

			return false;
		}


		/* fold the accumulated tree */
		ast_node_t* foldedNode = ast_node_new(AST_EXPR_COMP, line, column);
		foldedNode->as.expressionComparison.lhs = lhs;
		foldedNode->as.expressionComparison.op = op;
		foldedNode->as.expressionComparison.rhs = rhs;

		lhs = foldedNode;
	}


	*out = lhs;

	return true;
}


static bool ast_expr_add(parser_t* parser, ast_node_t** out)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;

	/* Handle left hand side (one tighter operand) */
	ast_node_t* lhs = NULL;
	if (!ast_expr_mult(parser, &lhs))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR(lhs))
		{
			ast_node_destroy(lhs);

			*out = parser_err_internal(
				parser,
				"additive expression left-hand side parse failed"
			);
		}
		else
		{
			*out = lhs;
		}

		return false;
	}


	/* As long as we can keep going, fold left */
	while (true)
	{
		token_type_t op = TOK_NONE;
		if (parser_check_and_advance(parser, TOK_PLUS))
		{
			op = TOK_PLUS;
		}
		else if (parser_check_and_advance(parser, TOK_MINUS))
		{
			op = TOK_MINUS;
		}
		else
		{
			break;
		}


		/* Handle right hand side (another tighter operand) */
		ast_node_t* rhs = NULL;
		if (!ast_expr_mult(parser, &rhs))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR(rhs))
			{
				ast_node_destroy(lhs);
				ast_node_destroy(rhs);

				*out = parser_err_internal(
					parser,
					"additive expression right-hand side parse failed"
				);
			}
			else
			{
				*out = rhs;
			}

			return false;
		}


		/* fold the accumulated tree */
		ast_node_t* foldedNode = ast_node_new(AST_EXPR_ADD, line, column);
		foldedNode->as.expressionAdditive.lhs = lhs;
		foldedNode->as.expressionAdditive.op = op;
		foldedNode->as.expressionAdditive.rhs = rhs;

		lhs = foldedNode;
	}


	*out = lhs;

	return true;
}


static bool ast_expr_mult(parser_t* parser, ast_node_t** out)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;

	/* Handle left hand side (one tighter operand) */
	ast_node_t* lhs = NULL;
	if (!ast_expr_expo(parser, &lhs))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR(lhs))
		{
			ast_node_destroy(lhs);

			*out = parser_err_internal(
				parser,
				"multiplicative expression left-hand side parse failed"
			);
		}
		else
		{
			*out = lhs;
		}

		return false;
	}


	/* As long as we can keep going, fold left */
	while (true)
	{
		token_type_t op = TOK_NONE;
		if (parser_check_and_advance(parser, TOK_STAR))
		{
			op = TOK_STAR;
		}
		else if (parser_check_and_advance(parser, TOK_SLASH_FWD))
		{
			op = TOK_SLASH_FWD;
		}
		else
		{
			break;
		}


		/* Handle right hand side (another tighter operand) */
		ast_node_t* rhs = NULL;
		if (!ast_expr_expo(parser, &rhs))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR(rhs))
			{
				ast_node_destroy(lhs);
				ast_node_destroy(rhs);

				*out = parser_err_internal(
					parser,
					"multiplicative expression right-hand side parse failed"
				);
			}
			else
			{
				*out = rhs;
			}

			return false;
		}


		/* fold the accumulated tree */
		ast_node_t* foldedNode = ast_node_new(AST_EXPR_MULT, line, column);
		foldedNode->as.expressionMultiplicative.lhs = lhs;
		foldedNode->as.expressionMultiplicative.op = op;
		foldedNode->as.expressionMultiplicative.rhs = rhs;

		lhs = foldedNode;
	}


	*out = lhs;

	return true;
}


static bool ast_expr_expo(parser_t* parser, ast_node_t** out)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;

	/* Handle left hand side */
	ast_node_t* lhs = NULL;
	if (!ast_expr_unary(parser, &lhs))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR(lhs))
		{
			ast_node_destroy(lhs);

			*out = parser_err_internal(
				parser,
				"exponent expression left-hand side parse failed"
			);
		}
		else
		{
			*out = lhs;
		}

		return false;
	}

	/* As long as we can keep going, fold left */
	while (true)
	{
		if (!parser_check_and_advance(parser, TOK_CARET))
		{
			break;
		}


		/* Handle right hand side (same operand) */
		ast_node_t* rhs = NULL;
		if (!ast_expr_expo(parser, &rhs))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR(rhs))
			{
				ast_node_destroy(lhs);
				ast_node_destroy(rhs);

				*out = parser_err_internal(
					parser,
					"exponent expression right-hand side parse failed"
				);
			}
			else
			{
				*out = rhs;
			}

			return false;
		}


		/* fold the accumulated tree */
		ast_node_t* foldedNode = ast_node_new(AST_EXPR_EXPO, line, column);
		foldedNode->as.expressionExponent.lhs = lhs;
		foldedNode->as.expressionExponent.rhs = rhs;

		lhs = foldedNode;
	}


	*out = lhs;

	return true;
}


static bool ast_expr_unary(parser_t* parser, ast_node_t** out)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;

	ast_node_t* rhs = NULL;
	while (true)
	{
		token_type_t op = TOK_NONE;
		if (parser_check_and_advance(parser, TOK_NOT))
		{
			op = TOK_NOT;
		}
		else if (parser_check_and_advance(parser, TOK_PLUS_PLUS))
		{
			op = TOK_PLUS_PLUS;
		}
		else if (parser_check_and_advance(parser, TOK_MINUS_MINUS))
		{
			op = TOK_MINUS_MINUS;
		}
		else
		{
			break;
		}


		ast_node_t* nextRhs = NULL;
		bool astParseStatus = false;
		switch (op)
		{
			case TOK_NOT:
			{
				astParseStatus = ast_expr_unary(parser, &nextRhs);
				break;
			}

			case TOK_MINUS_MINUS:
			case TOK_PLUS_PLUS:
			{
				astParseStatus = ast_expr_ident(parser, &nextRhs);
				break;
			}

			UNREACHABLE_DEFAULT(break);
		}

		if (!astParseStatus)
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR(nextRhs))
			{
				ast_node_destroy(rhs);
				ast_node_destroy(nextRhs);

				*out = parser_err_internal(
					parser,
					"unary expression right-hand side parse failed"
				);
			}
			else
			{
				*out = rhs;
			}

			return false;
		}



		/* fold the accumulated tree */
		ast_node_t* foldedNode = ast_node_new(AST_EXPR_UNARY, line, column);
		foldedNode->as.expressionUnary.op = op;
		foldedNode->as.expressionUnary.rhs = nextRhs;

		rhs = foldedNode;
	}

	if (NULL == rhs)
	{
		if (!ast_expr_postfix(parser, &rhs))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR((*out)->as.expressionUnary.rhs))
			{
				ast_node_destroy(*out);

				*out = parser_err_internal(
					parser,
					"unary expression right-hand side parse failed"
				);
			}

			return false;
		}
	}


	*out = rhs;

	return true;
}


static bool ast_expr_postfix(parser_t* parser, ast_node_t** out)
{
	/* AST folding attempt (START) */
	// const uint32_t line = parser->current->line;
	// const uint32_t column = parser->current->column;
	//
	// ast_node_t* lhs = NULL;
	// while (true)
	// {
	// 	const bool startsWithIdent = parser_check(parser, TOK_IDENT);
	//
	// 	/* Handle the lhs ast */
	// 	ast_node_t* nextLhs = NULL;
	// 	bool astParseStatus = false;
	// 	if (startsWithIdent)
	// 	{
	// 		astParseStatus = ast_expr_ident(parser, &nextLhs);
	// 	}
	// 	else
	// 	{
	// 		astParseStatus = ast_expr_primary(parser, &nextLhs);
	// 	}
	//
	// 	if (!astParseStatus)
	// 	{
	// 		/*
	// 		 * If the parsed ast is not an error but failed, this ast
	// 		 * becomes an error
	// 		 */
	// 		if (!IS_AST_ERR(nextLhs))
	// 		{
	// 			ast_node_destroy(lhs);
	// 			ast_node_destroy(nextLhs);
	//
	// 			*out = parser_err_internal(
	// 				parser,
	// 				"postfix expression left-hand side parse failed"
	// 			);
	// 		}
	//
	// 		return false;
	// 	}
	//
	//
	// 	/* Detect post-increment and post-decrement */
	// 	token_type_t op = TOK_NONE;
	// 	if (parser_check(parser, TOK_PLUS_PLUS))
	// 	{
	// 		op = TOK_PLUS_PLUS;
	// 	}
	// 	else if (parser_check(parser, TOK_MINUS_MINUS))
	// 	{
	// 		op = TOK_MINUS_MINUS;
	// 	}
	//
	//
	// 	/* Handle post-increment and post-decrement */
	// 	if (TOK_NONE != op)
	// 	{
	// 		if (!startsWithIdent)
	// 		{
	// 			ast_node_destroy(lhs);
	// 			ast_node_destroy(nextLhs);
	//
	// 			*out = parser_err_syntax(
	// 				parser,
	// 				"expected variable, field, or index while parsing "
	// 					"increment/decrement expression."
	// 			);
	//
	// 			return false;
	// 		}
	//
	// 		parser_advance(parser);
	//
	//
	// 		/* fold the accumulated tree */
	// 		ast_node_t* foldedNode = ast_node_new(
	// 			AST_EXPR_POSTFIX,
	// 			line,
	// 			column
	// 		);
	// 		foldedNode->as.expressionPostfix.lhs = nextLhs;
	// 		foldedNode->as.expressionPostfix.op = op;
	// 		arr_ast_node_init(&foldedNode->as.expressionPostfix.accessors);
	//
	// 		lhs = foldedNode;
	// 		continue;
	// 	}
	//
	//
	// 	/* Handle non-post-in(de)crement */
	// 	while (true)
	// 	{
	// 		ast_node_t* accessor = NULL;
	// 		bool accessorParseStatus;
	//
	// 		if (parser_check(parser, TOK_DOT))
	// 		{
	// 			accessorParseStatus = ast_accessor_member(parser, &accessor);
	// 		}
	// 		else if (parser_check(parser, TOK_BRACKET_LEFT))
	// 		{
	// 			accessorParseStatus = ast_accessor_index(parser, &accessor);
	// 		}
	// 		else if (parser_check(parser, TOK_PAREN_LEFT))
	// 		{
	// 			accessorParseStatus = ast_accessor_call(parser, &accessor);
	// 		}
	// 		else
	// 		{
	// 			break;
	// 		}
	//
	//
	// 		/* Handle accessor parse failure */
	// 		if (!accessorParseStatus)
	// 		{
	// 			ast_node_destroy(lhs);
	// 			ast_node_destroy(nextLhs);
	//
	// 			/*
	// 			 * If the parsed ast is not an error but failed, this ast
	// 			 * becomes an error
	// 			*/
	// 			if (!IS_AST_ERR(accessor))
	// 			{
	//
	// 				*out = parser_err_internal(
	// 					parser,
	// 					"postfix expression accessor parse failed"
	// 				);
	// 			}
	// 			else
	// 			{
	// 				*out = accessor;
	// 			}
	//
	// 			return false;
	// 		}
	//
	// 		arr_ast_node_add(
	// 			&nextLhs->as.expressionPostfix.accessors,
	// 			accessor
	// 		);
	// 	}
	//
	//
	// 	/* Ensure there is no trailing increment or decrement */
	// 	if (
	// 		parser_check(parser, TOK_PLUS_PLUS)
	// 		|| parser_check(parser, TOK_MINUS_MINUS)
	// 	)
	// 	{
	// 		ast_node_destroy(lhs);
	// 		ast_node_destroy(nextLhs);
	//
	// 		*out = parser_err_syntax(
	// 			parser,
	// 			"expected a variable, field, or index while parsing "
	// 				"increment/decrement expression."
	// 		);
	//
	// 		return false;
	// 	}
	//
	//
	// 	/* fold the accumulated tree */
	// 	ast_node_t* foldedNode = ast_node_new(
	// 		AST_EXPR_POSTFIX,
	// 		line,
	// 		column
	// 	);
	// 	foldedNode->as.expressionPostfix.lhs = nextLhs;
	// 	foldedNode->as.expressionPostfix.op = op;
	//
	// 	lhs = foldedNode;
	// }
	//
	// if (NULL == lhs)
	// {
	// 	if (!ast_expr_postfix(parser, &lhs))
	// 	{
	// 		/*
	// 		 * If the parsed ast is not an error but failed, this ast
	// 		 * becomes an error
	// 		 */
	// 		if (!IS_AST_ERR(lhs))
	// 		{
	// 			ast_node_destroy(lhs);
	//
	// 			*out = parser_err_internal(
	// 				parser,
	// 				"unary expression right-hand side parse failed"
	// 			);
	// 		}
	//
	// 		return false;
	// 	}
	// }
	//
	//
	// *out = lhs;
	//
	// return true;
	/* AST folding attempt (END) */


	*out = ast_node_new(
		AST_EXPR_POSTFIX,
		parser->current->line,
		parser->current->column
	);

	arr_ast_node_init(&(*out)->as.expressionPostfix.accessors);
	(*out)->as.expressionPostfix.op = TOK_NONE;


	const bool startsWithIdent = parser_check(parser, TOK_IDENT);

	bool lhsParseStatus = false;
	if (startsWithIdent)
	{
		lhsParseStatus = ast_expr_ident(
			parser,
			&(*out)->as.expressionPostfix.lhs
		);
	}
	else
	{
		lhsParseStatus = ast_expr_primary(
			parser,
			&(*out)->as.expressionPostfix.lhs
		);
	}

	/* Handle LHS AST parse failure */
	if (!lhsParseStatus)
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR((*out)->as.expressionPostfix.lhs))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"postfix expression right-hand side parse failed"
			);
		}

		return false;
	}


	/* Handle the in(de)crement */
	if (
		parser_check(parser, TOK_PLUS_PLUS)
		|| parser_check(parser, TOK_MINUS_MINUS)
	)
	{
		if (!startsWithIdent)
		{
			ast_node_destroy(*out);

			*out = parser_err_syntax(
				parser,
				"expected variable, field, or index while parsing "
					"increment/decrement expression."
			);

			return false;
		}

		if (parser_check_and_advance(parser, TOK_PLUS_PLUS))
		{
			(*out)->as.expressionPostfix.op = TOK_PLUS_PLUS;
		}
		else if (parser_check_and_advance(parser, TOK_MINUS_MINUS))
		{
			(*out)->as.expressionPostfix.op = TOK_MINUS_MINUS;
		}

		return true;
	}


	/* Handle any trailing accessors */
    while (true)
    {
        ast_node_t* accessor = NULL;
        bool accessorParseStatus;

        if (parser_check(parser, TOK_DOT))
        {
            accessorParseStatus = ast_accessor_member(parser, &accessor);
        }
        else if (parser_check(parser, TOK_BRACKET_LEFT))
        {
            accessorParseStatus = ast_accessor_index(parser, &accessor);
        }
        else if (parser_check(parser, TOK_PAREN_LEFT))
        {
            accessorParseStatus = ast_accessor_call(parser, &accessor);
        }
        else
        {
            break;
        }


        /* Handle accessor parse failure */
        if (!accessorParseStatus)
        {
            /*
             * If the parsed ast is not an error but failed, this ast
             * becomes an error
             */
            ast_node_destroy(*out);
            if (!IS_AST_ERR(accessor))
            {
                *out = parser_err_internal(
                    parser,
                    "postfix expression accessor parse failed"
                );
            }
            else
            {
                *out = accessor;
            }

            return false;
		}

        arr_ast_node_add(&(*out)->as.expressionPostfix.accessors, accessor);
    }

	if (
		parser_check(parser, TOK_PLUS_PLUS)
		|| parser_check(parser, TOK_MINUS_MINUS)
	)
	{
		ast_node_destroy(*out);

		*out = parser_err_syntax(
			parser,
			"expected a variable, field, or index while parsing "
				"increment/decrement expression."
		);

		return false;
	}

	return true;
}


static bool ast_expr_primary(parser_t* parser, ast_node_t** out)
{
	*out = ast_node_new(
		AST_EXPR_PRIMARY,
		parser->current->line,
		parser->current->column
	);

	/* Handle grouping */
	if (parser_check_and_advance(parser, TOK_PAREN_LEFT))
	{
		if (!ast_expr(parser, &(*out)->as.expressionPrimary.expression))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR((*out)->as.expressionPrimary.expression))
			{
				ast_node_destroy(*out);

				*out = parser_err_internal(
					parser,
					"primary expression grouping expression parse failed"
				);
			}

			return false;
		}


		/* Check for the closing ')' */
		if (!parser_check_and_advance(parser, TOK_PAREN_RIGHT))
		{
			ast_node_destroy(*out);

			*out = parser_err_syntax(
				parser,
				"expected ')' while parsing expression grouping."
			);

			return false;
		}

		return true;
	}


	/* Handle table definition */
	if (parser_check(parser, TOK_BRACE_LEFT))
	{
		if (!ast_table_def(parser, &(*out)->as.expressionPrimary.expression))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR((*out)->as.expressionPrimary.expression))
			{
				ast_node_destroy(*out);

				*out = parser_err_internal(
					parser,
					"primary expression table definition parse failed"
				);
			}

			return false;
		}

		return true;
	}


	/* Handle array definition */
	if (parser_check(parser, TOK_BRACKET_LEFT))
	{
		if (!ast_array_def(parser, &(*out)->as.expressionPrimary.expression))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR((*out)->as.expressionPrimary.expression))
			{
				ast_node_destroy(*out);

				*out = parser_err_internal(
					parser,
					"primary expression array definition parse failed"
				);
			}

			return false;
		}

		return true;
	}


	/* Handle function definition value */
	if (parser_check(parser, TOK_FUNC))
	{
		if (
			!ast_func_def_value(
				parser,
				&(*out)->as.expressionPrimary.expression
			)
		)
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR((*out)->as.expressionPrimary.expression))
			{
				ast_node_destroy(*out);

				*out = parser_err_internal(
					parser,
					"primary expression function definition value parse failed"
				);
			}

			return false;
		}

		return true;
	}


	/* Handle isSet expression */
	if (parser_check(parser, TOK_IS_SET))
	{
		if (!ast_stmt_is_set(parser, &(*out)->as.expressionPrimary.expression))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR((*out)->as.expressionPrimary.expression))
			{
				ast_node_destroy(*out);

				*out = parser_err_internal(
					parser,
					"primary expression isSet statement parse failed"
				);
			}

			return false;
		}

		return true;
	}


	/* Handle identifier */
	if (parser_check(parser, TOK_IDENT))
	{
		if (!ast_identifier(parser, &(*out)->as.expressionPrimary.expression))
		{
			/*
			 * If the parsed ast is not an error but failed, this ast
			 * becomes an error
			 */
			if (!IS_AST_ERR((*out)->as.expressionPrimary.expression))
			{
				ast_node_destroy(*out);

				*out = parser_err_internal(
					parser,
					"primary expression identifier parse failed"
				);
			}

			return false;
		}

		return true;
	}


	/* Handle literal */
	if (!ast_literal(parser, &(*out)->as.expressionPrimary.expression))
	{
		/*
		 * If the parsed ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR((*out)->as.expressionPrimary.expression))
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"primary expression literal parse failed"
			);
		}

		return false;
	}

	return true;
}


static bool ast_func_def_internal(
	parser_t* parser,
	ast_node_t** out,
	const uint32_t line,
	const uint32_t column,
	const bool global
)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_func_def_value(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_accessor_member(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_accessor_index(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_accessor_call(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_table_def(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_table_field(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_array_def(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
	*out = parser_err_unimplemented(parser);
	return false;
}


static bool ast_literal(parser_t* parser, ast_node_t** out)
{
	*out = ast_node_new(
		AST_LITERAL,
		parser->current->line,
		parser->current->column
	);


	/* Handle true */
	if (parser_check_and_advance(parser, TOK_TRUE))
	{
		(*out)->as.literal.type = TOK_TRUE;

		return true;
	}


	/* Handle false */
	if (parser_check_and_advance(parser, TOK_FALSE))
	{
		(*out)->as.literal.type = TOK_FALSE;

		return true;
	}


	/* Handle character */
	if (parser_check(parser, TOK_CHAR))
	{
		(*out)->as.literal.type = TOK_CHAR;
		(*out)->as.literal.value.t_char = parser->current->content[0];

		parser_advance(parser);

		return true;
	}


	/* Handle string */
	if (parser_check(parser, TOK_STRING))
	{
		(*out)->as.literal.type = TOK_STRING;

		const size_t nameLength = parser->current->contentLength + 1;
		(*out)->as.literal.value.t_string = malloc(sizeof(char) * nameLength);
		if (NULL == (*out)->as.literal.value.t_string)
		{
			tula_exit_err_no_mem();
			UNREACHABLE_RETURN(false);
		}

		if (
			0 == str_copy_safe(
				(*out)->as.literal.value.t_string,
				parser->current->content,
				nameLength
			)
		)
		{
			ast_node_destroy(*out);

			*out = parser_err_internal(
				parser,
				"failed to copy string literal"
			);

			return false;
		}


		/* consume the token */
		parser_advance(parser);

		return true;
	}


#define HANDLE_INTEGER_LITERAL( 												\
	token, 																		\
	conv_func, 																	\
	val_field, 																	\
	name, 																		\
	literal_name, 																\
	min, 																		\
	max 																		\
)																				\
	do {																		\
		(*out)->as.literal.type = token;										\
																				\
		switch (																\
			conv_func(															\
				parser->current->content,										\
				&(*out)->as.literal.value.val_field								\
			)																	\
		)																		\
		{																		\
			case NUM_CONV_OK:													\
			{																	\
				parser_advance(parser);											\
				return true;													\
			}																	\
																				\
			case NUM_CONV_INVALID:												\
			{																	\
				ast_node_destroy(*out);											\
																				\
				*out = parser_err_syntax(										\
					parser,														\
					"expected " name											\
						" integer while parsing " literal_name					\
						" literal."												\
				);																\
																				\
				parser_advance(parser);											\
				return false;													\
			}																	\
																				\
			case NUM_CONV_EXCEEDS_RANGE:										\
			{																	\
				ast_node_destroy(*out);											\
																				\
				*out = parser_err_numeric_range(								\
					parser,														\
					"value exceeds " name " integer range."						\
				);																\
																				\
				parser_advance(parser);											\
				return false;													\
			}																	\
																				\
			case NUM_CONV_EXCEEDS_MIN:											\
			{																	\
				ast_node_destroy(*out);											\
																				\
				*out = parser_err_numeric_range(								\
					parser,														\
					"value exceeds " name " integer minimum "					\
						"(" TO_STRING(min) ")."									\
				);																\
																				\
				parser_advance(parser);											\
				return false;													\
			}																	\
																				\
			case NUM_CONV_EXCEEDS_MAX:											\
			{																	\
				ast_node_destroy(*out);											\
																				\
				*out = parser_err_numeric_range(								\
					parser,														\
					"value exceeds " name " integer maximum "					\
						"(" TO_STRING(max) ")."									\
				);																\
																				\
				parser_advance(parser);											\
				return false;													\
			}																	\
																				\
			UNREACHABLE_DEFAULT(break);											\
		}																		\
																				\
		UNREACHABLE_RETURN(false);												\
	} while(0);

	/* Handle int8 */
	if (parser_check(parser, TOK_INT8))
	{
		HANDLE_INTEGER_LITERAL(
			TOK_INT8,
			str_to_int8,
			t_int8,
			"signed 8-bit",
			"int8",
			INT8_MIN,
			INT8_MAX
		)
	}

	/* Handle uint8 */
	if (parser_check(parser, TOK_UINT8))
	{
		HANDLE_INTEGER_LITERAL(
			TOK_UINT8,
			str_to_uint8,
			t_uint8,
			"unsigned 8-bit",
			"uint8",
			0,
			UINT8_MAX
		)
	}

	/* Handle int16 */
	if (parser_check(parser, TOK_INT16))
	{
		HANDLE_INTEGER_LITERAL(
			TOK_INT16,
			str_to_int16,
			t_int16,
			"signed 16-bit",
			"int16",
			INT16_MIN,
			INT16_MAX
		)
	}

	/* Handle uint16 */
	if (parser_check(parser, TOK_UINT16))
	{
		HANDLE_INTEGER_LITERAL(
			TOK_UINT16,
			str_to_uint16,
			t_uint16,
			"unsigned 16-bit",
			"uint16",
			0,
			UINT16_MAX
		)
	}

	/* Handle int32 */
	if (parser_check(parser, TOK_INT32))
	{
		HANDLE_INTEGER_LITERAL(
			TOK_INT32,
			str_to_int32,
			t_int32,
			"signed 32-bit",
			"int32",
			INT32_MIN,
			INT32_MAX
		)
	}

	/* Handle uint32 */
	if (parser_check(parser, TOK_UINT32))
	{
		HANDLE_INTEGER_LITERAL(
			TOK_UINT32,
			str_to_uint32,
			t_uint32,
			"unsigned 32-bit",
			"uint32",
			0,
			UINT32_MAX
		)
	}

	/* Handle int64 */
	if (parser_check(parser, TOK_INT64))
	{
		HANDLE_INTEGER_LITERAL(
			TOK_INT64,
			str_to_int64,
			t_int64,
			"signed 64-bit",
			"int64",
			INT64_MIN,
			INT64_MAX
		)
	}

	/* Handle uint64 */
	if (parser_check(parser, TOK_UINT64))
	{
		HANDLE_INTEGER_LITERAL(
			TOK_UINT64,
			str_to_uint64,
			t_uint64,
			"unsigned 64-bit",
			"uint64",
			0,
			UINT64_MAX
		)
	}

	return false;
}


static bool ast_identifier(parser_t* parser, ast_node_t** out)
{
	/* check for identifier token */
	if (!parser_check(parser, TOK_IDENT))
	{
		*out = parser_err_syntax(
			parser,
			"expected identifier while parsing identifier"
		);

		return false;
	}


	*out = ast_node_new(
		AST_IDENTIFIER,
		parser->current->line,
		parser->current->column
	);


	const size_t nameLength = parser->current->contentLength + 1;
	(*out)->as.identifier.name = malloc(sizeof(char) * nameLength);
	if (NULL == (*out)->as.identifier.name)
	{
		tula_exit_err_no_mem();
		UNREACHABLE_RETURN(false);
	}


	if (
		0 == str_copy_safe(
			(*out)->as.identifier.name,
			parser->current->content,
			nameLength
		)
	)
	{
		ast_node_destroy(*out);

		*out = parser_err_internal(
			parser,
			"failed to copy identifier name string"
		);

		return false;
	}


	/* consume the identifier */
	parser_advance(parser);

	return true;
}


parser_t* parser_new(scanner_t* scanner)
{
	if (NULL == scanner)
	{
		return NULL;
	}

	parser_t* parser = malloc(sizeof(parser_t));
	if (NULL == parser)
	{
		tula_exit_err_no_mem();
		UNREACHABLE_RETURN(NULL);
	}

	parser->scanner = scanner;
	parser->ast = NULL;
	parser->current = NULL;
	parser->hadError = false;

	/* Prime the cursor with the first token */
	parser_advance(parser);

	return parser;
}


void parser_destroy(parser_t* parser)
{
	if (NULL == parser)
	{
		return;
	}

	/* Per the engine's ownership model the parser owns its scanner. */
	if (NULL != parser->scanner)
	{
		scanner_destroy(parser->scanner);
		parser->scanner = NULL;
	}

	if (NULL != parser->ast)
	{
		ast_node_destroy(parser->ast);
		parser->ast = NULL;
	}

	free(parser);
}


ast_node_t* parser_parse(parser_t* parser)
{
	if (NULL == parser)
	{
		return NULL;
	}

	if (NULL == parser->ast)
	{
		if (!ast_program(parser, &parser->ast))
		{
			err_print_f("parser failure!");
		}
	}

	return parser->ast;
}
