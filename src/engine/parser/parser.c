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

/**
 * \brief	Whether a parse result represents a syntax error. Combining parse
 *			functions test their children with this and, on a hit, release any
 *			siblings and propagate the error node upward so exactly one error
 *			bubbles out of the tree.
 */
#define IS_ERR(node) (NULL != (node) && AST_ERROR == (node)->type)


/* Binary operator precedence levels, lowest to highest binding */
#define PREC_NONE			0
#define PREC_OR				1
#define PREC_AND			2
#define PREC_EQUALITY		3
#define PREC_COMPARISON		4
#define PREC_TERM			5
#define PREC_FACTOR			6
#define PREC_POWER			7


/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

/* Token cursor helpers */
static void advance(parser_t* parser);

static bool check(const parser_t* parser, token_type_t type);

static bool match(parser_t* parser, token_type_t type);

static bool is_at_end(const parser_t* parser);

static bool starts_expression(token_type_t type);

static int32_t binary_precedence(token_type_t type);

static char* dup_current(const parser_t* parser);

static ast_node_t* make_error(parser_t* parser, const char* message);

static ast_node_t* make_identifier(parser_t* parser);


/* Grammar — statements */
static ast_node_t* parse_program(parser_t* parser);

static ast_node_t* parse_block(parser_t* parser);

static ast_node_t* parse_statement(parser_t* parser);

static ast_node_t* parse_define(parser_t* parser);

static ast_node_t* parse_set(parser_t* parser);

static ast_node_t* parse_unset(parser_t* parser);

static ast_node_t* parse_named_assignment(
	parser_t* parser,
	ast_node_type_t type,
	bool isGlobal,
	uint32_t line,
	uint32_t column
);

static ast_node_t* parse_function_define(
	parser_t* parser,
	bool isGlobal,
	uint32_t line,
	uint32_t column
);

static ast_node_t* parse_if(parser_t* parser);

static ast_node_t* parse_while(parser_t* parser);

static ast_node_t* parse_do_while(parser_t* parser);

static ast_node_t* parse_for(parser_t* parser);

static ast_node_t* parse_return(parser_t* parser);


/* Grammar — expressions */
static ast_node_t* parse_expression(parser_t* parser);

static ast_node_t* parse_binary(parser_t* parser, int32_t minPrecedence);

static ast_node_t* parse_unary(parser_t* parser);

static ast_node_t* parse_postfix(parser_t* parser);

static ast_node_t* parse_primary(parser_t* parser);

static ast_node_t* parse_array(parser_t* parser);

static ast_node_t* parse_table(parser_t* parser);

static ast_node_t* parse_table_entry(parser_t* parser);

static ast_node_t* parse_function_value(parser_t* parser);

static ast_node_t* parse_parameters(parser_t* parser, arr_node_t* out);


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

static void advance(parser_t* parser)
{
	parser->current = scanner_read_next(parser->scanner);
}


static bool check(const parser_t* parser, const token_type_t type)
{
	return NULL != parser->current && type == parser->current->type;
}


static bool match(parser_t* parser, const token_type_t type)
{
	if (!check(parser, type))
	{
		return false;
	}

	advance(parser);
	return true;
}


static bool is_at_end(const parser_t* parser)
{
	return NULL == parser->current
		|| TOK_EOS == parser->current->type
		|| TOK_ERROR == parser->current->type;
}


static bool starts_expression(const token_type_t type)
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


static int32_t binary_precedence(const token_type_t type)
{
	switch (type)
	{
		case TOK_OR:
		{
			return PREC_OR;
		}

		case TOK_AND:
		{
			return PREC_AND;
		}

		case TOK_EQUAL_EQUAL:
		case TOK_EXCLAM_EQUAL:
		{
			return PREC_EQUALITY;
		}

		case TOK_GREATER_THAN:
		case TOK_LESS_THAN:
		case TOK_GT_EQUAL:
		case TOK_LT_EQUAL:
		{
			return PREC_COMPARISON;
		}

		case TOK_PLUS:
		case TOK_MINUS:
		{
			return PREC_TERM;
		}

		case TOK_STAR:
		case TOK_SLASH_FWD:
		case TOK_PERCENT:
		{
			return PREC_FACTOR;
		}

		case TOK_CARET:
		{
			return PREC_POWER;
		}

		DEFAULT_BREAK
	}

	return PREC_NONE;
}


