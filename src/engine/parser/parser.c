#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "common/exit.h"
#include "common/strings.h"

/*
 * ==================================================
 * Macros
 * ==================================================
*/

#define IS_AST_ERR(node) (NULL != (node) && AST_ERROR == (node)->type)



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


static bool ast_stmt_const_def(parser_t* parser, ast_node_t** out);


static bool ast_stmt_var_def(parser_t* parser, ast_node_t** out);


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


static bool ast_func_def(parser_t* parser, ast_node_t** out);


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

		/*
		 * If the statement ast is not an error but failed, this ast
		 * becomes an error
		 */
		if (!IS_AST_ERR(statement))
		{
			ast_node_destroy(*out);

			*out = parser_make_error(
				parser,
				"Internal Parser Failure: program statements parse failed"
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
			parser_make_error(parser, "Syntax Error: unexpected token.")
		);
	}

	return hadError;
}


static bool ast_block(parser_t* parser, ast_node_t** out)
{
	/* Find the opening { */
	if (!parser_check(parser, TOK_BRACE_LEFT))
	{
		*out = parser_make_error(
			parser,
			"Syntax Error: expected '{' while parsing block."
		);

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

			*out = parser_make_error(
				parser,
				"Internal Parser Failure: block statements parse failed"
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
			parser_make_error(parser, "Syntax Error: unexpected token.")
		);
	}


	/* Find the closing } */
	if (!parser_check_and_advance(parser, TOK_BRACE_RIGHT))
	{
		ast_node_destroy(*out);

		*out = parser_make_error(
			parser,
			"Syntax Error: expected '}' while parsing block."
		);

		return false;
	}

	return hadError;
}


static bool ast_statement(parser_t* parser, ast_node_t** out)
{
	*out = ast_node_new(
		AST_STATEMENT,
		parser->current->line,
		parser->current->column
	);

	/* TODO: Add from EBNF "function_call_statement" */
	switch (parser->current->type)
	{
		case TOK_BREAK:
		{
			(*out)->as.statement.statement = ast_node_new(
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
			(*out)->as.statement.statement = ast_node_new(
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
			if (ast_stmt_is_set(parser, &(*out)->as.statement.statement))
			{
				return true;
			}

			goto handle_parse_failure_and_exit;
		}

		case TOK_DEFINE:
		{
			/* TODO: Investigate look-ahead */
			(*out)->as.statement.statement = parser_make_error(
				parser,
				"Branch Unimplemented: " __FILE__ ":" STRINGIFY(__LINE__)
			);

			return false;
		}

		case TOK_SET:
		{
			if (ast_stmt_var_set(parser, &(*out)->as.statement.statement))
			{
				return true;
			}

			goto handle_parse_failure_and_exit;
		}

		case TOK_UNSET:
		{
			if (ast_stmt_var_unset(parser, &(*out)->as.statement.statement))
			{
				return true;
			}

			goto handle_parse_failure_and_exit;
		}

		case TOK_IF:
		{
			if (ast_stmt_comp(parser, &(*out)->as.statement.statement))
			{
				return true;
			}

			goto handle_parse_failure_and_exit;
		}

		case TOK_DO:
		case TOK_WHILE:
		{
			if (ast_stmt_cond_iter(parser, &(*out)->as.statement.statement))
			{
				return true;
			}

			goto handle_parse_failure_and_exit;
		}

		case TOK_FOR:
		{
			if (ast_stmt_num_iter(parser, &(*out)->as.statement.statement))
			{
				return true;
			}

			goto handle_parse_failure_and_exit;
		}

		case TOK_RETURN:
		{
			if (ast_stmt_return(parser, &(*out)->as.statement.statement))
			{
				return true;
			}

			goto handle_parse_failure_and_exit;
		}

		default:
		{
			(*out)->as.statement.statement = parser_make_error(
				parser,
				"Syntax Error: unexpected token."
			);

			return false;
		}
	}

handle_parse_failure_and_exit:
	/*
	 * If the parsed ast is not an error but failed, this ast
	 * becomes an error
	 */
	if (!IS_AST_ERR((*out)->as.statement.statement))
	{
		ast_node_destroy(*out);

		*out = parser_make_error(
			parser,
			"Internal Parser Failure: statement parse failed"
		);
	}

	return false;
}


static bool ast_stmt_return(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_num_iter(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_cond_iter(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_comp(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_func_call(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_const_def(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_var_def(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_var_set(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_var_unset(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_is_set(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_break(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_stmt_continue(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_condition(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_ident(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_logi_or(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_logi_and(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_equ(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_comp(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_add(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_mult(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_expo(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_unary(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_postfix(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_expr_primary(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_func_def(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_func_def_value(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_accessor_member(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_accessor_index(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_accessor_call(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_table_def(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_table_field(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_array_def(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_literal(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
}


static bool ast_identifier(parser_t* parser, ast_node_t** out)
{
	// TODO: Implement
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
		ast_program(parser, &parser->ast);
	}

	return parser->ast;
}
