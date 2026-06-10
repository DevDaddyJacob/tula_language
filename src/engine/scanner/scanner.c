#include "scanner.h"

#include <string.h>

#include "tula.h"
#include "common/strings.h"

/*
 * ==================================================
 * Macros
 * ==================================================
 */

/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

static token_type_t try_infer_identifier_type(
	const char* content,
	size_t contentLength,
	int32_t offset,
	token_type_t type,
	bool useAltValue
);


static token_type_t get_identifier_type(
	const char* content,
	size_t contentLength
);


static void scanner_skip_whitespace(const scanner_t* scanner);


static token_t* scanner_consume_operator(
	scanner_t* lexer,
	token_type_t type,
	size_t size
);


static token_t* scanner_consume_identifier(const scanner_t* scanner);


static token_t* scanner_consume_number(scanner_t* scanner);


static token_t* scanner_consume_string(const scanner_t* scanner);


static token_t* scanner_consume_char(const scanner_t* scanner);

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

static token_type_t try_infer_identifier_type(
	const char* content,
	const size_t contentLength,
	const int32_t offset,
	const token_type_t type,
	const bool useAltValue
)
{
	if (!TOKENS_IS_KEYWORD[type] && !TOKENS_IS_OPERATOR[type])
	{
		return TOK_IDENT;
	}

	const char* typeValue;
	if (useAltValue)
	{
		typeValue = TOKENS_VALUE_ALT[type];
	}
	else
	{
		typeValue = TOKENS_VALUE[type];
	}

	if (NULL == typeValue)
	{
		return TOK_IDENT;
	}

	for (int32_t i = offset; i < strlen(typeValue); i++)
	{
		if (i > contentLength)
		{
			return TOK_IDENT;
		}

		if (typeValue[i] != content[i])
		{
			return TOK_IDENT;
		}
	}

	return type;
}


static token_type_t get_identifier_type(
	const char* content,
	const size_t contentLength
)
{
	if (1 == contentLength)
	{
		goto end_with_default;
	}

	switch (content[0])
	{
		case 'a':
		{
			/* keyword "and" */
			return try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_AND,
				false
			);
		}

		case 'b':
		{
			/* keyword "break" */
			return try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_BREAK,
				false
			);
		}

		case 'c':
		{
			goto handle_c;
		}

		case 'd':
		{
			goto handle_d;
		}

		case 'e':
		{
			goto handle_e;
		}

		case 'f':
		{
			goto handle_f;
		}

		case 'g':
		{
			/* keyword "global" */
			return try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_GLOBAL,
				false
			);
		}

		case 'i':
		{
			goto handle_i;
		}

		case 'n':
		{
			/* keyword "not" */
			return try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_NOT,
				false
			);
		}

		case 'o':
		{
			/* keyword "or" */
			return try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_OR,
				false
			);
		}

		case 'r':
		{
			/* keyword "return" */
			return try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_RETURN,
				false
			);
		}

		case 's':
		{
			/* keyword "set" */
			return try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_SET,
				false
			);
		}

		case 't':
		{
			/* keyword "true" */
			return try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_TRUE,
				false
			);
		}

		case 'u':
		{
			/* keyword "unset" */
			return try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_UNSET,
				false
			);
		}

		case 'v':
		{
			/* keyword "variable" */
			const token_type_t inferred = try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_VARIABLE,
				false
			);

			if (TOK_VARIABLE != inferred)
			{
				/* keyword "var" */
				return try_infer_identifier_type(
					content, contentLength,
					1,
					TOK_VARIABLE,
					true
				);
			}

			return inferred;
		}

		case 'w':
		{
			/* keyword "while" */
			return try_infer_identifier_type(
				content, contentLength,
				1,
				TOK_WHILE,
				false
			);
		}

		DEFAULT_BREAK
	}


end_with_default:
	return TOK_IDENT;


handle_c:
	if (2 > contentLength)
	{
		goto end_with_default;
	}

	switch (content[1])
	{
		case 'o':
		{
			goto handle_co;
		}

		DEFAULT_BREAK
	}

	goto end_with_default;


handle_co:
	if (3 > contentLength)
	{
		goto end_with_default;
	}

	switch (content[2])
	{
		case 'n':
		{
			goto handle_con;
		}

		DEFAULT_BREAK
	}

	goto end_with_default;


handle_con:
	if (4 > contentLength)
	{
		goto end_with_default;
	}

	switch (content[3])
	{
		case 's':
		{
			goto handle_cons;
		}

		case 't':
		{
			/* keyword "continue" */
			return try_infer_identifier_type(
				content, contentLength,
				4,
				TOK_CONTINUE,
				false
			);
		}

		DEFAULT_BREAK
	}

	goto end_with_default;


