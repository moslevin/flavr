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
  \file  tlv_file.h

  \brief Tag-length-value file format used for encoding simulator run-time
         data (kernel-aware plugin data, code profiling statistics, etc.).
*/

#ifndef __TLV_FILE_H__
#define __TLV_FILE_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------
typedef enum
{
    TAG_KERNEL_AWARE_INTERRUPT,                 //!< Kernel-aware plugin generated interrupt events
    TAG_KERNEL_AWARE_CONTEXT_SWITCH,            //!< Kernel-aware plugin generated context switch events
    TAG_KERNEL_AWARE_PRINT,                     //!< Prints generated from kernel-aware debugger
    TAG_KERNEL_AWARE_TRACE_0,                   //!< Kernel trace events
    TAG_KERNEL_AWARE_TRACE_1,                   //!< Kernel trace events, 1 argument
    TAG_KERNEL_AWARE_TRACE_2,                   //!< Kernel trace events, 2 arguments
    TAG_KERNEL_AWARE_PROFILE,                   //!< Kernel-aware profiling events
    TAG_KERNEL_AWARE_THREAD_PROFILE_EPOCH,      //!< Epoch-based thread profiling (i.e. CPU use per thread, per epoch)
    TAG_KERNEL_AWARE_THREAD_PROFILE_GLOBAL,     //!< Global thread profiling (i.e. CPU use per thread, cumulative)
    TAG_CODE_PROFILE_FUNCTION_EPOCH,            //!< CPU Profiling for a given function (per epoch)
    TAG_CODE_PROFILE_FUNCTION_GLOBAL,           //!< CPU Profiling for a given function (cumulative)
    TAG_CODE_COVERAGE_FUNCTION_EPOCH,           //!< Code coverage for a given function (per epoch)
    TAG_CODE_COVERAGE_FUNCTION_GLOBAL,          //!< Code coverage for a given function (cumulative)
    TAG_CODE_COVERAGE_GLOBAL,                   //!< Global code coverage (cumulative)
    TAG_CODE_COVERAGE_ADDRESS,                  //!< Code coverage stats for a given address (cumulative)
//---
    TAG_COUNT
} FlavrTag_t;

//---------------------------------------------------------------------------
typedef struct
{
    FlavrTag_t eTag;        //!< Tag for the object
    uint16_t   u16Len;      //!< Number of bytes that follow in this entry
    uint8_t    au8Data[1];  //!< Data array (1 or more bytes)
} TLV_t;

//---------------------------------------------------------------------------
/*!
 * \brief TLV_WriteInit
 *
 * Initialize the TLV file used to store profiling and diagnostics information
 * in an efficient binary format.  Must be called before logging TLV data.
 *
 * \param szPath_ Name of the TLV output file to create
 */
void TLV_WriteInit( const char *szPath_ );

void TLV_WriteFinish( void );
//---------------------------------------------------------------------------
/*!
 * \brief TLV_Alloc
 *
 * Dynamically allocate an appropriately-sized TLV buffer struct with a
 * large enough data array to store u16Len_ bytes of data.
 *
 * \param u16Len_   Length of the data array to allocate
 * \return          Pointer to a newly-allocated object, or NULL on error
 */
TLV_t *TLV_Alloc( uint16_t u16Len_ );

//---------------------------------------------------------------------------
/*!
 * \brief TLV_Free
 *
 * Free a previously-allocated TLV object.
 *
 * \param pstTLV_ Pointer to a valid, previously-allocated TLV object
 */
void TLV_Free( TLV_t *pstTLV_ );

//---------------------------------------------------------------------------
/*!
 * \brief TLV_Write
 *
 * Write a TLV record to the active file stream.
 *
 * \param pstData_ Pointer to a valid TLV object to log
 * \return -1 on error, number of bytes written on success.
 */
int TLV_Write( TLV_t *pstData_ );

//---------------------------------------------------------------------------
/*!
 * \brief TLV_ReadInit
 *
 * Open the tlv-formatted binary specified in the szPath_ argument, and read
 * its contents into a newly-allocated buffer, which is passed back to the
 * user by the double-pointer pu8Buffer_ argument..
 *
 * \param szPath_       Path to the file to open
 * \param pu8Buffer_    Pointer which will be assigned to the newly-created
 *                      buffer.
 *
 * \return size of the newly-created buffer (in bytes), or 0 on error.
 */
int TLV_ReadInit( const char *szPath_, uint8_t **pu8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief TLV_Read
 *
 * Read an entry from a local copy of the TLV buffer into a user-provided
 * TLV pointer.
 *
 * \param pstTLV_       Pointer to a valid TLV object, with a buffer large
 *                      enough to hold the largest data object we may encounter.
 *
 * \param pu8Buffer_    Pointer to a buffer containing the contents of the
 *                      TLV input file.
 *
 * \param iIndex_       Byte index at whch to start reading TLV data.
 *
 * \return              Number of bytes read into the TLV struct
 */
int TLV_Read( TLV_t *pstTLV_, uint8_t *pu8Buffer_, int iIndex_);

//---------------------------------------------------------------------------
/*!
 * \brief TLV_ReadFinish
 *
 * Dispose of the in-ram copy of the TLV read buffer, allocated from
 * TLV_ReadInit
 *
 * \param pu8Buffer_    Pointer to the previously allocated TLV ram buffer
 */
void TLV_ReadFinish( uint8_t *pu8Buffer_ );

#endif
