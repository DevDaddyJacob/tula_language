#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "../tula.h"
#include "../common/strings.h"

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
    OPTION_INTERACTIVE,     /** -i or --interactive */
} option_type_t;



static void printHelpMenu();

static bool hasNext(const cli_params_t* params);

static const char* peekArgument(const cli_params_t* params);

static const char* consumeArgument(cli_params_t* params);

static option_type_t parseOptionType(const char* arg);

static bool consumeNextOption(cli_params_t* params, cli_config_t* config);


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

static void printHelpMenu()
{
#define TOTAL_OPTIONS 4
	// ReSharper disable once CppTooWideScope
	const char* options[TOTAL_OPTIONS][2] ={
        {
            "-h, --help",
            "prints this menu",
        },
        {
            "-i, --interactive",
            "enters the REPL mode (Read-Evaluate-Print-Loop)",
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
        "Usage: " TULA_PROGRAM_NAME " [OPTIONS]... [FILE]\n\n" \
        "Options:\n"
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


static bool hasNext(const cli_params_t* params)
{
    if (params->pointer >= params->argc)
    {
    	return false;
    }

    return true;
}

static const char* peekArgument(const cli_params_t* params)
{
    if (!hasNext(params))
    {
    	return NULL;
    }

    return params->argv[params->pointer];
}


static const char* consumeArgument(cli_params_t* params)
{
    if (!hasNext(params))
    {
    	return NULL;
    }

    return params->argv[params->pointer++];
}


static option_type_t parseOptionType(const char* arg)
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
                    if (str_partialEquals(arg, "end-of-options", 14, 2))
                    {
                        return OPTION_END_OF_OPTIONS;
                    }

					if (str_partialEquals(arg, "help", 4, 2))
					{
						return OPTION_HELP;
					}

                	if (str_partialEquals(arg, "version", 7, 2))
                	{
						return OPTION_VERSION;
					}

                	return OPTION_INVALID;
				}
            }
        }

        case 'i':
        {
        	return OPTION_INTERACTIVE;
        }

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


static bool consumeNextOption(cli_params_t* params, cli_config_t* config)
{
    option_type_t optionType;

    if (!hasNext(params))
    {
    	return false;
    }

    switch (optionType = parseOptionType(peekArgument(params)))
    {
        case OPTION_INVALID:
        case OPTION_HELP:
        case OPTION_VERSION:
    	{
            const char* arg = peekArgument(params);
            free(params);
            free(config);

            switch (optionType)
            {
                case OPTION_INVALID:
            	{
                    exitErr_badUsageF("'%s' is not a recognized option.", arg);
                }

                case OPTION_HELP:
            	{
                    printHelpMenu();
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
            consumeArgument(params);
            return false;
        }

        case OPTION_INTERACTIVE:
    	{
            consumeArgument(params);
            config->interactive = true;

        	return true;
        }
    }

    return true; /* Unreachable? */
}


cli_config_t* cli_parseArgs(const int argc, const char** argv)
{
	/* Allocate memory for params & config */
    cli_params_t* params = malloc(sizeof(cli_params_t));
    if (NULL == params)
    {
        exitErr_noMem();
    }

    cli_config_t* config = malloc(sizeof(cli_config_t));
    if (NULL == config)
    {
        free(params);
        exitErr_noMem();
    }



    /* Initialize the params */
    params->pointer = 1;
    params->argc = argc;
    params->argv = argv;


    /* Default the config */
    config->file = NULL;
    config->interactive = false;


    /* Consume the options */
    while (consumeNextOption(params, config)) { }


	/* If we still have next args, store it as the file */
    if (hasNext(params))
    {
		const char* arg = consumeArgument(params);
		const uint8_t is_quotted = ('"' == arg[0] || '\'' == arg[0])
			&& (
				str_endsWithChar(arg, '"', strlen(arg))
				|| str_endsWithChar(arg, '\'', strlen(arg))
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

			exitErr_noMem();
		}


		/* Copy the memory of the argument to the config */
		str_copy(config->file, is_quotted ? arg + 1 : arg, str_len);
	}

    free(params);
	return config;
}


void cli_destroy(cli_config_t* config)
{
    if (config == NULL)
    {
    	return;
    }

    if (config->file != NULL)
    {
        free(config->file);
    	config->file = NULL;
    }

    free(config);
}