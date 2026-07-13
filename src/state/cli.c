#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "config.h"
#include "common/exit.h"
#include "common/strings.h"

/*
 * ==================================================
 * Macros
 * ==================================================
 */

/*
 * def(
 *		identifier,			<-	The identifier used to define the element
 *		executable,			<-	The executable macro the option is included in
 *		usage,				<-	The usage of the args
 *		description,		<-	The description of the arg
 *	)
 */
#define DEFINE_CLI_ARGS(def)													\
	def(																		\
		OPTION_TEST_MODE,														\
		TULA_EXE_DEBUG_VALUE,													\
		"-m, --mode <scanner|parser>",											\
		"defines the testing mode to use"										\
	)																			\
	def(																		\
		OPTION_INPUT_FILE,														\
		TULA_EXE_DEBUG_VALUE,													\
		"-i, --input-file <file>",												\
		"the path of the input file to use"										\
	)																			\
	def(																		\
		OPTION_INTERACTIVE,														\
		TULA_EXE_FULL_VALUE,													\
		"-i, --interactive",													\
		"enters the REPL mode (Read-Evaluate-Print-Loop)"						\
	)																			\
	def(																		\
		OPTION_HELP,															\
		TULA_EXE_ALL_VALUE,														\
		"-h, --help",															\
		"prints this menu"														\
	)																			\
	def(																		\
		OPTION_VERSION_SHORT,													\
		TULA_EXE_ALL_VALUE,														\
		"-v",																	\
		"prints the language version"											\
	)																			\
	def(																		\
		OPTION_VERSION_FULL,													\
		TULA_EXE_ALL_VALUE,														\
		"--version",															\
		"prints the detailed version"											\
	)																			\
	def(																		\
		OPTION_END_OF_OPTIONS,													\
		TULA_EXE_ALL_VALUE,														\
		"--, --end-of-options",													\
		"indicates the end of the options"										\
	)																			\


#define CLI_ARG_IS_ENABLED(macroPrefix, flag) \
	CLI_ARG_IS_ENABLED_(macroPrefix, flag)

#define CLI_ARG_IS_ENABLED_(macroPrefix, flag) \
	macroPrefix##flag


#define CLI_ARGS_ENUM_DEFINER_0(_0)
#define CLI_ARGS_ENUM_DEFINER_1(identifier) identifier,

#define CLI_ARGS_ENUM_DEFINER(identifier, exe, _2, _3) \
	CLI_ARG_IS_ENABLED(CLI_ARGS_ENUM_DEFINER_, exe)(identifier)


#define CLI_ARGS_HELP_TABLE_DEFINER_0(_0, _1)
#define CLI_ARGS_HELP_TABLE_DEFINER_1(usage, desc) { usage, desc },

#define CLI_ARGS_HELP_TABLE_DEFINER(identifier, exe, usage, desc) \
	CLI_ARG_IS_ENABLED(CLI_ARGS_HELP_TABLE_DEFINER_, exe)(usage, desc)


#define CLI_ARGS_TOTAL_OPTIONS_DEFINER(_0, exe, _2, _3) + exe		/* NOLINT(*-macro-parentheses) */

#define TOTAL_CLI_OPTIONS (0 DEFINE_CLI_ARGS(CLI_ARGS_TOTAL_OPTIONS_DEFINER))


/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

/* ReSharper disable once CppClassNeverUsed */
struct tula_cli_params {
    int pointer;
    int argc;
    const char** argv;
};


typedef enum tula_option_type {
    OPTION_UNKNOWN,         /** Not an option */
	OPTION_INVALID,         /** Invalid option */
	DEFINE_CLI_ARGS(CLI_ARGS_ENUM_DEFINER)
} option_type_t;



static void print_help_menu(void);

static bool has_next(const struct tula_cli_params* params);

static const char* peek_argument(const struct tula_cli_params* params);

static const char* consume_argument(struct tula_cli_params* params);

static option_type_t parse_option_type(const char* arg);

static bool consume_next_option(
	struct tula_cli_params* params,
	struct tula_cli_config* config
);


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
	const char* options[TOTAL_CLI_OPTIONS][2] = {
		DEFINE_CLI_ARGS(CLI_ARGS_HELP_TABLE_DEFINER)
	};
	UNUSED(options);

    fprintf(
        stdout,
        "Usage: " TULA_PROGRAM_NAME " [OPTIONS]... "
#ifdef TULA_EXE_FULL
        "[FILE]"
#endif
        "\n\nOptions:\n"
    );

    for (int i = 0; i < TOTAL_CLI_OPTIONS; i++) {
        fprintf(
            stdout,
            "\t%s\n\t\t%s\n\n",
            options[i][0],
            options[i][1]
        );
    }
}


static bool has_next(const struct tula_cli_params* params)
{
    if (params->pointer >= params->argc)
    {
    	return false;
    }

    return true;
}

