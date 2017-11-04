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
    \file   debug_sym.h

    \brief  Symbolic debugging support for data and functions.
*/

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
    Debug_t     eType;          //!< Debug symbol type
    uint32_t    u32StartAddr;   //!< Start of the address range held by the symbol
    uint32_t    u32EndAddr;     //!< Last address held by the symbol
    const char *szName;         //!< Name of the debug symbol

    uint64_t    u64TotalRefs;   //!< Total reference count, used in code profiling
    uint64_t    u64EpochRefs;   //!< Current reference count, used in code profiling
} Debug_Symbol_t;

//---------------------------------------------------------------------------
/*!
 * \brief Symbol_Add_Func
 *
 * Add a new function into the emulator's debug symbol table.
 *
 * \param szName_  - Name of the symbol (string)
 * \param u32Addr_ - Start aadress of the function
 * \param u32Len_  - Size of the function (in bytes)
 */
void Symbol_Add_Func( const char *szName_, const uint32_t u32Addr_, const uint32_t u32Len_ );

//---------------------------------------------------------------------------
/*!
 * \brief Symbol_Add_Obj
 *
 * Add a new object into the emulator's debug symbol table.
 *
 * \param szName_  - Name of the symbol (string)
 * \param u32Addr_ - Start aadress of the object
 * \param u32Len_  - Size of the object (in bytes)
 *
 */
void Symbol_Add_Obj( const char *szName_, const uint32_t u32Addr_, const uint32_t u32Len_ );

//---------------------------------------------------------------------------
/*!
 * \brief Symbol_Get_Obj_Count
 *
 * Get the current count of the objects stored in the symbol table
 *
 * \return Number of objects in the symbol table
 */
uint32_t Symbol_Get_Obj_Count( void );

//---------------------------------------------------------------------------
/*!
 * \brief Symbol_Get_Func_Count
 *
 * Get the current count of the functions stored in the symbol table.
 *
 * \return Number of functions in the symbol table
 */
uint32_t Symbol_Get_Func_Count( void );

//---------------------------------------------------------------------------
/*!
 * \brief Symbol_Func_At_Index
 *
 * Return a point to a debug symbol (function) stored in the table at a
 * specific table index.
 *
 * \param u32Index_ - Table index to look up
 * \return Pointer to the symbol retrieved, or NULL if index out-of-range.
 */
Debug_Symbol_t *Symbol_Func_At_Index( uint32_t u32Index_ );

//---------------------------------------------------------------------------
/*!
 * \brief Symbol_Obj_At_Index
 *
 * Return a point to a debug symbol (object) stored in the table at a
 * specific table index.
 *
 * \param u32Index_ - Table index to look up
 * \return Pointer to the symbol retrieved, or NULL if index out-of-range.
 */
Debug_Symbol_t *Symbol_Obj_At_Index( uint32_t u32Index_ );

//---------------------------------------------------------------------------
/*!
 * \brief Symbol_Find_Func_By_Name
 *
 * Search the local debug symbol table for a function specified by name.
 *
 * \param szName_ - Name of the object to look-up
 * \return Pointer to the symbol retrieved, or NULL if index out-of-range.
 *
 */
Debug_Symbol_t *Symbol_Find_Func_By_Name( const char *szName_ );

//---------------------------------------------------------------------------
/*!
 * \brief Symbol_Find_Obj_By_Name
 *
 * Search the local debug symbol table for an object specified by name.
 *
 * \param szName_ - Name of the object to look up
 * \return Pointer to the symbol retrieved, or NULL if index out-of-range.
 */
Debug_Symbol_t *Symbol_Find_Obj_By_Name( const char *szName_ );

#endif