static char* dup_current(const parser_t* parser)
{
	const char* content = parser->current->content;
	const size_t size = (NULL == content ? 0 : strlen(content)) + 1;

	char* copy = malloc(sizeof(char) * size);
	if (NULL == copy)
	{
		tula_exit_err_no_mem();
		UNREACHABLE_RETURN(NULL);
	}

	str_copy_safe(copy, NULL == content ? "" : content, size);

	return copy;
}


static ast_node_t* make_error(parser_t* parser, const char* message)
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
		if (TOK_ERROR == parser->current->type
			&& NULL != parser->current->content)
		{
			text = parser->current->content;
		}
	}

	return ast_node_new_error(line, column, text);
}


static ast_node_t* make_identifier(parser_t* parser)
{
	ast_node_t* node = ast_node_new(
		AST_IDENTIFIER,
		parser->current->line,
		parser->current->column
	);

	node->as.identifier.name = dup_current(parser);
	advance(parser);

	return node;
}


static ast_node_t* parse_program(parser_t* parser)
{
	const uint32_t line = NULL == parser->current ? 1 : parser->current->line;
	const uint32_t column
		= NULL == parser->current ? 1 : parser->current->column;

	ast_node_t* root = ast_node_new(AST_BLOCK, line, column);

	/* Consume top-level statements until the stream ends */
	while (!is_at_end(parser))
	{
		ast_node_t* statement = parse_statement(parser);
		arr_node_add(&root->as.block.statements, statement);

		if (IS_ERR(statement))
		{
			break;
		}
	}

	/* Surface a trailing scanner error as data in the tree */
	if (!parser->hadError && check(parser, TOK_ERROR))
	{
		arr_node_add(
			&root->as.block.statements,
			make_error(parser, "Unexpected token.")
		);
	}

	return root;
}


static ast_node_t* parse_block(parser_t* parser)
{
	if (!check(parser, TOK_BRACE_LEFT))
	{
		return make_error(parser, "Expected '{' to begin a block.");
	}

	ast_node_t* block = ast_node_new(
		AST_BLOCK,
		parser->current->line,
		parser->current->column
	);
	advance(parser);

	/* Consume statements until the closing brace */
	while (!check(parser, TOK_BRACE_RIGHT) && !is_at_end(parser))
	{
		ast_node_t* statement = parse_statement(parser);
		if (IS_ERR(statement))
		{
			ast_node_destroy(block);
			return statement;
		}

		arr_node_add(&block->as.block.statements, statement);
	}

	if (!match(parser, TOK_BRACE_RIGHT))
	{
		ast_node_destroy(block);
		return make_error(parser, "Expected '}' to close a block.");
	}

	return block;
}


static ast_node_t* parse_statement(parser_t* parser)
{
	switch (parser->current->type)
	{
		case TOK_DEFINE:
		{
			return parse_define(parser);
		}

		case TOK_SET:
		{
			return parse_set(parser);
		}

		case TOK_UNSET:
		{
			return parse_unset(parser);
		}

		case TOK_IF:
		{
			return parse_if(parser);
		}

		case TOK_WHILE:
		{
			return parse_while(parser);
		}

		case TOK_DO:
		{
			return parse_do_while(parser);
		}

		case TOK_FOR:
		{
			return parse_for(parser);
		}

		case TOK_RETURN:
		{
			return parse_return(parser);
		}

		case TOK_BREAK:
		{
			ast_node_t* node = ast_node_new(
				AST_BREAK,
				parser->current->line,
				parser->current->column
			);
			advance(parser);
			return node;
		}

		case TOK_CONTINUE:
		{
			ast_node_t* node = ast_node_new(
				AST_CONTINUE,
				parser->current->line,
				parser->current->column
			);
			advance(parser);
			return node;
		}

		default:
		{
			/*
			 * Anything else is an expression used as a statement — a function
			 * call, an increment/decrement, an isSet query, and so on.
			 */
			ast_node_t* expression = parse_expression(parser);
			if (IS_ERR(expression))
			{
				return expression;
			}

			ast_node_t* statement = ast_node_new(
				AST_EXPR_STATEMENT,
				expression->line,
				expression->column
			);
			statement->as.exprStatement.expression = expression;
			return statement;
		}
	}
}


