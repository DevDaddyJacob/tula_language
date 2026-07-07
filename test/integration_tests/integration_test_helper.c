#include "integration_test_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/**
 * TBD
 */
/* static void example(); */


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

void dynamic_buffer_init(dynamic_buffer_t* buffer)
{
	if (NULL == buffer)
	{
		return;
	}

	buffer->count = 0;
	buffer->capacity = 0;
	buffer->values = NULL;
}


void dynamic_buffer_destroy(dynamic_buffer_t* buffer)
{
	if (NULL == buffer)
	{
		return;
	}

	if (NULL != buffer->values)
	{
		free(buffer->values);
	}

	free(buffer);
}


void dynamic_buffer_write(
	dynamic_buffer_t* buffer,
	const char* content,
	size_t contentSize
)
{
	if (NULL == buffer || NULL == content)
	{
		return;
	}

	/* Check if we need to expand the array size */
	while (buffer->capacity < buffer->count + contentSize) {
		const size_t oldCapacity = buffer->capacity;

		if (256 > oldCapacity)
		{
			buffer->capacity = 256;
		}
		else
		{
			buffer->capacity = oldCapacity * 2;
		}

		buffer->values = realloc(buffer->values, buffer->capacity);
		if (NULL == buffer->values)
		{
			fprintf(stderr, "Failed to reallocate buffer memory");
			exit(1);
		}
	}

	/* Add the value to the end of the array */
	memcpy(buffer->values + buffer->count, content, contentSize);
	buffer->count += contentSize;
}


void read_file(const char* path, dynamic_buffer_t* dBuffer)
{
	FILE *file = fopen(path, "r");
	if (NULL == file) {
		perror("Error opening file");
		return;
	}


	if (0 != fseek(file, 0, SEEK_END)) {
		perror("Error seeking to end");
		fclose(file);
		return;
	}


	long fileSize = ftell(file);
	if (0 > fileSize) {
		perror("Error telling file size");
		fclose(file);
		return;
	}


	if (0 != fseek(file, 0, SEEK_SET)) {
		perror("Error seeking to start");
		fclose(file);
		return;
	}


	char *buffer = malloc(fileSize + 1);
	if (NULL == buffer) {
		perror("Memory allocation failed");
		fclose(file);
		return;
	}


	size_t bytesRead = fread(buffer, 1, fileSize, file);
	if (bytesRead < fileSize) {
		if (ferror(file)) {
			perror("Error reading file");
			free(buffer);
			fclose(file);
			return;
		}
	}

	buffer[bytesRead] = '\0';
    fclose(file);

	dynamic_buffer_init(dBuffer);
	dynamic_buffer_write(dBuffer, buffer, bytesRead);
	dynamic_buffer_write(dBuffer, "\0", 1);
	free(buffer);
}


void execute_tulad(
	const char* args,
	dynamic_buffer_t* outStdoutBuff,
	int32_t* outStatus
)
{
	/* Prepare the command */
	const char* executable = TULAD_EXE_PATH;
	const size_t executableLength = strlen(executable);
	const size_t commandSize = executableLength + 1 + strlen(args) + 1;
	char* command = calloc(commandSize, sizeof(char));
	if (NULL == command)
	{
		return;
	}

	memcpy(command, executable, executableLength);
	command[executableLength] = ' ';
	memcpy(command + executableLength + 1, args, strlen(args));
	command[commandSize - 1] = '\0';


	/* Initialize the output buffer */
	dynamic_buffer_init(outStdoutBuff);


	/* Run the command */
	FILE* pipe = os_popen(command, "r");
	free(command);

	if (NULL == pipe)
	{
		dynamic_buffer_destroy(outStdoutBuff);
		perror("Failed to run command");
		*outStatus = 1;
		return;
	}


	/* Read the stdout */
	char buff[512];
	while (fgets(buff, 512, pipe) != NULL)
	{
		dynamic_buffer_write(outStdoutBuff, buff, strlen(buff));
	}

	dynamic_buffer_write(outStdoutBuff, "\0", 1);


	/* Get the status */
	*outStatus = os_pclose(pipe);
}
/* static void example(); { return; } */