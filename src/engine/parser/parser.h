#ifndef TULA_ENGINE_PARSER_PARSER_H
#define TULA_ENGINE_PARSER_PARSER_H

#include <stdbool.h>

#include "ast.h"
#include "engine/scanner/scanner.h"


/**
 * \brief	The parser — the second stage of the engine.
 *
 *			It pulls tokens from a \ref scanner_t and assembles them, guided by
 *			the language grammar, into an Abstract Syntax Tree. Following the
 *			engine's top-down ownership discipline, a parser owns both the
 *			scanner that feeds it and the AST it produces; destroying the parser
 *			destroys both.
 */
typedef struct tula_parser
{
	/**
	 * \brief	The scanner tokens are pulled from (owned).
	 */
	scanner_t* scanner;

	/**
	 * \brief	The root of the produced AST (owned), or NULL until
	 *			\ref parser_parse has run.
	 */
	ast_node_t* ast;

	/**
	 * \brief	The token currently under consideration (borrowed from the
	 *			scanner).
	 */
	const token_t* current;

	/**
	 * \brief	Whether a syntax error has been encountered.
	 */
	bool hadError;
} parser_t;


/**
 * \brief           Allocates a new parser over the given scanner
 * \param[in]       scanner: The scanner to pull tokens from; the parser takes
 *                  ownership of it
 * \return          Returns a pointer to the new parser, or NULL if \p scanner
 *                  is NULL
 */
parser_t* parser_new(scanner_t* scanner);


/**
 * \brief           Releases the parser, its scanner, and its AST
 * \param[in]       parser: Pointer to the parser to free (a NULL is a safe
 *                  no-op)
 */
void parser_destroy(parser_t* parser);


/**
 * \brief           Parses the entire token stream into an AST
 * \param[in]       parser: The parser to run
 * \return          Returns the root AST_BLOCK node, owned by the parser. A
 *                  malformed program is reported as AST_ERROR nodes within the
 *                  tree rather than by failing.
 * \note            Calling this more than once returns the already-built tree.
 */
ast_node_t* parser_parse(parser_t* parser);


#endif /* TULA_ENGINE_PARSER_PARSER_H */