static ast_node_t* parse_define(parser_t* parser)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;
	advance(parser); /* consume 'define' */

	const bool isGlobal = match(parser, TOK_GLOBAL);

	if (match(parser, TOK_VARIABLE))
	{
		return parse_named_assignment(
			parser,
			AST_VAR_DEFINE,
			isGlobal,
			line,
			column
		);
	}

	if (match(parser, TOK_CONSTANT))
	{
		return parse_named_assignment(
			parser,
			AST_CONST_DEFINE,
			isGlobal,
			line,
			column
		);
	}

	if (match(parser, TOK_FUNC))
	{
		return parse_function_define(parser, isGlobal, line, column);
	}

	return make_error(
		parser,
		"Expected 'variable', 'constant', or 'function' after 'define'."
	);
}


static ast_node_t* parse_set(parser_t* parser)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;
	advance(parser); /* consume 'set' */

	const bool isGlobal = match(parser, TOK_GLOBAL);

	if (!match(parser, TOK_VARIABLE))
	{
		return make_error(parser, "Expected 'variable' after 'set'.");
	}

	return parse_named_assignment(
		parser,
		AST_VAR_SET,
		isGlobal,
		line,
		column
	);
}


static ast_node_t* parse_unset(parser_t* parser)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;
	advance(parser); /* consume 'unset' */

	const bool isGlobal = match(parser, TOK_GLOBAL);

	if (!match(parser, TOK_VARIABLE))
	{
		return make_error(parser, "Expected 'variable' after 'unset'.");
	}

	if (!check(parser, TOK_IDENT))
	{
		return make_error(parser, "Expected a variable name after 'unset'.");
	}

	ast_node_t* node = ast_node_new(AST_VAR_UNSET, line, column);
	node->as.unset.isGlobal = isGlobal;
	node->as.unset.name = dup_current(parser);
	advance(parser);

	return node;
}


static ast_node_t* parse_named_assignment(
	parser_t* parser,
	const ast_node_type_t type,
	const bool isGlobal,
	const uint32_t line,
	const uint32_t column
)
{
	if (!check(parser, TOK_IDENT))
	{
		return make_error(parser, "Expected an identifier name.");
	}

	char* name = dup_current(parser);
	advance(parser);

	if (!match(parser, TOK_EQUAL))
	{
		free(name);
		return make_error(parser, "Expected '=' after the name.");
	}

	ast_node_t* value = parse_expression(parser);
	if (IS_ERR(value))
	{
		free(name);
		return value;
	}

	ast_node_t* node = ast_node_new(type, line, column);
	node->as.variable.isGlobal = isGlobal;
	node->as.variable.name = name;
	node->as.variable.value = value;

	return node;
}


static ast_node_t* parse_function_define(
	parser_t* parser,
	const bool isGlobal,
	const uint32_t line,
	const uint32_t column
)
{
	if (!check(parser, TOK_IDENT))
	{
		return make_error(parser, "Expected a function name.");
	}

	ast_node_t* node = ast_node_new(AST_FUNC_DEFINE, line, column);
	node->as.function.isGlobal = isGlobal;
	node->as.function.name = dup_current(parser);
	advance(parser);

	ast_node_t* paramsError = parse_parameters(
		parser,
		&node->as.function.parameters
	);
	if (IS_ERR(paramsError))
	{
		ast_node_destroy(node);
		return paramsError;
	}

	ast_node_t* body = parse_block(parser);
	if (IS_ERR(body))
	{
		ast_node_destroy(node);
		return body;
	}

	node->as.function.body = body;
	return node;
}