handle_cons:
	if (5 > contentLength)
	{
		goto end_with_default;
	}

	switch (content[4])
	{
		case 't':
		{
			if (6 > contentLength)
			{
				/* keyword "const" */
				return TOK_CONSTANT;
			}

			switch (content[4])
			{
				case ' ':
				case '\t':
				case '\r':
				case '\n':
				{
					/* keyword "const" */
					return TOK_CONSTANT;
				}

				default:
				{
					/* keyword "constant" */
					return try_infer_identifier_type(
						content, contentLength,
						5,
						TOK_CONSTANT,
						false
					);
				}
			}
		}

		DEFAULT_BREAK
	}

	goto end_with_default;


handle_d:
	if (2 > contentLength)
	{
		goto end_with_default;
	}

	switch (content[1])
	{
		case 'e':
		{
			goto handle_de;
		}

		case 'o':
		{
			/* keyword "do" */
			return TOK_DO;
		}

		DEFAULT_BREAK
	}

	goto end_with_default;


handle_de:
	if (3 > contentLength)
	{
		goto end_with_default;
	}

	switch (content[2])
	{
		case 'f':
		{
			if (4 > contentLength)
			{
				/* keyword "def" */
				return TOK_DEFINE;
			}

			switch (content[3])
			{
				case ' ':
				case '\t':
				case '\r':
				case '\n':
				{
					/* keyword "def" */
					return TOK_DEFINE;
				}

				default:
				{
					/* keyword "define" */
					return try_infer_identifier_type(
						content, contentLength,
						4,
						TOK_DEFINE,
						false
					);
				}
			}

			break;
		}

		DEFAULT_BREAK
	}

	goto end_with_default;


handle_e:
	if (2 > contentLength)
	{
		goto end_with_default;
	}

	switch (content[1])
	{
		case 'l':
		{
			/* keyword "else" */
			return try_infer_identifier_type(
				content, contentLength,
				2,
				TOK_ELSE,
				false
			);
		}

		case 'm':
		{
			/* keyword "emit" */
			return try_infer_identifier_type(
				content, contentLength,
				2,
				TOK_EMIT,
				false
			);
		}

		DEFAULT_BREAK
	}

	goto end_with_default;


handle_f:
	if (2 > contentLength)
	{
		goto end_with_default;
	}

	switch (content[1])
	{
		case 'a':
		{
			/* keyword "false" */
			return try_infer_identifier_type(
				content, contentLength,
				2,
				TOK_FALSE,
				false
			);
		}

		case 'o':
		{
			/* keyword "for" */
			return try_infer_identifier_type(
				content, contentLength,
				2,
				TOK_FOR,
				false
			);
		}

		case 'u':
		{
			/* keyword "function" */
			const token_type_t inferred = try_infer_identifier_type(
				content, contentLength,
				2,
				TOK_FUNC,
				false
			);

			if (TOK_FUNC != inferred)
			{
				/* keyword "func" */
				return try_infer_identifier_type(
					content, contentLength,
					2,
					TOK_FUNC,
					true
				);
			}

			return inferred;
		}

		DEFAULT_BREAK
	}

	goto end_with_default;


handle_i:
	if (2 > contentLength)
	{
		goto end_with_default;
	}

	switch (content[1])
	{
		case 'f':
		{
			/* keyword "if" */
			return TOK_IF;
		}

		case 's':
		{
			/* keyword "isSet" */
			return try_infer_identifier_type(
				content, contentLength,
				2,
				TOK_IS_SET,
				false
			);
		}

		DEFAULT_BREAK
	}

	goto end_with_default;
}


static void scanner_skip_whitespace(const scanner_t* scanner)
{
	while (buf_reader_has_next(scanner->reader))
	{
		switch (buf_reader_peek(scanner->reader))
		{
			/* handle standard whitespace characters */
			case ' ':
			case '\r':
			case '\t':
			case '\n':
			{
				buf_reader_consume(scanner->reader);
				break;
			}

			/* handle comments which are essentially honorary whitespace */
			case '/':
			{
				const int32_t nextChar = buf_reader_peek_n(scanner->reader, 1);

				if ('/' == nextChar)
				{
					while (
						buf_reader_has_next(scanner->reader)
						&& '\n' != buf_reader_peek(scanner->reader)
					)
					{
						buf_reader_consume(scanner->reader);
					}

					continue;
				}

				if ('*' == nextChar)
				{
					/* eat the '*' up next to prime the while loop */
					buf_reader_consume(scanner->reader);

					while (
						buf_reader_has_next(scanner->reader)
						&& '*' != buf_reader_peek(scanner->reader)
						&& '/' != buf_reader_peek_n(scanner->reader, 1)
					)
					{
						buf_reader_consume(scanner->reader);
					}

					/* consume the trailing end comment chars */
					buf_reader_consume(scanner->reader);
					buf_reader_consume(scanner->reader);
				}
			}

			default:
			{
				return;
			}
		}
	}
}


