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
 * (c) Copyright 2014-15, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
    \file   ka_joystick.c

    \brief  Mark3 RTOS Kernel-Aware graphics library
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL/SDL.h>

#include "ka_joystick.h"
#include "write_callout.h"
#include "debug_sym.h"
#include "avr_cpu.h"

//---------------------------------------------------------------------------
#define FLAVR_JOY_UP		0x01
#define FLAVR_JOY_DOWN		0x02
#define FLAVR_JOY_LEFT		0x04
#define FLAVR_JOY_RIGHT		0x08
#define FLAVR_JOY_FIRE		0x10

//---------------------------------------------------------------------------
static uint8_t u8Val = 0;

//---------------------------------------------------------------------------
static bool KA_Scan_Joystick( uint16_t u16Addr_, uint8_t u8Data_ )
{
    Debug_Symbol_t *pstSymbol = 0;
    pstSymbol = Symbol_Find_Obj_By_Name( "g_ucFlavrJoy" );

    if (!pstSymbol)
    {
        fprintf(stderr, "Invalid joystick scan register\n");
        return true;
    }

    uint16_t u16Addr = (uint16_t)(pstSymbol->u32StartAddr & 0x0000FFFF);

    SDL_Event stEvent;

    while (SDL_PollEvent(&stEvent))
    {
        switch (stEvent.type)
        {
            case SDL_KEYDOWN:
            {
                switch( stEvent.key.keysym.sym )
                {
                    case SDLK_UP:
                        u8Val |= FLAVR_JOY_UP;
                        break;
                    case SDLK_DOWN:
                        u8Val |= FLAVR_JOY_DOWN;
                        break;
                    case SDLK_LEFT:
                        u8Val |= FLAVR_JOY_LEFT;
                        break;
                    case SDLK_RIGHT:
                        u8Val |= FLAVR_JOY_RIGHT;
                        break;
                    case SDLK_a:
                        u8Val |= FLAVR_JOY_FIRE;
                        break;
                    case SDLK_ESCAPE:
                        exit(0);
                        break;
                    default:
                        break;
                }
            }
                break;
            case SDL_KEYUP:
            {
                switch( stEvent.key.keysym.sym )
                {
                    case SDLK_UP:
                        u8Val &= ~FLAVR_JOY_UP;
                        break;
                    case SDLK_DOWN:
                        u8Val &= ~FLAVR_JOY_DOWN;
                        break;
                    case SDLK_LEFT:
                        u8Val &= ~FLAVR_JOY_LEFT;
                        break;
                    case SDLK_RIGHT:
                        u8Val &= ~FLAVR_JOY_RIGHT;
                        break;
                    case SDLK_a:
                        u8Val &= ~FLAVR_JOY_FIRE;
                        break;
                    default:
                        break;
                }
            }
                break;
            default:
                break;
        }
    }

    stCPU.pstRAM->au8RAM[ u16Addr ] = u8Val;

    return true;
}

//---------------------------------------------------------------------------
void KA_Joystick_Init( void )
{
    Debug_Symbol_t *pstSymbol = 0;
    pstSymbol = Symbol_Find_Obj_By_Name( "g_ucFlavrJoyUp" );

    if (!pstSymbol)
    {
        fprintf(stderr, "Kernel-aware joystick driver not found\n" );
        return;
    }

    // Ensure that we actually have the information we need at a valid address
    uint16_t u16CurrPtr = (uint16_t)(pstSymbol->u32StartAddr & 0x0000FFFF);
    if (!u16CurrPtr)
    {
        fprintf(stderr, "Invalid address for joystick driver global\n" );
        return;
    }

    // Add a callback so that when a joystick scan is requested, we parse keyboard input
    WriteCallout_Add( KA_Scan_Joystick, u16CurrPtr );

}