static ast_node_t* parse_if(parser_t* parser)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;
	advance(parser); /* consume 'if' */

	ast_node_t* node = ast_node_new(AST_IF, line, column);

	/* Parse the leading branch and any 'else if' branches */
	for (;;)
	{
		if (!match(parser, TOK_PAREN_LEFT))
		{
			ast_node_destroy(node);
			return make_error(parser, "Expected '(' after 'if'.");
		}

		ast_node_t* condition = parse_expression(parser);
		if (IS_ERR(condition))
		{
			ast_node_destroy(node);
			return condition;
		}

		if (!match(parser, TOK_PAREN_RIGHT))
		{
			ast_node_destroy(condition);
			ast_node_destroy(node);
			return make_error(parser, "Expected ')' after the condition.");
		}

		ast_node_t* body = parse_block(parser);
		if (IS_ERR(body))
		{
			ast_node_destroy(condition);
			ast_node_destroy(node);
			return body;
		}

		arr_node_add(&node->as.conditional.conditions, condition);
		arr_node_add(&node->as.conditional.bodies, body);

		/* An 'else' either starts another branch or ends the chain */
		if (!match(parser, TOK_ELSE))
		{
			break;
		}

		if (match(parser, TOK_IF))
		{
			continue;
		}

		ast_node_t* elseBody = parse_block(parser);
		if (IS_ERR(elseBody))
		{
			ast_node_destroy(node);
			return elseBody;
		}

		node->as.conditional.elseBody = elseBody;
		break;
	}

	return node;
}


static ast_node_t* parse_while(parser_t* parser)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;
	advance(parser); /* consume 'while' */

	if (!match(parser, TOK_PAREN_LEFT))
	{
		return make_error(parser, "Expected '(' after 'while'.");
	}

	ast_node_t* condition = parse_expression(parser);
	if (IS_ERR(condition))
	{
		return condition;
	}

	if (!match(parser, TOK_PAREN_RIGHT))
	{
		ast_node_destroy(condition);
		return make_error(parser, "Expected ')' after the condition.");
	}

	ast_node_t* body = parse_block(parser);
	if (IS_ERR(body))
	{
		ast_node_destroy(condition);
		return body;
	}

	ast_node_t* node = ast_node_new(AST_WHILE, line, column);
	node->as.loop.condition = condition;
	node->as.loop.body = body;

	return node;
}


static ast_node_t* parse_do_while(parser_t* parser)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;
	advance(parser); /* consume 'do' */

	ast_node_t* body = parse_block(parser);
	if (IS_ERR(body))
	{
		return body;
	}

	if (!match(parser, TOK_WHILE))
	{
		ast_node_destroy(body);
		return make_error(parser, "Expected 'while' after the do-block.");
	}

	if (!match(parser, TOK_PAREN_LEFT))
	{
		ast_node_destroy(body);
		return make_error(parser, "Expected '(' after 'while'.");
	}

	ast_node_t* condition = parse_expression(parser);
	if (IS_ERR(condition))
	{
		ast_node_destroy(body);
		return condition;
	}

	if (!match(parser, TOK_PAREN_RIGHT))
	{
		ast_node_destroy(body);
		ast_node_destroy(condition);
		return make_error(parser, "Expected ')' after the condition.");
	}

	ast_node_t* node = ast_node_new(AST_DO_WHILE, line, column);
	node->as.loop.condition = condition;
	node->as.loop.body = body;

	return node;
}