static const char* peek_argument(const struct tula_cli_params* params)
{
    if (!has_next(params))
    {
    	return NULL;
    }

    return params->argv[params->pointer];
}


static const char* consume_argument(struct tula_cli_params* params)
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
                		return OPTION_VERSION_FULL;
                	}

#ifdef TULA_EXE_DEBUG
                	if (str_equals_partial(arg, "mode", 4, 2))
                	{
                		return OPTION_TEST_MODE;
                	}

                	if (str_equals_partial(arg, "input-file", 10, 2))
                	{
                		return OPTION_INPUT_FILE;
                	}
#endif /* TULA_EXE_DEBUG */

                	return OPTION_INVALID;
				}
            }
        }

        case 'i':
        {
#ifdef TULA_EXE_FULL
        	return OPTION_INTERACTIVE;
#endif /* TULA_EXE_FULL */

#ifdef TULA_EXE_DEBUG
        	return OPTION_INPUT_FILE;
#endif /* TULA_EXE_DEBUG */
        }

#ifdef TULA_EXE_DEBUG
    	case 'm':
        {
        	return OPTION_TEST_MODE;
        }
#endif /* TULA_EXE_DEBUG */

        case 'h':
		{
			return OPTION_HELP;
		}

        case 'v':
		{
			return OPTION_VERSION_SHORT;
		}

        default:
		{
			return OPTION_INVALID;
		}

    }
}


static bool consume_next_option(
	struct tula_cli_params* params,
	struct tula_cli_config* config
)
{
    if (!has_next(params))
    {
    	return false;
    }

    switch (parse_option_type(peek_argument(params)))
    {
    	case OPTION_UNKNOWN:
    	{
    		return false;
    	}

    	case OPTION_INVALID:
    	{
    		exit_err_bad_usage_f(
				"'%s' is not a recognized option.",
				peek_argument(params)
			);
    	}

    	case OPTION_HELP:
    	{
    		print_help_menu();
    		goto cleanup_exit_good;
    	}

    	case OPTION_VERSION_SHORT:
    	{
    		printf("Tula v" TULA_LANGUAGE_VERSION "\n");
    		goto cleanup_exit_good;
    	}

    	case OPTION_VERSION_FULL:
    	{
    		printf(
    			"Tula v" TULA_RELEASE_VERSION_DETAILED
    				"\n  - Executable Type:\t\t" TO_STRING(TULA_EXECUTABLE_TYPE)
    					" (" TULA_PROGRAM_NAME ")"
    				"\n  - Language Version:\t\t" TULA_LANGUAGE_VERSION
    				"\n  - Release Version:\t\t" TULA_RELEASE_VERSION
    				"\n  - Release Hash:\t\t" TULA_COMMIT_HASH_FULL
    				"\n"
    		);
    		goto cleanup_exit_good;
    	}

        case OPTION_END_OF_OPTIONS:
    	{
            consume_argument(params);
            return false;
    	}

#ifdef TULA_EXE_DEBUG
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
#endif /* TULA_EXE_DEBUG */

#ifdef TULA_EXE_FULL
    	case OPTION_INTERACTIVE:
        {
        	consume_argument(params);
        	config->interactive = true;

        	return true;
        }
#endif /* TULA_EXE_FULL */
    }

    return true; /* Unreachable? */

cleanup_exit_good:
	free(params);
	free(config);
	tula_exit(TULA_EXIT_GOOD);
}


struct tula_cli_config* cli_parse_args(const int argc, const char** argv)
{
	if (1 == argc)
	{
		print_help_menu();
		tula_exit(TULA_EXIT_GOOD);
	}

	/* Allocate memory for params & config */
    struct tula_cli_params* params = malloc(sizeof(struct tula_cli_params));
    if (NULL == params)
    {
        tula_exit_err_no_mem();
    }

    struct tula_cli_config* config = malloc(sizeof(struct tula_cli_config));
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
#ifdef TULA_EXE_FULL
	config->file = NULL;
	config->interactive = false;
#endif /* TULA_EXE_FULL */

#ifdef TULA_EXE_DEBUG
	config->testMode = TEST_MODE_VERSION;
	config->inputFile = false;
#endif /* TULA_EXE_DEBUG */


    /* Consume the options */
    while (consume_next_option(params, config)) { }


#ifdef TULA_EXE_FULL
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
#endif /* TULA_EXE_FULL */

    free(params);
	return config;
}


void cli_destroy(struct tula_cli_config* config)
{
    if (config == NULL)
    {
    	return;
    }

#ifdef TULA_EXE_DEBUG
	if (config->inputFile != NULL)
	{
		free(config->inputFile);
		config->inputFile = NULL;
	}
#endif /* TULA_EXE_DEBUG */

#ifdef TULA_EXE_FULL
	if (config->file != NULL)
	{
		free(config->file);
		config->file = NULL;
	}
#endif /* TULA_EXE_FULL */

    free(config);
}