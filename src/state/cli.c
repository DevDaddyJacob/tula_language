#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "tula.h"
#include "common/exit.h"
#include "common/strings.h"

/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

typedef struct tula_cli_params {
    int pointer;
    int argc;
    const char** argv;
} cli_params_t;


typedef enum tula_option_type {
    OPTION_UNKNOWN,         /** Not an option */
    OPTION_INVALID,         /** Invalid option */
    OPTION_END_OF_OPTIONS,  /** -- or --end-of-options */
    OPTION_HELP,            /** -h or --help */
    OPTION_VERSION,         /** -v or --version */

#ifdef TULA_EXE_STANDARD
    OPTION_INTERACTIVE,     /** -i or --interactive */
#endif /* TULA_EXE_STANDARD */

#ifdef TULA_EXE_DEBUGGING
    OPTION_INPUT_FILE,		/** -i or --input-file */
    OPTION_TEST_MODE,		/** -m or --mode */
#endif /* TULA_EXE_DEBUGGING */
} option_type_t;



static void print_help_menu();

static bool has_next(const cli_params_t* params);

static const char* peek_argument(const cli_params_t* params);

static const char* consume_argument(cli_params_t* params);

static option_type_t parse_option_type(const char* arg);

static bool consume_next_option(cli_params_t* params, cli_config_t* config);


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

static void print_help_menu()
{
#ifdef TULA_EXE_STANDARD
#	define TOTAL_OPTIONS 4
#endif /* TULA_EXE_STANDARD */

#ifdef TULA_EXE_DEBUGGING
#	define TOTAL_OPTIONS 5
#endif /* TULA_EXE_DEBUGGING */


	// ReSharper disable once CppTooWideScope
	const char* options[TOTAL_OPTIONS][2] ={
#ifdef TULA_EXE_STANDARD
		{
			"-i, --interactive",
			"enters the REPL mode (Read-Evaluate-Print-Loop)",
		},
#endif /* TULA_EXE_STANDARD */

#ifdef TULA_EXE_DEBUGGING
		{
			"-m, --mode <scanner>",
			"defines the testing mode to use",
		},
		{
			"-i, --input-file <file>",
			"the path of the input file to use",
		},
#endif /* TULA_EXE_DEBUGGING */
        {
            "-h, --help",
            "prints this menu",
        },
        {
            "-v, --version",
            "prints the version of the program",
        },
        {
            "--, --end-of-options",
            "indicates the end of the options",
        }
    };

    fprintf(
        stdout,
        "Usage: " TULA_PROGRAM_NAME " [OPTIONS]... "
#ifdef TULA_EXE_STANDARD
        "[FILE]"
#endif
        "\n\nOptions:\n"
    );

    for (int i = 0; i < TOTAL_OPTIONS; i++) {
        fprintf(
            stdout,
            "\t%s\n\t\t%s\n\n",
            options[i][0],
            options[i][1]
        );
    }

#undef TOTAL_OPTIONS
    tula_exit(TULA_EXIT_GOOD);
}


static bool has_next(const cli_params_t* params)
{
    if (params->pointer >= params->argc)
    {
    	return false;
    }

    return true;
}

static const char* peek_argument(const cli_params_t* params)
{
    if (!has_next(params))
    {
    	return NULL;
    }

    return params->argv[params->pointer];
}


static const char* consume_argument(cli_params_t* params)
{
    if (!has_next(params))
    {
    	return NULL;
    }

    return params->argv[params->pointer++];
}


static option_type_t parse_option_type(const char* arg)
{
    if ('-' != arg[0] || 2 > strlen(arg))
    {
    	return OPTION_UNKNOWN;
    }

    switch (arg[1])
    {
        case '-':
    	{
            if (2 == strlen(arg))
            {
            	return OPTION_END_OF_OPTIONS;
            }

            switch (arg[2])
            {
                default:
            	{
                    if (str_equals_partial(arg, "end-of-options", 14, 2))
                    {
                        return OPTION_END_OF_OPTIONS;
                    }

					if (str_equals_partial(arg, "help", 4, 2))
					{
						return OPTION_HELP;
					}

                	if (str_equals_partial(arg, "version", 7, 2))
                	{
						return OPTION_VERSION;
                	}

#ifdef TULA_EXE_DEBUGGING
                	if (str_equals_partial(arg, "mode", 4, 2))
                	{
                		return OPTION_TEST_MODE;
                	}

                	if (str_equals_partial(arg, "input-file", 10, 2))
                	{
                		return OPTION_INPUT_FILE;
                	}
#endif /* TULA_EXE_DEBUGGING */

                	return OPTION_INVALID;
				}
            }
        }

        case 'i':
        {
#ifdef TULA_EXE_STANDARD
        	return OPTION_INTERACTIVE;
#endif /* TULA_EXE_STANDARD */

#ifdef TULA_EXE_DEBUGGING
        	return OPTION_INPUT_FILE;
#endif /* TULA_EXE_DEBUGGING */
        }

#ifdef TULA_EXE_DEBUGGING
    	case 'm':
        {
        	return OPTION_TEST_MODE;
        }
#endif /* TULA_EXE_DEBUGGING */

        case 'h':
		{
			return OPTION_HELP;
		}

        case 'v':
		{
			return OPTION_VERSION;
		}

        default:
		{
			return OPTION_INVALID;
		}

    }
}