static ast_node_t* parse_for(parser_t* parser)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;
	advance(parser); /* consume 'for' */

	if (!match(parser, TOK_PAREN_LEFT))
	{
		return make_error(parser, "Expected '(' after 'for'.");
	}

	ast_node_t* node = ast_node_new(AST_FOR, line, column);

	/* Initializer — an optional local variable definition */
	if (!check(parser, TOK_COMMA))
	{
		if (!check(parser, TOK_DEFINE))
		{
			ast_node_destroy(node);
			return make_error(
				parser,
				"Expected a variable definition or ',' in the for-initializer."
			);
		}

		ast_node_t* initializer = parse_define(parser);
		if (IS_ERR(initializer))
		{
			ast_node_destroy(node);
			return initializer;
		}

		node->as.forLoop.initializer = initializer;
	}

	if (!match(parser, TOK_COMMA))
	{
		ast_node_destroy(node);
		return make_error(parser, "Expected ',' after the for-initializer.");
	}

	/* Condition — an optional expression */
	if (!check(parser, TOK_COMMA))
	{
		ast_node_t* condition = parse_expression(parser);
		if (IS_ERR(condition))
		{
			ast_node_destroy(node);
			return condition;
		}

		node->as.forLoop.condition = condition;
	}

	if (!match(parser, TOK_COMMA))
	{
		ast_node_destroy(node);
		return make_error(parser, "Expected ',' after the for-condition.");
	}

	/* Update — an optional set or expression */
	if (!check(parser, TOK_PAREN_RIGHT))
	{
		ast_node_t* update = check(parser, TOK_SET)
			? parse_set(parser)
			: parse_expression(parser);
		if (IS_ERR(update))
		{
			ast_node_destroy(node);
			return update;
		}

		node->as.forLoop.update = update;
	}

	if (!match(parser, TOK_PAREN_RIGHT))
	{
		ast_node_destroy(node);
		return make_error(parser, "Expected ')' after the for-clauses.");
	}

	ast_node_t* body = parse_block(parser);
	if (IS_ERR(body))
	{
		ast_node_destroy(node);
		return body;
	}

	node->as.forLoop.body = body;
	return node;
}


static ast_node_t* parse_return(parser_t* parser)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;
	advance(parser); /* consume 'return' */

	ast_node_t* node = ast_node_new(AST_RETURN, line, column);

	/* The returned value is optional */
	if (starts_expression(parser->current->type))
	{
		ast_node_t* value = parse_expression(parser);
		if (IS_ERR(value))
		{
			ast_node_destroy(node);
			return value;
		}

		node->as.ret.value = value;
	}

	return node;
}


static ast_node_t* parse_expression(parser_t* parser)
{
	return parse_binary(parser, PREC_OR);
}


static ast_node_t* parse_binary(parser_t* parser, const int32_t minPrecedence)
{
	ast_node_t* left = parse_unary(parser);
	if (IS_ERR(left))
	{
		return left;
	}

	/* Precedence climbing: fold operators at or above the current threshold */
	for (;;)
	{
		const token_type_t op = parser->current->type;
		const int32_t precedence = binary_precedence(op);

		if (PREC_NONE == precedence || precedence < minPrecedence)
		{
			break;
		}

		advance(parser);

		/* '^' is right-associative; every other operator is left-associative */
		const int32_t nextMinimum = TOK_CARET == op
			? precedence
			: precedence + 1;

		ast_node_t* right = parse_binary(parser, nextMinimum);
		if (IS_ERR(right))
		{
			ast_node_destroy(left);
			return right;
		}

		ast_node_t* binary = ast_node_new(
			AST_BINARY,
			left->line,
			left->column
		);
		binary->as.binary.op = op;
		binary->as.binary.left = left;
		binary->as.binary.right = right;

		left = binary;
	}

	return left;
}


