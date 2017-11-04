/****************************************************************************
 *     (     (                      (     |
 *    )\ )  )\ )    (              )\ )   |
 *   (()/( (()/(    )\     (   (  (()/(   | -- [ Funkenstein ] -------------
 *    /(_)) /(_))((((_)()\  )\  /(_))     | -- [ Litle ] -------------------
 *   (_))_|(_))   )\ _ )\ ((_)((_)(_))    | -- [ AVR ] ---------------------
 *   | |_  | |    (_)_\(_)\ \ / / | _ \   | -- [ Virtual ] -----------------
 *   | __| | |__   / _ \   \ V /  |   /   | -- [ Runtime ] -----------------
 *   |_|   |____| /_/ \_\   \_/   |_|_\   |
 *                                        | "Yeah, it does Arduino..."
 * ---------------------------------------+----------------------------------
 * (c) Copyright 2014-17, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
  \file  options.c

  \brief Module for managing command-line options.
*/

#include "emu_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//---------------------------------------------------------------------------
/*!
    Local data structure used to define a command-line option.
*/
typedef struct
{
    const char *szAttribute;    //!< Name of the attribute (i.e. what's parsed from the commandline)
    const char *szDescription;  //!< Description string, used for printing valid options
    char *szParameter;          //!< Parameter string associated with the option
    bool bStandalone;           //!< Attribute is standalone (no parameter value expected)
} Option_t;

//---------------------------------------------------------------------------
/*!
    Enumerated type specifcying the known command-line options accepted by
    flAVR.
*/
typedef enum
{
    OPTION_VARIANT,
    OPTION_FREQ,
    OPTION_HEXFILE,
    OPTION_ELFFILE,
    OPTION_DEBUG,
    OPTION_GDB,
    OPTION_SILENT,
    OPTION_DISASM,
    OPTION_TRACE,
    OPTION_MARK3,
    OPTION_EXITRESET,
    OPTION_PROFILE,
//-- New options go here ^^^
    OPTION_NUM      //!< Total count of command-line options supported
} OptionIndex_t;

//---------------------------------------------------------------------------
/*!
    Table of available commandline options.  Order must match enumeration
    defined above.
*/
static Option_t astAttributes[OPTION_NUM] =
{
    {"--variant",   "Specify the CPU variant by model name (default - atmega328p)", NULL, false },
    {"--freq",      "Speed (in Hz) of the simulated CPU", NULL, false },
    {"--hexfile",   "Programming file (intel HEX binary). Mutually exclusive with --elffile ", NULL, false },
    {"--elffile",   "Programming file (ELF binary).  Mutually exclusive with --hexfile", NULL, false },
    {"--debug",     "Run simulator in interactive debug mode.  Mutually exclusive with --gdb", NULL, true },
    {"--gdb",       "Run simulator as a GDB remote, on the specified port.", NULL, false },
    {"--silent",    "Start without the flavr-banner print", NULL, true },
    {"--disasm",    "Disassemble programming file to standard output", NULL, true },
    {"--trace",     "Enable tracebuffer support when used in conjunction with --debug", NULL, true },
    {"--mark3",     "Enable Mark3 kernel-aware plugin", NULL, true },
    {"--exitreset", "Exit simulator if a jump-to-zero operation is encountered", NULL, true },
    {"--profile",   "Run with code profile and code coverage enabled", NULL, true },
};

//---------------------------------------------------------------------------
/*!
 * \brief Options_SetDefaults
 *
 * Set certain options to default implicit values, in case none are specific
 * from the commandline.
 *
 */
static void Options_SetDefaults( void )
{
    astAttributes[ OPTION_VARIANT ].szParameter  = strdup( "atmega328p" );
    astAttributes[ OPTION_FREQ ].szParameter     = strdup( "16000000" );
}
//---------------------------------------------------------------------------
const char *Options_GetByName (const char *szAttribute_)
{
    uint16_t j;

    // linear search for the correct option value.
    for (j = 0; j < OPTION_NUM; j++)
    {
        if (0 == strcmp(astAttributes[j].szAttribute, szAttribute_))
        {
            return (const char*)astAttributes[j].szParameter;
        }
    }
    return NULL;
}

//---------------------------------------------------------------------------
/*!
 * \brief Options_ParseElement
 *
 * Parse out the next commandline option, starting with argv[ start_ ].
 * Modifies the values stored in the local astAttributes table.
 *
 * \param start_ Starting index
 * \param argc_  Total number of arguments
 * \param argv_  Command-line argument vector
 * \return The next index to process
 */
static uint16_t Options_ParseElement( int start_, int argc_, char **argv_ )
{
    // Parse out specific option parameter data for a given option attribute
    uint16_t i = start_;
    uint16_t j;    

    while (i < argc_)
    {        
        // linear search for the correct option value.
        for (j = 0; j < OPTION_NUM; j++)
        {
            if (0 == strcmp(astAttributes[j].szAttribute, argv_[i]))
            {
                // Match - is the option stand-alone, or does it take a parameter?
                if (astAttributes[j].bStandalone)
                {
                    // Standalone argument, auto-seed a "1" value for the parameter to
                    // indicate that the option was set on the commandline
                    astAttributes[j].szParameter = strdup("1");
                    return 1;
                }

                // ensure the user provided a parameter for this attribute
                if (i + 1 >= argc_)
                {
                    fprintf( stderr, "Error: Paramter expected for attribute %s", argv_[i] );
                    exit(-1);
                }
                else if (*(char*)argv_[i+1] == '-')
                {
                    fprintf( stderr, "Error: Paramter expected for attribute %s", argv_[i] );
                    exit(-1);
                }
                // Check to see if a parameter has already been set; if so, free the existing value
                if (NULL != astAttributes[j].szParameter)
                {
                    free(astAttributes[j].szParameter );
                }
                // fprintf( stderr, "Match: argv[i]=%s, argv[i+1]=%s\n", argv_[i], argv_[i+1] );
                astAttributes[j].szParameter = strdup(argv_[i+1]);
            }
        }
        // Read attribute + parameter combo, 2 tokens        
        return 2;
    }

    // Unknown option - 1 token
    fprintf( stderr, "WARN: Invalid option \"%s\"", argv_[i] );

    return 1;
}

//---------------------------------------------------------------------------
/*!
 * \brief Options_Parse
 *
 * Parse the commandline optins, seeding the array of known parameters with
 * the values specified by the user on the commandline
 *
 * \param argc_ Number of arguments
 * \param argv_ Argument vector, passed from main().
 */
static void Options_Parse(int argc_, char **argv_ )
{
    uint16_t i = 1;
    while (i < argc_)
    {
        // Parse out token from the command line array.
        i += Options_ParseElement( i, argc_, argv_ );
    }
}

//---------------------------------------------------------------------------
void Options_Init( int argc_, char **argv_ )
{
    Options_SetDefaults();
    Options_Parse( argc_, argv_ );
}

//---------------------------------------------------------------------------
void Options_PrintUsage(void)
{
    int i;
    printf("\n Usage:\n\n"
             "    flavr <options>\n\n Where <options> include:\n");
    for (i = 0; i < OPTION_NUM; i++)
    {
        printf( "  %14s: %s", astAttributes[i].szAttribute, astAttributes[i].szDescription );
        if (!astAttributes[i].bStandalone)
        {
            printf(" (takes an argument)" );
        }
        printf( "\n" );
    }
}
