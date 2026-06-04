#ifndef TULA_COMMON_BUFFERED_READER_H
#define TULA_COMMON_BUFFERED_READER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


#if !defined(BUF_READER_BUFFER_SIZE) && defined(TULA_READER_BUFFER_SIZE)
#define BUF_READER_BUFFER_SIZE TULA_READER_BUFFER_SIZE
#endif

#if !defined(BUF_READER_BUFFER_SIZE) && defined(TULA_MAX_BUFFER_SIZE)
#define BUF_READER_BUFFER_SIZE TULA_MAX_BUFFER_SIZE
#endif

#if !defined(BUF_READER_BUFFER_SIZE)
#define BUF_READER_BUFFER_SIZE 4096
#endif


typedef struct tula_buf_reader
{
	FILE* file;
	char buffer[BUF_READER_BUFFER_SIZE];

	/**
	 * \brief	The number of valid bytes currently in the buffer
	 */
	size_t bufferLength;

	/**
	 * \brief	The index of the next byte to be consumed from the buffer
	 */
	size_t bufferPosition;

	char* fileName;
	uint32_t lineNumber;
	uint32_t columnNumber;

	bool stateEOF;
	bool stateError;
} buf_reader_t;


buf_reader_t* buf_reader_new(const char* fileName);

void buf_reader_destroy(buf_reader_t* reader);

bool buf_reader_open(buf_reader_t* reader, const char* fileName);

bool buf_reader_close(buf_reader_t* reader);

int32_t buf_reader_read(buf_reader_t* reader);

int32_t buf_reader_peek(buf_reader_t* reader);

int32_t buf_reader_peek_n(buf_reader_t* reader, size_t n);

void buf_reader_consume(buf_reader_t* reader);

bool buf_reader_is_open(buf_reader_t* reader);

bool buf_reader_has_next(buf_reader_t* reader);

bool buf_reader_has_error(buf_reader_t* reader);


#endif /* TULA_COMMON_BUFFERED_READER_H */
