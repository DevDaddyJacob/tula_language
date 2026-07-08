#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "common/buffered_reader.h"
#include "engine/parser/ast.h"
#include "engine/parser/parser.h"
#include "engine/scanner/scanner.h"

/*
 * ==================================================
 * Helpers
 * ==================================================
 */

#define TEMP_FILE "test_parser_tmp.tula"

static void write_temp_file(const char* content)
{
	FILE* f = fopen(TEMP_FILE, "wb");
	if (NULL == f)
	{
		TEST_FAIL_MESSAGE("Could not create temp file");
	}

	fwrite(content, 1, strlen(content), f);
	fclose(f);
}


/**
 * \brief	Builds a parser over the given source. The parser owns the scanner
 *			and reader, so a single parser_destroy tears the whole chain down.
 */
static parser_t* parser_for(const char* source)
{
	write_temp_file(source);

	buf_reader_t* reader = buf_reader_new(NULL);
	TEST_ASSERT_TRUE(buf_reader_open(reader, TEMP_FILE));

	scanner_t* scanner = scanner_new(reader);
	return parser_new(scanner);
}


/*
 * ==================================================
 * setUp / tearDown
 * ==================================================
 */

void setUp(void) { }

void tearDown(void)
{
	remove(TEMP_FILE);
}


/* -------------------------------------------------------------------------
 * AST module basics
 * ------------------------------------------------------------------------- */

void test_ast_node_new_zero_initializes(void)
{
	ast_node_t* node = ast_node_new(AST_RETURN, 3, 7);

	TEST_ASSERT_EQUAL_INT(AST_RETURN, node->type);
	TEST_ASSERT_EQUAL_UINT32(3, node->line);
	TEST_ASSERT_EQUAL_UINT32(7, node->column);
	TEST_ASSERT_NULL(node->as.ret.value);

	ast_node_destroy(node);
}


void test_ast_node_type_value_table_is_complete(void)
{
#define DEFINER(identifier, value) \
	TEST_ASSERT_NOT_NULL(AST_NODE_TYPE_VALUE[identifier]); \
	TEST_ASSERT_EQUAL_STRING(value, AST_NODE_TYPE_VALUE[identifier]);

	DEFINE_AST_NODE_TYPES(DEFINER)
#undef DEFINER
}


void test_arr_node_add_grows(void)
{
	arr_ast_node_t array;
	arr_node_init(&array);

	for (uint32_t i = 0; i < 20; i++)
	{
		arr_node_add(&array, ast_node_new(AST_BREAK, i, 0));
	}

	TEST_ASSERT_EQUAL_size_t(20, array.count);
	TEST_ASSERT_TRUE(array.capacity >= array.count);
	TEST_ASSERT_EQUAL_UINT32(19, array.values[19]->line);

	arr_node_destroy(&array);
}


/* -------------------------------------------------------------------------
 * Statement parsing
 * ------------------------------------------------------------------------- */

void test_parse_produces_block_root(void)
{
	parser_t* parser = parser_for("break\ncontinue\n");
	ast_node_t* root = parser_parse(parser);

	TEST_ASSERT_EQUAL_INT(AST_BLOCK, root->type);
	TEST_ASSERT_EQUAL_size_t(2, root->as.block.statements.count);
	TEST_ASSERT_EQUAL_INT(
		AST_BREAK,
		root->as.block.statements.values[0]->type
	);
	TEST_ASSERT_EQUAL_INT(
		AST_CONTINUE,
		root->as.block.statements.values[1]->type
	);
	TEST_ASSERT_FALSE(parser->hadError);

	parser_destroy(parser);
}


void test_parse_variable_definition(void)
{
	parser_t* parser = parser_for("def var x = 42\n");
	ast_node_t* root = parser_parse(parser);

	ast_node_t* define = root->as.block.statements.values[0];
	TEST_ASSERT_EQUAL_INT(AST_VAR_DEFINE, define->type);
	TEST_ASSERT_FALSE(define->as.variable.isGlobal);
	TEST_ASSERT_EQUAL_STRING("x", define->as.variable.name);

	ast_node_t* value = define->as.variable.value;
	TEST_ASSERT_EQUAL_INT(AST_LITERAL, value->type);
	TEST_ASSERT_EQUAL_INT(TOK_INT32, value->as.literal.literalType);
	TEST_ASSERT_EQUAL_STRING("42", value->as.literal.value);

	parser_destroy(parser);
}


void test_parse_global_constant_definition(void)
{
	parser_t* parser = parser_for("def global const PI = 3\n");
	ast_node_t* root = parser_parse(parser);

	ast_node_t* define = root->as.block.statements.values[0];
	TEST_ASSERT_EQUAL_INT(AST_CONST_DEFINE, define->type);
	TEST_ASSERT_TRUE(define->as.variable.isGlobal);
	TEST_ASSERT_EQUAL_STRING("PI", define->as.variable.name);

	parser_destroy(parser);
}


/* -------------------------------------------------------------------------
 * Expression parsing
 * ------------------------------------------------------------------------- */