static ast_node_t* parse_unary(parser_t* parser)
{
	/* Logical negation */
	if (check(parser, TOK_NOT))
	{
		const uint32_t line = parser->current->line;
		const uint32_t column = parser->current->column;
		advance(parser);

		ast_node_t* operand = parse_unary(parser);
		if (IS_ERR(operand))
		{
			return operand;
		}

		ast_node_t* node = ast_node_new(AST_UNARY, line, column);
		node->as.unary.op = TOK_NOT;
		node->as.unary.operand = operand;
		return node;
	}

	/* Prefix increment / decrement, which apply only to an identifier */
	if (check(parser, TOK_PLUS_PLUS) || check(parser, TOK_MINUS_MINUS))
	{
		const ast_node_type_t type = check(parser, TOK_PLUS_PLUS)
			? AST_PRE_INCREMENT
			: AST_PRE_DECREMENT;
		const uint32_t line = parser->current->line;
		const uint32_t column = parser->current->column;
		advance(parser);

		if (!check(parser, TOK_IDENT))
		{
			return make_error(
				parser,
				"Expected an identifier after a prefix '++' or '--'."
			);
		}

		ast_node_t* node = ast_node_new(type, line, column);
		node->as.incdec.target = make_identifier(parser);
		return node;
	}

	return parse_postfix(parser);
}


static ast_node_t* parse_postfix(parser_t* parser)
{
	ast_node_t* node = parse_primary(parser);
	if (IS_ERR(node))
	{
		return node;
	}

	/*
	 * Fold trailing accessors, calls, and postfix increment/decrement onto the
	 * operand, so chains like "a.b[0].c(x)" nest left-to-right.
	 */
	for (;;)
	{
		/* Member access: object.name */
		if (check(parser, TOK_DOT))
		{
			advance(parser); /* consume '.' */

			if (!check(parser, TOK_IDENT))
			{
				ast_node_destroy(node);
				return make_error(
					parser,
					"Expected a member name after '.'."
				);
			}

			ast_node_t* member = ast_node_new(
				AST_MEMBER,
				node->line,
				node->column
			);
			member->as.member.object = node;
			member->as.member.name = dup_current(parser);
			advance(parser);

			node = member;
			continue;
		}

		/* Index access: object[subscript] */
		if (check(parser, TOK_BRACKET_LEFT))
		{
			ast_node_t* index = ast_node_new(
				AST_INDEX,
				node->line,
				node->column
			);
			index->as.index.object = node;
			advance(parser); /* consume '[' */

			ast_node_t* subscript = parse_expression(parser);
			if (IS_ERR(subscript))
			{
				ast_node_destroy(index);
				return subscript;
			}

			index->as.index.subscript = subscript;

			if (!match(parser, TOK_BRACKET_RIGHT))
			{
				ast_node_destroy(index);
				return make_error(
					parser,
					"Expected ']' to close the index access."
				);
			}

			node = index;
			continue;
		}

		/* Call: callee(arguments) */
		if (check(parser, TOK_PAREN_LEFT))
		{
			ast_node_t* call = ast_node_new(
				AST_CALL,
				node->line,
				node->column
			);
			call->as.call.callee = node;
			advance(parser); /* consume '(' */

			if (!check(parser, TOK_PAREN_RIGHT))
			{
				for (;;)
				{
					ast_node_t* argument = parse_expression(parser);
					if (IS_ERR(argument))
					{
						ast_node_destroy(call);
						return argument;
					}

					arr_node_add(&call->as.call.arguments, argument);

					if (match(parser, TOK_COMMA))
					{
						continue;
					}

					break;
				}
			}

			if (!match(parser, TOK_PAREN_RIGHT))
			{
				ast_node_destroy(call);
				return make_error(
					parser,
					"Expected ')' to close the call arguments."
				);
			}

			node = call;
			continue;
		}

		/* Postfix increment / decrement, which apply only to an identifier */
		if ((check(parser, TOK_PLUS_PLUS) || check(parser, TOK_MINUS_MINUS))
			&& AST_IDENTIFIER == node->type)
		{
			const ast_node_type_t type = check(parser, TOK_PLUS_PLUS)
				? AST_POST_INCREMENT
				: AST_POST_DECREMENT;

			ast_node_t* wrapper = ast_node_new(
				type,
				node->line,
				node->column
			);
			wrapper->as.incdec.target = node;
			advance(parser);

			node = wrapper;
			continue;
		}

		break;
	}

	return node;
}


