/****************************************************************************
 *     (     (                      (     |
 *    )\ )  )\ )    (              )\ )   |
 *   (()/( (()/(    )\     (   (  (()/(   | -- [ Funkenstein ] -------------
 *    /(_)) /(_))((((_)(   )\  )\  /(_))  | -- [ Litle ] -------------------
 *   (_))_|(_))   )\ _ )\ ((_)((_)(_))    | -- [ AVR ] ---------------------
 *   | |_  | |    (_)_\(_)\ \ / / | _ \   | -- [ Virtual ] -----------------
 *   | __| | |__   / _ \   \ V /  |   /   | -- [ Runtime ] -----------------
 *   |_|   |____| /_/ \_\   \_/   |_|_\   |
 *                                        | "Yeah, it does Arduino..."
 * ---------------------------------------+----------------------------------
 * (c) Copyright 2014, Funkenstein Software Consulting, All rights reserved
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
typedef struct
{
    const char *szAttribute;    //!< Name of the attribute (i.e. what's parsed from the commandline)
    char *szParameter;          //!< Parameter string associated with the option
} Option_t;

//---------------------------------------------------------------------------
typedef enum
{
    OPTION_VARIANT,
    OPTION_FREQ,
    OPTION_RAMSIZE,
    OPTION_ROMSIZE,
    OPTION_EESIZE,
    OPTION_HEXFILE,
//--
    OPTION_NUM
} OptionIndex_t;

//---------------------------------------------------------------------------
static Option_t astAttributes[OPTION_NUM] =
{
    {"--variant", NULL },
    {"--freq", NULL },
    {"--ramsize", NULL },
    {"--romsize", NULL },
    {"--eesize", NULL },
    {"--hexfile", NULL }
};

//---------------------------------------------------------------------------
static void Options_SetDefaults( void )
{
    //!ToDO - Grab these default values from the emu_config.h file.
    astAttributes[ OPTION_VARIANT ].szParameter  = strdup( "atmega328p" );
    astAttributes[ OPTION_FREQ ].szParameter     = strdup( "16000000" );
    astAttributes[ OPTION_RAMSIZE ].szParameter  = strdup( "2048" );
    astAttributes[ OPTION_ROMSIZE ].szParameter  = strdup( "32768" );
    astAttributes[ OPTION_EESIZE ].szParameter   = strdup( "2048" );
    astAttributes[ OPTION_HEXFILE ].szParameter  = strdup( "markade2.hex" );
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
                // Match - ensure the user provided a parameter for this attribute
                if (i + 1 >= argc_)
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
    // fprintf( stderr, "WARN: Invalid option \"%s\"", argv_[i] );

    return 1;
}

//---------------------------------------------------------------------------
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