void test_parse_respects_operator_precedence(void)
{
	/* 1 + 2 * 3 must nest as 1 + (2 * 3) */
	parser_t* parser = parser_for("def var y = 1 + 2 * 3\n");
	ast_node_t* root = parser_parse(parser);

	ast_node_t* value = root->as.block.statements.values[0]->as.variable.value;
	TEST_ASSERT_EQUAL_INT(AST_BINARY, value->type);
	TEST_ASSERT_EQUAL_INT(TOK_PLUS, value->as.binary.op);
	TEST_ASSERT_EQUAL_INT(AST_LITERAL, value->as.binary.left->type);

	ast_node_t* right = value->as.binary.right;
	TEST_ASSERT_EQUAL_INT(AST_BINARY, right->type);
	TEST_ASSERT_EQUAL_INT(TOK_STAR, right->as.binary.op);

	parser_destroy(parser);
}


void test_parse_power_is_right_associative(void)
{
	/* 2 ^ 3 ^ 2 must nest as 2 ^ (3 ^ 2) */
	parser_t* parser = parser_for("def var y = 2 ^ 3 ^ 2\n");
	ast_node_t* root = parser_parse(parser);

	ast_node_t* value = root->as.block.statements.values[0]->as.variable.value;
	TEST_ASSERT_EQUAL_INT(AST_BINARY, value->type);
	TEST_ASSERT_EQUAL_INT(TOK_CARET, value->as.binary.op);
	TEST_ASSERT_EQUAL_INT(AST_LITERAL, value->as.binary.left->type);
	TEST_ASSERT_EQUAL_INT(AST_BINARY, value->as.binary.right->type);

	parser_destroy(parser);
}


void test_parse_function_call_statement(void)
{
	parser_t* parser = parser_for("add(1, x)\n");
	ast_node_t* root = parser_parse(parser);

	ast_node_t* statement = root->as.block.statements.values[0];
	TEST_ASSERT_EQUAL_INT(AST_EXPR_STATEMENT, statement->type);

	ast_node_t* call = statement->as.exprStatement.expression;
	TEST_ASSERT_EQUAL_INT(AST_CALL, call->type);
	TEST_ASSERT_EQUAL_INT(AST_IDENTIFIER, call->as.call.callee->type);
	TEST_ASSERT_EQUAL_STRING("add", call->as.call.callee->as.identifier.name);
	TEST_ASSERT_EQUAL_size_t(2, call->as.call.arguments.count);

	parser_destroy(parser);
}


void test_parse_member_and_index_access_chain(void)
{
	/* grid[row].value nests as member(index(grid, row), "value") */
	parser_t* parser = parser_for("def var v = grid[row].value\n");
	ast_node_t* root = parser_parse(parser);

	ast_node_t* value = root->as.block.statements.values[0]->as.variable.value;
	TEST_ASSERT_EQUAL_INT(AST_MEMBER, value->type);
	TEST_ASSERT_EQUAL_STRING("value", value->as.member.name);

	ast_node_t* object = value->as.member.object;
	TEST_ASSERT_EQUAL_INT(AST_INDEX, object->type);
	TEST_ASSERT_EQUAL_INT(AST_IDENTIFIER, object->as.index.object->type);
	TEST_ASSERT_EQUAL_STRING(
		"grid",
		object->as.index.object->as.identifier.name
	);
	TEST_ASSERT_EQUAL_INT(AST_IDENTIFIER, object->as.index.subscript->type);

	parser_destroy(parser);
}


void test_parse_call_on_member_access(void)
{
	/* io.print(x) is a call whose callee is a member access */
	parser_t* parser = parser_for("io.print(x)\n");
	ast_node_t* root = parser_parse(parser);

	ast_node_t* call =
		root->as.block.statements.values[0]->as.exprStatement.expression;
	TEST_ASSERT_EQUAL_INT(AST_CALL, call->type);
	TEST_ASSERT_EQUAL_INT(AST_MEMBER, call->as.call.callee->type);
	TEST_ASSERT_EQUAL_STRING("print", call->as.call.callee->as.member.name);
	TEST_ASSERT_EQUAL_size_t(1, call->as.call.arguments.count);

	parser_destroy(parser);
}


/* -------------------------------------------------------------------------
 * Error handling
 * ------------------------------------------------------------------------- */

void test_parse_reports_syntax_error_as_node(void)
{
	parser_t* parser = parser_for("def var z = )\n");
	ast_node_t* root = parser_parse(parser);

	TEST_ASSERT_TRUE(parser->hadError);

	ast_node_t* last = root->as.block.statements.values[
		root->as.block.statements.count - 1
	];
	TEST_ASSERT_EQUAL_INT(AST_ERROR, last->type);
	TEST_ASSERT_NOT_NULL(last->as.error.message);

	parser_destroy(parser);
}


int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_ast_node_new_zero_initializes);
	RUN_TEST(test_ast_node_type_value_table_is_complete);
	RUN_TEST(test_arr_node_add_grows);

	RUN_TEST(test_parse_produces_block_root);
	RUN_TEST(test_parse_variable_definition);
	RUN_TEST(test_parse_global_constant_definition);

	RUN_TEST(test_parse_respects_operator_precedence);
	RUN_TEST(test_parse_power_is_right_associative);
	RUN_TEST(test_parse_function_call_statement);
	RUN_TEST(test_parse_member_and_index_access_chain);
	RUN_TEST(test_parse_call_on_member_access);

	RUN_TEST(test_parse_reports_syntax_error_as_node);

	return UNITY_END();
}
