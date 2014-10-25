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
 * (c) Copyright 2014, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
  \file  avr_coreregs.h

  \brief Module containing struct definition for the core AVR registers
*/

#ifndef __AVR_COREREG_H__
#define __AVR_COREREG_H__

#include <stdint.h>

/*!
 This is a bit of overkill, but there are reasons why the struct
 is presented as more than just a single array of 32 8-bit uints.

 Here, we create anonymous unions between the following core
 registers representations:
 1) 32, 8-bit registers, as an array (r[0] through r[31])
 2) 16, 16-bit register-pairs, as an array (r_word[0] through r_word[15])
 3) 32, 8-bit registers, as named registers (r0 through r31)
 4) 16, 16-bit register-pairs, as named registers(r1_0, through r31_30)
 5) X, Y and Z registers map to r27_26, r29_28, and r31_30
*/
typedef struct
{
    union
    {
        uint8_t r[32];
        uint16_t r_word[16];
        struct
        {
            uint16_t r1_0;
            uint16_t r3_2;
            uint16_t r5_4;
            uint16_t r7_6;
            uint16_t r9_8;
            uint16_t r11_10;
            uint16_t r13_12;
            uint16_t r15_14;
            uint16_t r17_16;
            uint16_t r19_18;
            uint16_t r21_20;
            uint16_t r23_22;
            uint16_t r25_24;
            uint16_t r27_26;
            uint16_t r29_28;
            uint16_t r31_30;
        };
        struct
        {
            uint8_t r0;
            uint8_t r1;
            uint8_t r2;
            uint8_t r3;
            uint8_t r4;
            uint8_t r5;
            uint8_t r6;
            uint8_t r7;
            uint8_t r8;
            uint8_t r9;
            uint8_t r10;
            uint8_t r11;
            uint8_t r12;
            uint8_t r13;
            uint8_t r14;
            uint8_t r15;
            uint8_t r16;
            uint8_t r17;
            uint8_t r18;
            uint8_t r19;
            uint8_t r20;
            uint8_t r21;
            uint8_t r22;
            uint8_t r23;
            uint8_t r24;
            uint8_t r25;
            union
            {
                uint16_t X;
                struct
                {
                    uint8_t r26;
                    uint8_t r27;
                };
            };
            union
            {
                uint16_t Y;
                struct
                {
                    uint8_t r28;
                    uint8_t r29;
                };
            };
            union
            {
                uint16_t Z;
                struct
                {
                    uint8_t r30;
                    uint8_t r31;
                };
            };
        };
    };
} AVR_CoreRegisters;

#endif