static bool consume_next_option(cli_params_t* params, cli_config_t* config)
{
    option_type_t optionType;

    if (!has_next(params))
    {
    	return false;
    }

    switch (optionType = parse_option_type(peek_argument(params)))
    {
        case OPTION_INVALID:
        case OPTION_HELP:
        case OPTION_VERSION:
    	{
            const char* arg = peek_argument(params);
            free(params);
            free(config);

            switch (optionType)
            {
                case OPTION_INVALID:
            	{
                    exit_err_bad_usage_f(
                    	"'%s' is not a recognized option.",
                    	arg
                    );
                }

                case OPTION_HELP:
            	{
                    print_help_menu();
                    break;
                }

                case OPTION_VERSION:
            	{
                    printf(TULA_RELEASE "\n");
                    tula_exit(TULA_EXIT_GOOD);
                }

                UNREACHABLE_DEFAULT(break);
            }

            UNREACHABLE_RETURN(false);
        }

        case OPTION_UNKNOWN:
    	{
            return false;
        }

        case OPTION_END_OF_OPTIONS:
    	{
            consume_argument(params);
            return false;
    	}

#ifdef TULA_EXE_STANDARD
    	case OPTION_INTERACTIVE:
        {
        	consume_argument(params);
        	config->interactive = true;

        	return true;
        }
#endif /* TULA_EXE_STANDARD */

#ifdef TULA_EXE_DEBUGGING
    	case OPTION_INPUT_FILE:
        {
        	consume_argument(params);

        	if (has_next(params))
        	{
        		const char* arg = consume_argument(params);
        		const uint8_t is_quotted = ('"' == arg[0] || '\'' == arg[0])
					&& (
						str_ends_with_char(arg, '"', strlen(arg))
						|| str_ends_with_char(arg, '\'', strlen(arg))
					);

        		const size_t raw_len = strlen(arg);
        		const size_t str_len = is_quotted
					? (raw_len > 2 ? raw_len - 1 : 1)
					: raw_len + 1;


        		/* Allocate memory for the file */
        		config->inputFile = (char*)malloc(sizeof(char) * str_len);
        		if (NULL == config->inputFile)
        		{
        			free(params);
        			free(config);

        			tula_exit_err_no_mem();
        		}


        		/* Copy the memory of the argument to the config */
        		str_copy_safe(
        			config->inputFile,
        			is_quotted ? arg + 1 : arg,
        			str_len
        		);
        	}

        	return true;
        }

    	case OPTION_TEST_MODE:
        {
        	consume_argument(params);

        	const char* arg = consume_argument(params);

#define DEFINE_TEST_MODE_PARSER(identifier, value) \
	if (str_equals(arg, value, sizeof(value) - 1)) \
    { \
		config->testMode = (identifier); \
		return true; \
    }
        	DEFINE_TEST_MODES(DEFINE_TEST_MODE_PARSER)
#undef DEFINE_TEST_MODE_PARSER

        	return true;
        }
#endif /* TULA_EXE_DEBUGGING */
    }

    return true; /* Unreachable? */
}


cli_config_t* cli_parse_args(const int argc, const char** argv)
{
	/* Allocate memory for params & config */
    cli_params_t* params = malloc(sizeof(cli_params_t));
    if (NULL == params)
    {
        tula_exit_err_no_mem();
    }

    cli_config_t* config = malloc(sizeof(cli_config_t));
    if (NULL == config)
    {
        free(params);
        tula_exit_err_no_mem();
    }



    /* Initialize the params */
    params->pointer = 1;
    params->argc = argc;
    params->argv = argv;


	/* Default the config */
#ifdef TULA_EXE_STANDARD
	config->file = NULL;
	config->interactive = false;
#endif /* TULA_EXE_STANDARD */

#ifdef TULA_EXE_DEBUGGING
	config->testMode = TEST_MODE_VERSION;
	config->inputFile = false;
#endif /* TULA_EXE_DEBUGGING */


    /* Consume the options */
    while (consume_next_option(params, config)) { }


#ifdef TULA_EXE_STANDARD
	/* If we still have next args, store it as the file */
    if (has_next(params))
    {
		const char* arg = consume_argument(params);
		const uint8_t is_quotted = ('"' == arg[0] || '\'' == arg[0])
			&& (
				str_ends_with_char(arg, '"', strlen(arg))
				|| str_ends_with_char(arg, '\'', strlen(arg))
			);

		const size_t raw_len = strlen(arg);
		const size_t str_len = is_quotted
			? (raw_len > 2 ? raw_len - 1 : 1)
			: raw_len + 1;


		/* Allocate memory for the file */
		config->file = (char*)malloc(sizeof(char) * str_len);
		if (NULL == config->file)
		{
			free(params);
			free(config);

			tula_exit_err_no_mem();
		}


		/* Copy the memory of the argument to the config */
		str_copy_safe(config->file, is_quotted ? arg + 1 : arg, str_len);
	}
#endif /* TULA_EXE_STANDARD */

    free(params);
	return config;
}


void cli_destroy(cli_config_t* config)
{
    if (config == NULL)
    {
    	return;
    }

#ifdef TULA_EXE_STANDARD
	if (config->file != NULL)
	{
		free(config->file);
		config->file = NULL;
	}
#endif /* TULA_EXE_STANDARD */

#ifdef TULA_EXE_DEBUGGING
	if (config->inputFile != NULL)
	{
		free(config->inputFile);
		config->inputFile = NULL;
	}
#endif /* TULA_EXE_DEBUGGING */

    free(config);
}