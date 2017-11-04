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
    \file   ka_graphics.c

    \brief  Mark3 RTOS Kernel-Aware graphics library
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdint.h>
#include <SDL/SDL.h>

#include "kernel_aware.h"
#include "debug_sym.h"
#include "write_callout.h"
#include "interrupt_callout.h"

//---------------------------------------------------------------------------
#define GFX_RES_X       (128)
#define GFX_RES_Y       (160)
#define GFX_SCALE       (3)

//---------------------------------------------------------------------------
typedef struct
{
    uint16_t usX;       //!< X coordinate of the pixel
    uint16_t usY;       //!< Y coordinate of the pixel
    uint32_t uColor;    //!< Color of the pixel in 5:6:5 format
} DrawPoint_t;

//---------------------------------------------------------------------------
static SDL_Surface *pstScreen = 0;

//---------------------------------------------------------------------------
void KA_Graphics_Close(void)
{
    if (pstScreen)
    {
        SDL_FreeSurface(pstScreen);
    }
    SDL_Quit();
}

//---------------------------------------------------------------------------
void KA_Graphics_ClearScreen(void)
{
    memset( pstScreen->pixels, 0, sizeof(uint16_t) * (GFX_RES_X*GFX_SCALE) * (GFX_RES_Y*GFX_SCALE) );
}

//---------------------------------------------------------------------------
void KA_Graphics_DrawPoint(DrawPoint_t *pstPoint_)
{
    uint32_t *pixels = (uint32_t*)pstScreen->pixels;

   // printf( "X:%d Y:%d C=%08X\n", pstPoint_->usX, pstPoint_->usY, pstPoint_->uColor );
    if ((pstPoint_->usX < GFX_RES_X ) && (pstPoint_->usY < GFX_RES_Y))
    {
        int i,j;
        for (i = 0; i < GFX_SCALE; i++)
        {
            for (j = 0; j < GFX_SCALE; j++)
            {
                pixels[ ((uint32_t)((pstPoint_->usY*GFX_SCALE)+i) * (GFX_RES_X*GFX_SCALE) ) +
                         (uint32_t)((pstPoint_->usX*GFX_SCALE)+j) ] = (uint32_t)pstPoint_->uColor;
            }
        }
    }
}

//---------------------------------------------------------------------------
void KA_Graphics_Flip(void)
{
    if (pstScreen)
    {
        SDL_Flip(pstScreen);
    }
}

//---------------------------------------------------------------------------
bool KA_Graphics_Command( uint16_t u16Addr_, uint8_t u8Data_ )
{
    Debug_Symbol_t *pstSymbol = Symbol_Find_Obj_By_Name( "g_pclPoint" );

    switch( u8Data_ )
    {
        case 1:
            if (pstSymbol)
            {
                uint16_t u16PointAddr = *(uint16_t*)(&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ]);
                DrawPoint_t *pstPoint = (DrawPoint_t*)(&stCPU.pstRAM->au8RAM[ u16PointAddr ]);
                KA_Graphics_DrawPoint( pstPoint );
            }
            break;
        case 2:
            KA_Graphics_Flip();
            break;
        case 0:
        default:
            break;
    }

    return true;
}

//---------------------------------------------------------------------------
void KA_Graphics_Init(void)
{
    Debug_Symbol_t *pstSymbol = 0;
    pstSymbol = Symbol_Find_Obj_By_Name( "g_u8GfxCommand" );

    // Use pstSymbol's address to get a pointer to the current thread.
    if (!pstSymbol)
    {
        fprintf(stderr, "Kernel-aware graphics driver not found\n" );
        return;
    }

    // Ensure that we actually have the information we need at a valid address
    uint16_t u16CurrPtr = (uint16_t)(pstSymbol->u32StartAddr & 0x0000FFFF);
    if (!u16CurrPtr)
    {
        fprintf(stderr, "Invalid address for graphics driver global\n" );
        return;
    }

    // Add a callback so that when g_pstCurrent changes, we can update our
    // locally-tracked statistics.
    WriteCallout_Add( KA_Graphics_Command, u16CurrPtr );

    SDL_Init( SDL_INIT_EVERYTHING );
    pstScreen = SDL_SetVideoMode( GFX_RES_X * GFX_SCALE, GFX_RES_Y * GFX_SCALE, 32, SDL_SWSURFACE);
    fprintf(stderr, "Kernel-Aware Graphics Installed\n");

    atexit( KA_Graphics_Close );

}
