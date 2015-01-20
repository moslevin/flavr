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
#ifndef __DEBUG_SYM_H__
#define __DEBUG_SYM_H__

#include <stdint.h>

//---------------------------------------------------------------------------
typedef enum
{
    DBG_OBJ = 0,
    DBG_FUNC,
//--
    DBG_COUNT
} Debug_t;

//---------------------------------------------------------------------------
typedef struct
{
    Debug_t     eType;
    uint32_t    u32StartAddr;
    uint32_t    u32EndAddr;
    const char *szName;
} Debug_Symbol_t;

//---------------------------------------------------------------------------
void Symbol_Add_Func( const char *szName_, const uint32_t u32Addr_, const uint32_t u32Len_ );

//---------------------------------------------------------------------------
void Symbol_Add_Obj( const char *szName_, const uint32_t u32Addr_, const uint32_t u32Len_ );

#endif