static token_t* scanner_consume_operator(
	scanner_t* lexer,
	token_type_t type,
	size_t size
)
{
	// TODO: Implement
	return NULL;
}


static token_t* scanner_consume_identifier(const scanner_t* scanner)
{
	const uint32_t startLine = scanner->reader->lineNumber;
	const uint32_t startCol = scanner->reader->columnNumber;

	uint32_t i = 0;
	char buffer[TOKEN_MAX_LENGTH] = { 0 };

	/*
	 * First char cannot be numeric
	 * At this point we assume the next char is not a number, otherwise
	 * we would have expected a call to consume_number instead.
	 */
	buffer[i++] = (char) buf_reader_read(scanner->reader);

	int32_t nextChar = buf_reader_peek(scanner->reader);
	while (
		buf_reader_has_next(scanner->reader)
		&& ('_' == nextChar
			|| char_is_alpha(nextChar)
			|| char_is_digit(nextChar))
		&& TOKEN_MAX_LENGTH > i
	)
	{
		buffer[i++] = (char) buf_reader_read(scanner->reader);
		nextChar = buf_reader_peek(scanner->reader);
	}

	if (TOKEN_MAX_LENGTH <= i)
	{
		return token_new_error(
			scanner->reader->lineNumber,
			scanner->reader->columnNumber,
			"Identifier too long! Identifiers cannot exceed " \
				STRINGIFY(TOKEN_MAX_LENGTH) " characters."
		);
	}

	return token_new(
		get_identifier_type(buffer, i),
		startLine,
		startCol,
		buffer,
		i
	);
}


static token_t* scanner_consume_number(scanner_t* scanner)
{
	// TODO: Implement
	return NULL;
}


static token_t* scanner_consume_string(const scanner_t* scanner)
{
	const uint32_t startLine = scanner->reader->lineNumber;
	const uint32_t startCol = scanner->reader->columnNumber;

	uint32_t i = 0;
	char buffer[TOKEN_STRING_MAX_LENGTH] = { 0 };

	/* Don't include the first character (") in the string */
	buf_reader_consume(scanner->reader);

	int32_t nextChar = buf_reader_peek(scanner->reader);
	bool inEscapeSequence = false;
	while (
		buf_reader_has_next(scanner->reader)
		&& (inEscapeSequence || '"' != nextChar)
		&& TOKEN_STRING_MAX_LENGTH > i
	)
	{
		if (!inEscapeSequence && '\\' == nextChar)
		{
			buf_reader_consume(scanner->reader);
			inEscapeSequence = true;
		}
		else
		{
			inEscapeSequence = false;
			buffer[i++] = (char) buf_reader_read(scanner->reader);
		}

		nextChar = buf_reader_peek(scanner->reader);
	}


	/* Consume the trialing '"' */
	buf_reader_consume(scanner->reader);


	if (TOKEN_STRING_MAX_LENGTH <= i)
	{
		return token_new_error(
			scanner->reader->lineNumber,
			scanner->reader->columnNumber,
			"String too long! Strings cannot exceed " \
				STRINGIFY(TOKEN_STRING_MAX_LENGTH) " characters."
		);
	}

	return token_new(
		TOK_STRING,
		startLine,
		startCol,
		buffer,
		i
	);
}


static token_t* scanner_consume_char(const scanner_t* scanner)
{
	const uint32_t startLine = scanner->reader->lineNumber;
	const uint32_t startCol = scanner->reader->columnNumber;

	/* Don't include the first character (') in the string */
	buf_reader_consume(scanner->reader);

	char buffer[1] = { 0 };
	buffer[0] = (char) buf_reader_read(scanner->reader);

	if ('\'' != buf_reader_peek(scanner->reader))
	{
		return token_new_error(
			scanner->reader->lineNumber,
			scanner->reader->columnNumber,
			"Unexpected character! Expected \"'\"."
		);
	}

	/* Consume the trialing ' */
	buf_reader_consume(scanner->reader);

	return token_new(
		TOK_CHAR,
		startLine,
		startCol,
		buffer,
		1
	);
}


scanner_t* scanner_new(buf_reader_t* reader)
{
	if (NULL == reader)
	{
		return NULL;
	}

	scanner_t* scanner = malloc(sizeof(scanner_t));
	if (NULL == scanner)
	{
		return NULL;
	}

	arr_token_t* tokens = malloc(sizeof(arr_token_t));
	if (NULL == tokens)
	{
		free(scanner);
		return NULL;
	}

	scanner->reader = reader;
	scanner->tokens = tokens;
	arr_token_init(scanner->tokens);

	return scanner;
}


