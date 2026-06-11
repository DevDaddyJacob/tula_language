#include "buffered_reader.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "os.h"

/*
 * ==================================================
 * Macros
 * ==================================================
 */

/* #define XYZ "ABC" */

/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

/**
 * TBD
 */
static size_t buf_reader_refill(buf_reader_t* reader);


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

static size_t buf_reader_refill(buf_reader_t* reader) {
	/* Validate the reader provided */
	if (NULL == reader)
	{
		return 0;
	}


	/* Ensure we are not EOF or in an error state */
	if (reader->stateEOF || reader->stateError)
	{
		return 0;
	}


	/* Preserve remaining characters to the front of the buffer */
	size_t bufRemaining = BUF_READER_BUFFER_SIZE - reader->bufferPosition;
	if (reader->bufferPosition == reader->bufferLength)
	{
		bufRemaining = 0;
	}

	if (0 < bufRemaining) {
		memmove(
			reader->buffer,
			reader->buffer + reader->bufferPosition,
			bufRemaining
		);
	}


	/* Read into the buffer */
	const size_t read = fread(
		reader->buffer + bufRemaining,
		1,
		BUF_READER_BUFFER_SIZE - bufRemaining,
		reader->file
	);

	reader->bufferLength = bufRemaining + read;
	reader->bufferPosition = 0;


	/* Set states */
	if (0 == reader->bufferLength)
	{
		if (feof(reader->file))
		{
			reader->stateEOF = true;
		}
		else
		{
			reader->stateError = true;
		}

		return 0;
	}

	return read;
}

buf_reader_t* buf_reader_new(const char* fileName)
{
	/* Allocate the memory for the struct */
	buf_reader_t* reader = malloc(sizeof(buf_reader_t));
	if (NULL == reader)
	{
		return NULL;
	}


	/* Default the struct values */
	memset(reader->buffer, 0, sizeof(reader->buffer));

	reader->file = NULL;
	reader->bufferLength = 0;
	reader->bufferPosition = 0;
	reader->lineNumber = 0;
	reader->columnNumber = 0;
	reader->stateEOF = false;
	reader->stateError = false;
	reader->fileName = NULL;

	if (NULL != fileName)
	{
		reader->fileName = os_path_normalize(fileName);

		if (NULL == reader->fileName)
		{
			buf_reader_destroy(reader);
			return NULL;
		}
	}

	return reader;
}

void buf_reader_destroy(buf_reader_t* reader)
{
	/* Validate the reader provided */
	if (NULL == reader)
	{
		return;
	}


	/* Close the reader if needed */
	if (buf_reader_is_open(reader))
	{
		buf_reader_close(reader);
	}


	if (NULL != reader->fileName)
	{
		free(reader->fileName);
	}

	free(reader);
}

bool buf_reader_open(buf_reader_t* reader, const char* fileName)
{
	/* Validate the parameters provided */
	if (NULL == reader)
	{
		return false;
	}

	if (NULL == fileName && NULL == reader->fileName)
	{
		return false;
	}


	/* If we are given a file name, add / replace the existing one */
	if (NULL != fileName)
	{
		if (buf_reader_is_open(reader))
		{
			buf_reader_close(reader);
		}

		free(reader->fileName);
		reader->fileName = os_path_normalize(fileName);
	}


	/* Validate the path exists and is a file */
	if (!os_path_exists(reader->fileName))
	{
		reader->stateError = true;
		return false;
	}

	if (!os_path_is_file(reader->fileName))
	{
		reader->stateError = true;
		return false;
	}


	/* Open the file in read mode */
	reader->file = fopen(reader->fileName, "rb");
	if (NULL == reader->file)
	{
		reader->stateError = true;
		return false;
	}

	buf_reader_refill(reader);
	reader->columnNumber = 0;
	reader->lineNumber = 1;

	return true;
}


bool buf_reader_close(buf_reader_t* reader)
{
	/* Validate the parameters provided */
	if (NULL == reader)
	{
		return false;
	}


	/* Close the file handle if needed */
	if (NULL != reader->file)
	{
		fclose(reader->file);
		reader->file = NULL;
	}


	/* Reset fields */
	reader->bufferLength = 0;
	reader->bufferPosition = 0;
	reader->lineNumber = 0;
	reader->columnNumber = 0;
	reader->stateEOF = false;
	reader->stateError = false;


	return true;
}


int32_t buf_reader_read(buf_reader_t* reader)
{
	/* Validate the parameters provided */
	if (NULL == reader)
	{
		return -1;
	}


	if (reader->bufferPosition >= reader->bufferLength) {
		if (!buf_reader_refill(reader))
		{
			return -1;
		}
	}

	const int32_t ch = (unsigned char)reader->buffer[reader->bufferPosition++];

	if ('\n' == ch)
	{
		reader->lineNumber++;
		reader->columnNumber = 1;
	}
	else
	{
		reader->columnNumber++;
	}

	return ch;
}


int32_t buf_reader_peek(buf_reader_t* reader)
{
	if (NULL == reader)
	{
		return -1;
	}


	/* Ensure there is data in the buffer */
	if (reader->bufferPosition >= reader->bufferLength) {
		if (!buf_reader_refill(reader))
		{
			return -1;
		}
	}

	return (unsigned char)reader->buffer[reader->bufferPosition];
}


int32_t buf_reader_peek_n(buf_reader_t* reader, size_t n)
{
	if (NULL == reader)
	{
		return -1;
	}

	if (BUF_READER_BUFFER_SIZE <= n)
	{
		return -1;
	}


	if (reader->bufferPosition + n >= reader->bufferLength)
	{
		if (0 == buf_reader_refill(reader))
		{
			return -1;
		}

		if (n >= reader->bufferLength)
		{
			return -1;
		}
	}

	return (unsigned char)reader->buffer[reader->bufferPosition + n];
}


void buf_reader_consume(buf_reader_t* reader)
{
	/* Validate the parameters provided */
	if (NULL == reader)
	{
		return;
	}

	buf_reader_read(reader);
}

bool buf_reader_is_open(buf_reader_t* reader)
{
	/* Validate the parameters provided */
	if (NULL == reader)
	{
		return false;
	}

	return NULL != reader->file;
}

bool buf_reader_has_next(buf_reader_t* reader)
{
	/* Validate the parameters provided */
	if (NULL == reader)
	{
		return false;
	}

	return !reader->stateEOF;
}

bool buf_reader_has_error(buf_reader_t* reader)
{
	/* Validate the parameters provided */
	if (NULL == reader)
	{
		return false;
	}

	return reader->stateError;
}