static ast_node_t* parse_primary(parser_t* parser)
{
	const token_t* token = parser->current;

	/* Primitive literals (numbers, strings, chars, booleans) */
	if (TOKENS_IS_PRIMITIVE[token->type])
	{
		ast_node_t* node = ast_node_new(
			AST_LITERAL,
			token->line,
			token->column
		);
		node->as.literal.literalType = token->type;
		node->as.literal.value = dup_current(parser);
		advance(parser);
		return node;
	}

	switch (token->type)
	{
		case TOK_IDENT:
		{
			return make_identifier(parser);
		}

		case TOK_IS_SET:
		{
			const uint32_t line = token->line;
			const uint32_t column = token->column;
			advance(parser); /* consume 'isSet' */

			if (!match(parser, TOK_PAREN_LEFT))
			{
				return make_error(parser, "Expected '(' after 'isSet'.");
			}

			if (!check(parser, TOK_IDENT))
			{
				return make_error(
					parser,
					"Expected an identifier inside 'isSet(...)'."
				);
			}

			ast_node_t* target = make_identifier(parser);

			if (!match(parser, TOK_PAREN_RIGHT))
			{
				ast_node_destroy(target);
				return make_error(
					parser,
					"Expected ')' after the 'isSet' argument."
				);
			}

			ast_node_t* node = ast_node_new(AST_IS_SET, line, column);
			node->as.isSet.target = target;
			return node;
		}

		case TOK_PAREN_LEFT:
		{
			advance(parser); /* consume '(' */

			ast_node_t* expression = parse_expression(parser);
			if (IS_ERR(expression))
			{
				return expression;
			}

			if (!match(parser, TOK_PAREN_RIGHT))
			{
				ast_node_destroy(expression);
				return make_error(
					parser,
					"Expected ')' after the expression."
				);
			}

			return expression;
		}

		case TOK_BRACKET_LEFT:
		{
			return parse_array(parser);
		}

		case TOK_BRACE_LEFT:
		{
			return parse_table(parser);
		}

		case TOK_FUNC:
		{
			return parse_function_value(parser);
		}

		DEFAULT_BREAK
	}

	return make_error(parser, "Expected an expression.");
}


static ast_node_t* parse_array(parser_t* parser)
{
	ast_node_t* node = ast_node_new(
		AST_ARRAY,
		parser->current->line,
		parser->current->column
	);
	advance(parser); /* consume '[' */

	/* Elements, allowing an optional trailing comma */
	while (!check(parser, TOK_BRACKET_RIGHT) && !is_at_end(parser))
	{
		ast_node_t* element = parse_expression(parser);
		if (IS_ERR(element))
		{
			ast_node_destroy(node);
			return element;
		}

		arr_node_add(&node->as.array.elements, element);

		if (!match(parser, TOK_COMMA))
		{
			break;
		}
	}

	if (!match(parser, TOK_BRACKET_RIGHT))
	{
		ast_node_destroy(node);
		return make_error(parser, "Expected ']' to close the array.");
	}

	return node;
}


static ast_node_t* parse_table(parser_t* parser)
{
	ast_node_t* node = ast_node_new(
		AST_TABLE,
		parser->current->line,
		parser->current->column
	);
	advance(parser); /* consume '{' */

	/* Entries, allowing an optional trailing comma */
	while (!check(parser, TOK_BRACE_RIGHT) && !is_at_end(parser))
	{
		ast_node_t* entry = parse_table_entry(parser);
		if (IS_ERR(entry))
		{
			ast_node_destroy(node);
			return entry;
		}

		arr_node_add(&node->as.table.entries, entry);

		if (!match(parser, TOK_COMMA))
		{
			break;
		}
	}

	if (!match(parser, TOK_BRACE_RIGHT))
	{
		ast_node_destroy(node);
		return make_error(parser, "Expected '}' to close the table.");
	}

	return node;
}


