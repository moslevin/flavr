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
    \file   mega_eeprom.c

    \brief  AVR atmega EEPROM plugin
*/

#include "mega_eeprom.h"

#include "avr_cpu.h"

#include <stdint.h>
#include <stdbool.h>

//---------------------------------------------------------------------------
typedef enum
{
    EEPROM_STATE_IDLE = 0,      //!< EEPROM is idle
    EEPROM_STATE_WRITE_ENABLE,  //!< EEPROM write is enabled (for 4 cycles)
    EEPROM_STATE_READ,          //!< EEPROM is reading a byte
    EEPROM_STATE_WRITE,         //!< EEPROM is writing a byte
    //--
    EEPROM_STATES
} EEPROM_State_t;


//---------------------------------------------------------------------------
typedef enum
{
    EEPROM_MODE_ATOMIC = 0,     //!< Atomic Clear/Write operation
    EEPROM_MODE_ERASE,          //!< Erase only
    EEPROM_MODE_WRITE,          //!< Write only
    //----
    EEPROM_MODES
} EEPROM_Mode_t;

//---------------------------------------------------------------------------
static EEPROM_State_t eState = EEPROM_STATE_IDLE;
static uint32_t       u32CountDown = 0;

//---------------------------------------------------------------------------
static void EEARH_Write( uint8_t u8Addr_ )
{
    stCPU.pstRAM->stRegisters.EEARH.r = (u8Addr_ & 0x03);
}

//---------------------------------------------------------------------------
static void EEARL_Write( uint8_t u8Addr_ )
{
    stCPU.pstRAM->stRegisters.EEARL.r = u8Addr_;
}

//---------------------------------------------------------------------------
static uint16_t EEAR_Read( void )
{
    uint16_t u16Addr;
    u16Addr = ((uint16_t)(stCPU.pstRAM->stRegisters.EEARH.r) << 8) |
              (uint16_t)(stCPU.pstRAM->stRegisters.EEARL.r);
}

//---------------------------------------------------------------------------
static void EEPE_Clear(void)
{
    stCPU.pstRAM->stRegisters.EECR.EEPE = 0;
}

//---------------------------------------------------------------------------
static void EEPE_Set(void)
{
    stCPU.pstRAM->stRegisters.EECR.EEPE = 1;
}

//---------------------------------------------------------------------------
static bool EEPE_Read(void)
{
    return (stCPU.pstRAM->stRegisters.EECR.EEPE == 1);
}

//---------------------------------------------------------------------------
static void EEMPE_Clear(void)
{
    stCPU.pstRAM->stRegisters.EECR.EEMPE = 0;
}

//---------------------------------------------------------------------------
static void EEMPE_Set(void)
{
    stCPU.pstRAM->stRegisters.EECR.EEMPE = 1;
}

//---------------------------------------------------------------------------
static bool EEMPE_Read(void)
{
    return (stCPU.pstRAM->stRegisters.EECR.EEMPE == 1);
}

//---------------------------------------------------------------------------
static void EERIE_Clear(void)
{
    stCPU.pstRAM->stRegisters.EECR.EERIE = 0;
}

//---------------------------------------------------------------------------
static void EERIE_Set(void)
{
    stCPU.pstRAM->stRegisters.EECR.EERIE = 1;
}

//---------------------------------------------------------------------------
static bool EERIE_Read(void)
{
    return (stCPU.pstRAM->stRegisters.EECR.EERIE == 1);
}

//---------------------------------------------------------------------------
static EEPROM_Mode_t EEPM_Read(void)
{
    EEPROM_Mode_t eRet;
    eRet = (EEPROM_Mode_t)(stCPU.pstRAM->stRegisters.EECR.r & (0x30)) >> 4;
    return eRet;
}

//---------------------------------------------------------------------------
static uint8_t EEDR_Read(void)
{
    return stCPU.pstRAM->stRegisters.EEDR.r;
}

//---------------------------------------------------------------------------
static void EEPROM_Read(void *context_, uint8_t ucAddr_, uint8_t *pucValue_ )
{
    return (stCPU.pstRAM->stRegisters.EECR.r & 0x3F);
}

//---------------------------------------------------------------------------
static void EEPROM_Write(void *context_, uint8_t ucAddr_, uint8_t ucValue_ )
{
    // We're only interested in the EECR register.  If we really want to be
    // 100% CPU-accurate, we'd take into account a ton of additionl logic for
    // other peripherals (CPU SPM registers, etc.), but that's a lot of code
    // when pretty much everyone is going to be using the app note or the AVR
    // libc implementation, which is very much "sunny case" code.  In short,
    // this will handle incorrectly-implemented code incorrectly.

    stCPU.pstRAM->stRegisters.EECR.r |= (ucValue & 0x3F);

    switch (eState)
    {
        case EEPROM_STATE_IDLE:
        {
            if ((ucValue_ & 0x01) == 0x01)  // Read
            {
                // When the data is read, the data is available in the next instruction
                // but the CPU is halted for 4 cycles before it's executed.
                eState = EEPROM_STATE_READ;
                u32CountDown = 4;

                stCPU.u16ExtraCycles += u32CountDown;
                stCPU.u64CycleCount += u32CountDown;

                // Read data at EEPROM address to EEPROM data register
                stCPU.pstRAM->stRegisters.EEDR.r = stCPU.pu8EEPROM[ EEAR_Read() ];
            }
            else if ((ucValue_ & 0x04) == 0x04) // Program Enable
            {
                // Must initiate a write within 4 cycles of enabling the EEPROM write bit
                eState = EEPROM_STATE_WRITE_ENABLE;
                u32CountDown = 4;
            }
        }
            break;

        case EEPROM_STATE_WRITE_ENABLE:
        {
            if ((ucValue_ & 0x02) == 0x02) // Value has EEPE
            {
                eState = EEPROM_STATE_WRITE;

                switch ( EEPM_Read() )
                {
                    //!! ToDo - Fix the times to use RC-oscilator times, not CPU-clock times.
                    case EEPROM_MODE_ATOMIC:
                    {
                        stCPU.pu8EEPROM[ EEAR_Read() ] = EEDR_Read();
                        u32CountDown = 48000;
                    }
                        break;
                    case EEPROM_MODE_WRITE:
                    {
                        stCPU.pu8EEPROM[ EEAR_Read() ] &= EEDR_Read();
                        u32CountDown = 25000;
                    }
                        break;
                    case EEPROM_MODE_ERASE:
                    {
                        stCPU.pu8EEPROM[ EEAR_Read() ] = 0xFF;
                        u32CountDown = 25000;
                    }
                        break;
                    default:
                        break;
                }
            }
        }
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------
static void EEPROM_Clock(void *context_)
{

    if (u32CountDown)
    {
        u32CountDown--;
        if (!u32CountDown)
        {
            // We're only interested in the EECR register.
            switch (eState)
            {
                case EEPROM_STATE_WRITE:
                {
                    EEPE_Clear();
                    eState = EEPROM_STATE_IDLE;
                }
                    break;
                case EEPROM_STATE_READ:
                {
                    eState = EEPROM_STATE_IDLE;
                }
                    break;
                case EEPROM_STATE_WRITE_ENABLE:
                {
                    EEMPE_Clear();
                    eState = EEPROM_STATE_IDLE;
                }
                    break;
                default:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------
AVRPeripheral stEEPROM =
{
    EEPROM_Init,
    EEPROM_Read,
    EEPROM_Write,
    EEPROM_Clock,
    0,
    0x3F,
    0x3F
};