void scanner_destroy(scanner_t* scanner)
{
	if (NULL == scanner)
	{
		return;
	}

	if (NULL != scanner->reader)
	{
		buf_reader_destroy(scanner->reader);
		scanner->reader = NULL;
	}

	if (NULL != scanner->tokens)
	{
		arr_token_destroy(scanner->tokens);
		scanner->tokens = NULL;
	}

	free(scanner);
}


const token_t* scanner_read_next(scanner_t* scanner)
{
#define CONSUME_TOKEN(consumer) \
	do { \
		const token_t* token = consumer; \
		if (NULL == token) \
		{ \
			tula_exit_err_no_mem(); \
			UNREACHABLE_RETURN(NULL); \
		} \
		arr_token_add(scanner->tokens, token); \
		return token; \
	} while (false)

	if (NULL == scanner)
	{
		return NULL;
	}


	/* Consume any whitespace at the start */
	scanner_skip_whitespace(scanner);


	/* Make sure there is still content left */
	if (!buf_reader_has_next(scanner->reader))
	{
		CONSUME_TOKEN(token_new_eos(
			scanner->reader->lineNumber,
			scanner->reader->columnNumber
		));
	}


	/* Determine how to consume the token based on the next character */
	const int32_t nextChar = buf_reader_peek(scanner->reader);

	if ('_' == nextChar || char_is_alpha(nextChar))
	{
		CONSUME_TOKEN(scanner_consume_identifier(scanner));
	}

	if (char_is_digit(nextChar))
	{
		CONSUME_TOKEN(scanner_consume_number(scanner));
	}

	switch (nextChar)
	{
		case '\0':
		{
			CONSUME_TOKEN(token_new_eos(
				scanner->reader->lineNumber,
				scanner->reader->columnNumber
			));
		}

		case '"':
		{
			CONSUME_TOKEN(scanner_consume_string(scanner));
		}

		case '\'':
		{
			CONSUME_TOKEN(scanner_consume_char(scanner));
		}

		case '(':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_PAREN_LEFT, 1));
		}

		case ')':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_PAREN_RIGHT, 1));
		}

		case '[':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_BRACKET_LEFT, 1));
		}

		case ']':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_BRACKET_RIGHT, 1));
		}

		case '{':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_BRACE_LEFT, 1));
		}

		case '}':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_BRACE_RIGHT, 1));
		}

		case '.':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_DOT, 1));
		}

		case ',':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_COMMA, 1));
		}

		case '=':
		{
			if ('=' == buf_reader_peek_n(scanner->reader, 1))
			{
				CONSUME_TOKEN(scanner_consume_operator(
					scanner,
					TOK_EQUAL_EQUAL,
					2
				));
			}

			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_EQUAL, 1));
		}

		case '+':
		{
			if ('+' == buf_reader_peek_n(scanner->reader, 1))
			{
				CONSUME_TOKEN(scanner_consume_operator(
					scanner,
					TOK_PLUS_PLUS,
					2
				));
			}

			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_PLUS, 1));
		}

		case '-':
		{
			if ('-' == buf_reader_peek_n(scanner->reader, 1))
			{
				CONSUME_TOKEN(scanner_consume_operator(
					scanner,
					TOK_MINUS_MINUS,
					2
				));
			}

			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_MINUS, 1));
		}

		case '*':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_STAR, 1));
		}

		case '^':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_CARET, 1));
		}

		case '/':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_SLASH_FWD, 1));
		}

		case '%':
		{
			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_PERCENT, 1));
		}

		case '>':
		{
			if ('=' == buf_reader_peek_n(scanner->reader, 1))
			{
				CONSUME_TOKEN(scanner_consume_operator(
					scanner,
					TOK_GT_EQUAL,
					2
				));
			}

			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_GREATER_THAN, 1));
		}

		case '<':
		{
			if ('=' == buf_reader_peek_n(scanner->reader, 1))
			{
				CONSUME_TOKEN(scanner_consume_operator(
					scanner,
					TOK_LT_EQUAL,
					2
				));
			}

			CONSUME_TOKEN(scanner_consume_operator(scanner, TOK_LESS_THAN, 1));
		}

		case '!':
		{
			if ('=' == buf_reader_peek_n(scanner->reader, 1))
			{
				CONSUME_TOKEN(scanner_consume_operator(
					scanner,
					TOK_EXCLAM_EQUAL,
					2
				));
			}

			break;
		}

		DEFAULT_BREAK
	}

	CONSUME_TOKEN(token_new_error(
		scanner->reader->lineNumber,
		scanner->reader->columnNumber,
		"Unexpected character"
	));

#undef CONSUME_TOKEN
}

void scanner_read_all(scanner_t* scanner)
{
	const token_t* lastToken = scanner_read_next(scanner);

	while (
		NULL != lastToken
		&& TOK_ERROR != lastToken->type
		&& TOK_EOS != lastToken->type
	)
	{
		lastToken = scanner_read_next(scanner);
	}
}