static ast_node_t* parse_table_entry(parser_t* parser)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;

	if (!match(parser, TOK_BRACKET_LEFT))
	{
		return make_error(parser, "Expected '[' to begin a table key.");
	}

	/* Keys are limited to string, char, or integer literals */
	const token_type_t keyType = parser->current->type;
	const bool isIntegerKey = keyType >= TOK_INT8 && keyType <= TOK_UINT64;
	if (TOK_STRING != keyType && TOK_CHAR != keyType && !isIntegerKey)
	{
		return make_error(
			parser,
			"Table keys must be a string, char, or integer literal."
		);
	}

	ast_node_t* key = ast_node_new(AST_LITERAL, line, column);
	key->as.literal.literalType = keyType;
	key->as.literal.value = dup_current(parser);
	advance(parser);

	if (!match(parser, TOK_BRACKET_RIGHT))
	{
		ast_node_destroy(key);
		return make_error(parser, "Expected ']' after the table key.");
	}

	if (!match(parser, TOK_EQUAL))
	{
		ast_node_destroy(key);
		return make_error(parser, "Expected '=' after the table key.");
	}

	ast_node_t* value = parse_expression(parser);
	if (IS_ERR(value))
	{
		ast_node_destroy(key);
		return value;
	}

	ast_node_t* entry = ast_node_new(AST_TABLE_ENTRY, line, column);
	entry->as.tableEntry.key = key;
	entry->as.tableEntry.value = value;

	return entry;
}


static ast_node_t* parse_function_value(parser_t* parser)
{
	const uint32_t line = parser->current->line;
	const uint32_t column = parser->current->column;
	advance(parser); /* consume 'function' */

	ast_node_t* node = ast_node_new(AST_FUNCTION, line, column);

	ast_node_t* paramsError = parse_parameters(
		parser,
		&node->as.function.parameters
	);
	if (IS_ERR(paramsError))
	{
		ast_node_destroy(node);
		return paramsError;
	}

	ast_node_t* body = parse_block(parser);
	if (IS_ERR(body))
	{
		ast_node_destroy(node);
		return body;
	}

	node->as.function.body = body;
	return node;
}


static ast_node_t* parse_parameters(parser_t* parser, arr_node_t* out)
{
	if (!match(parser, TOK_PAREN_LEFT))
	{
		return make_error(parser, "Expected '(' to begin the parameter list.");
	}

	if (!check(parser, TOK_PAREN_RIGHT))
	{
		for (;;)
		{
			if (!check(parser, TOK_IDENT))
			{
				return make_error(parser, "Expected a parameter name.");
			}

			arr_node_add(out, make_identifier(parser));

			if (match(parser, TOK_COMMA))
			{
				continue;
			}

			break;
		}
	}

	if (!match(parser, TOK_PAREN_RIGHT))
	{
		return make_error(parser, "Expected ')' to close the parameter list.");
	}

	/* NULL signals success; a non-error return value is never produced */
	return NULL;
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
	advance(parser);

	return parser;
}


void parser_destroy(parser_t* parser)
{
	if (NULL == parser)
	{
		return;
	}

	/*
	 * Per the engine's ownership model the parser owns its scanner and should
	 * release it here. That is deferred for now: scanner_destroy ->
	 * arr_token_destroy corrupts the heap when tearing down a multi-token array
	 * (see the BUG note in engine/scanner/token.c). Once that is fixed under
	 * its own commit and regression test, restore the scanner_destroy call
	 * below. Until then the scanner is left for process exit to reclaim, which
	 * matches how the scanner test harness already behaves.
	 */
	/* scanner_destroy(parser->scanner); */
	parser->scanner = NULL;

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
		parser->ast = parse_program(parser);
	}

	return parser->ast;
}
