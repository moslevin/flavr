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
  \file  intel_hex.h

  \brief Module for decoding Intel hex formatted programming files
*/

#ifndef __INTEL_HEX_H__
#define __INTEL_HEX_H__

#include <stdint.h>
#include <stdbool.h>

//---------------------------------------------------------------------------
// Load a hex file into the ROM section of a virtual AVR.
#define MAX_HEX_DATA_BYTES      (255)    // max data bytes per line in a record

//---------------------------------------------------------------------------
// Record types in the HEX specification
#define RECORD_DATA             (0)
#define RECORD_EOF              (1)
#define RECORD_EXTENDED_SEGMENT (2)
#define RECORD_START_SEGMENT    (3)
#define RECORD_EXTENDED_LINEAR  (4)
#define RECORD_START_LINEAR     (5)

//---------------------------------------------------------------------------
#define RECORD_TYPE_MAX         (5)

//---------------------------------------------------------------------------
// For reference, this is the line format for an intel hex record.
// :WWXXYYYYzz.....zzCC
//  Where : = the ":" start code
//  WW = the byte count in the data field
//  XX = the record type
//  YYYY = record address
//  zz = data bytes
//  CC = 2's complement checksum of all fields, excluding start code and checksum

//---------------------------------------------------------------------------
/*!
  Data type used to represent a single Intel Hex Record.
*/
typedef struct
{
    uint8_t  u8ByteCount;       //!< Number of bytes in this record
    uint8_t  u8RecordType;      //!< Record type stored in this record
    uint16_t u16Address;        //!< 16-bit address/offset in this record
    uint8_t  u8Data[MAX_HEX_DATA_BYTES];    //!< Record data bytes
    uint8_t  u8Checksum;        //!< 8-bit Checksum for the record
    uint32_t u32Line;           //!< Current line number in the file
} HEX_Record_t;

//---------------------------------------------------------------------------
/*!
 * \brief HEX_Print_Record
 *
 * Print the contents of a single Intel hex record to standard output.
 *
 * \param stRecord_ Pointer to a valid, initialized hex record
 */
void HEX_Print_Record( HEX_Record_t *stRecord_ );

//---------------------------------------------------------------------------
/*!
 * \brief HEX_Read_Record
 *
 * Read the next Intel Hex file record from an open Intel Hex programming
 * file.
 *
 * \param fd_ [in] Open file handle corresponding to the hex file
 *
 * \param stRecord_ [out] Pointer to a valid hex record struct
 *
 * \return true - hex record read succeeded, false - failure or EOF.
 */
bool HEX_Read_Record( int fd_, HEX_Record_t *stRecord_ );

#endif
