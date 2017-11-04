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
    \file   debug_sym.c

    \brief  Symbolic debugging support for data and functions.
*/


#include "debug_sym.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------
static Debug_Symbol_t *pstFuncSymbols = 0;
static uint32_t        u32FuncCount = 0;

static Debug_Symbol_t *pstObjSymbols = 0;
static uint32_t        u32ObjCount = 0;

//---------------------------------------------------------------------------
void Symbol_Add_Func( const char *szName_, const uint32_t u32Addr_, const uint32_t u32Len_ )
{
    pstFuncSymbols = (Debug_Symbol_t*)realloc( pstFuncSymbols, (u32FuncCount + 1) * sizeof(Debug_Symbol_t));
    Debug_Symbol_t *pstNew = &pstFuncSymbols[u32FuncCount];

    pstNew->eType           = DBG_FUNC;
    pstNew->szName          = strdup( szName_ );
    pstNew->u32StartAddr    = u32Addr_;
    pstNew->u32EndAddr      = u32Addr_ + u32Len_ - 1;
    pstNew->u64EpochRefs    = 0;
    pstNew->u64TotalRefs    = 0;
    u32FuncCount++;
}

//---------------------------------------------------------------------------
void Symbol_Add_Obj( const char *szName_, const uint32_t u32Addr_, const uint32_t u32Len_ )
{
    pstObjSymbols = (Debug_Symbol_t*)realloc( pstObjSymbols, (u32ObjCount + 1) * sizeof(Debug_Symbol_t));
    Debug_Symbol_t *pstNew = &pstObjSymbols[u32ObjCount];

    pstNew->eType           = DBG_OBJ;
    pstNew->szName          = strdup( szName_ );
    pstNew->u32StartAddr    = u32Addr_;
    pstNew->u32EndAddr      = u32Addr_ + u32Len_ - 1;

    u32ObjCount++;
}


//---------------------------------------------------------------------------
uint32_t Symbol_Get_Obj_Count( void )
{
    return u32ObjCount;
}

//---------------------------------------------------------------------------
uint32_t Symbol_Get_Func_Count( void )
{
    return u32FuncCount;
}

//---------------------------------------------------------------------------
Debug_Symbol_t *Symbol_Func_At_Index( uint32_t u32Index_ )
{
    if (u32Index_ >= u32FuncCount)
    {
        return 0;
    }
    return &pstFuncSymbols[u32Index_];
}

//---------------------------------------------------------------------------
Debug_Symbol_t *Symbol_Obj_At_Index( uint32_t u32Index_ )
{
    if (u32Index_ >= u32ObjCount)
    {
        return 0;
    }
    return &pstObjSymbols[u32Index_];
}

//---------------------------------------------------------------------------
Debug_Symbol_t *Symbol_Find_Func_By_Name( const char *szName_ )
{
    uint32_t i = 0;
    for (i = 0; i < u32FuncCount; i++)
    {
        if (0 == strcmp(szName_,pstFuncSymbols[i].szName))
        {
            return &pstFuncSymbols[i];
        }
    }
    return 0;
}

//---------------------------------------------------------------------------
Debug_Symbol_t *Symbol_Find_Obj_By_Name( const char *szName_ )
{
    uint32_t i = 0;
    for (i = 0; i < u32ObjCount; i++)
    {
        if (0 == strcmp(szName_,pstObjSymbols[i].szName))
        {
            return &pstObjSymbols[i];
        }
    }
    return 0;
}


