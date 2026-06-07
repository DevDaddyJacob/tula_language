#ifndef TULA_ENGINE_SCANNER_SCANNER_H
#define TULA_ENGINE_SCANNER_SCANNER_H

#include "token.h"
#include "common/buffered_reader.h"

typedef struct tula_scanner
{
	/**
	 * \brief	The buffered reader to read characters from
	 */
	buf_reader_t* reader;

	/**
	 * \brief	An array of the tokens read by the scanner
	 */
	arr_token_t* tokens;
} scanner_t;


scanner_t* scanner_new(buf_reader_t* reader);

void scanner_destroy(scanner_t* scanner);

const token_t* scanner_read_next(scanner_t* scanner);

void scanner_read_all(scanner_t* scanner);

#endif /* TULA_ENGINE_SCANNER_SCANNER_H */
