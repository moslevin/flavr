/****************************************************************************
 *     (     (                      (     |
 *    )\ )  )\ )    (              )\ )   |
 *   (()/( (()/(    )\     (   (  (()/(   | -- [ Funkenstein ] -------------
 *    /(_)) /(_))((((_)()\  )\  /(_))  | -- [ Litle ] -------------------
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
  \file  avr_registerfile.h

  \brief Module providing an AVR register file
*/

#ifndef __AVR_REGISTERFILE_H__
#define __AVR_REGISTERFILE_H__

//---------------------------------------------------------------------------
#include "avr_coreregs.h"
#include "avr_periphregs.h"

//---------------------------------------------------------------------------
typedef struct
{
    //-- 0x00
    AVR_CoreRegisters CORE_REGISTERS;

    //-- 0x20
    AVR_PIN     PINA;
    AVR_DDR     DDRA;
    AVR_PORT    PORTA;

    //-- 0x23
    AVR_PIN     PINB;
    AVR_DDR     DDRB;
    AVR_PORT    PORTB;

    //-- 0x26
    AVR_PIN     PINC;
    AVR_DDR     DDRC;
    AVR_PORT    PORTC;

    //-- 0x29
    AVR_PIN     PIND;
    AVR_DDR     DDRD;
    AVR_PORT    PORTD;

    //-- 0x2C
    uint8_t     RESERVED_0x2C;
    uint8_t     RESERVED_0x2D;
    uint8_t     RESERVED_0x2E;
    uint8_t     RESERVED_0x2F;
    uint8_t     RESERVED_0x30;
    uint8_t     RESERVED_0x31;
    uint8_t     RESERVED_0x32;
    uint8_t     RESERVED_0x33;
    uint8_t     RESERVED_0x34;

    //-- 0x35
    AVR_TIFR0   TIFR0;
    AVR_TIFR1   TIFR1;
    AVR_TIFR2   TIFR2;

    //-- 0x38
    uint8_t     RESERVED_0x38;
    uint8_t     RESERVED_0x39;
    uint8_t     RESERVED_0x3A;

    //-- 0x3B
    AVR_PCIFR   PCIFR;
    AVR_EIFR    EIFR;
    AVR_EIMSK   EIMSK;

    //-- 0x3E
    uint8_t     GPIOR0;

    //-- 0x3F
    AVR_EECR    EECR;

    //-- 0x40
    uint8_t     EEDR;
    uint8_t     EEARL;
    uint8_t     EEARH;

    //-- 0x43
    AVR_GTCCR   GTCCR;
    AVR_TCCR0A  TCCR0A;
    AVR_TCCR0B  TCCR0B;
    uint8_t     TCNT0;
    uint8_t     OCR0A;
    uint8_t     OCR0B;

    //-- 0x49
    uint8_t     RESERVED_0x49;
    uint8_t     GPIOR1;
    uint8_t     GPIOR2;

    AVR_SPCR    SPCR;
    AVR_SPSR    SPSR;
    uint8_t     SPDR;

    uint8_t     RESERVED_0x4F;
    AVR_ACSR    ACSR;

    uint8_t     RESERVED_0x51;
    uint8_t     RESERVED_0x52;

    //-- 0x53
    AVR_SMCR    SMCR;
    AVR_MCUSR   MCUSR;
    AVR_MCUCR   MCUCR;
    uint8_t     RESERVED_0x56;

    AVR_SPMCSR  SPMCSR;
    uint8_t     RESERVED_0x58;
    uint8_t     RESERVED_0x59;
    uint8_t     RESERVED_0x5A;
    uint8_t     RESERVED_0x5B;
    uint8_t     RESERVED_0x5C;
    AVR_SPL     SPL;
    AVR_SPH     SPH;
    AVR_SREG    SREG;

    //-- 0x60
    AVR_WDTCSR  WDTCSR;
    AVR_CLKPR   CLKPR;
    uint8_t     RESERVED_0x62;
    uint8_t     RESERVED_0x63;
    AVR_PRR     PRR;
    uint8_t     RESERVED_0x65;
    uint8_t     OSCCAL;
    uint8_t     RESERVED_0x67;

    AVR_PCICR   PCICR;
    AVR_EICRA   EICRA;
    uint8_t     RESERVED_0x6A;

    AVR_PCMSK0  PCMSK0;
    AVR_PCMSK1  PCMSK1;
    AVR_PCMSK2  PCMSK2;
    AVR_TIMSK0  TIMSK0;
    AVR_TIMSK1  TIMSK1;
    AVR_TIMSK2  TIMSK2;

    uint8_t     RESERVED_0x71;
    uint8_t     RESERVED_0x72;
    uint8_t     RESERVED_0x73;
    uint8_t     RESERVED_0x74;
    uint8_t     RESERVED_0x75;
    uint8_t     RESERVED_0x76;
    uint8_t     RESERVED_0x77;

    uint8_t     ADCL;
    uint8_t     ADCH;
    AVR_ADCSRA  ADSRA;
    AVR_ADCSRB  ADSRB;
    AVR_ADMUX   ADMXUX;
    uint8_t     RESERVED_0x7F;

    AVR_DIDR0   DIDR0;
    AVR_DIDR1   DIDR1;
    AVR_TCCR1A  TCCR1A;
    AVR_TCCR1B  TCCR1B;
    AVR_TCCR1C  TCCR1C;
    uint8_t     RESERVED_0x83;

    uint8_t     TCNT1L;
    uint8_t     TCNT1H;
    uint8_t     ICR1L;
    uint8_t     ICR1H;
    uint8_t     OCR1AL;
    uint8_t     OCR1AH;
    uint8_t     OCR1BL;
    uint8_t     OCR1BH;

    uint8_t     RESERVED_0x8C;
    uint8_t     RESERVED_0x8D;
    uint8_t     RESERVED_0x8E;
    uint8_t     RESERVED_0x8F;

    uint8_t     RESERVED_0x90;
    uint8_t     RESERVED_0x91;
    uint8_t     RESERVED_0x92;
    uint8_t     RESERVED_0x93;
    uint8_t     RESERVED_0x94;
    uint8_t     RESERVED_0x95;
    uint8_t     RESERVED_0x96;
    uint8_t     RESERVED_0x97;
    uint8_t     RESERVED_0x98;
    uint8_t     RESERVED_0x99;
    uint8_t     RESERVED_0x9A;
    uint8_t     RESERVED_0x9B;
    uint8_t     RESERVED_0x9C;
    uint8_t     RESERVED_0x9D;
    uint8_t     RESERVED_0x9E;
    uint8_t     RESERVED_0x9F;

    uint8_t     RESERVED_0xA0;
    uint8_t     RESERVED_0xA1;
    uint8_t     RESERVED_0xA2;
    uint8_t     RESERVED_0xA3;
    uint8_t     RESERVED_0xA4;
    uint8_t     RESERVED_0xA5;
    uint8_t     RESERVED_0xA6;
    uint8_t     RESERVED_0xA7;
    uint8_t     RESERVED_0xA8;
    uint8_t     RESERVED_0xA9;
    uint8_t     RESERVED_0xAA;
    uint8_t     RESERVED_0xAB;
    uint8_t     RESERVED_0xAC;
    uint8_t     RESERVED_0xAD;
    uint8_t     RESERVED_0xAE;
    uint8_t     RESERVED_0xAF;

    //--0xB0
    AVR_TCCR2A  TCCR2A;
    AVR_TCCR2B  TCCR2B;
    uint8_t     TCNT2;
    uint8_t     OCR2A;
    uint8_t     OCR2B;

    uint8_t     RESERVED_0xB5;
    AVR_ASSR    ASSR;
    uint8_t     RESERVED_0xB7;
    uint8_t     TWBR;
    AVR_TWSR    TWSR;
    AVR_TWAR    TWAR;
    uint8_t     TWDR;
    AVR_TWCR    TWCR;
    AVR_TWAMR   TWAMR;

    uint8_t     RESERVED_0xBE;
    uint8_t     RESERVED_0xBF;

    //--0xC0
    AVR_UCSR0A  UCSR0A;
    AVR_UCSR0B  UCSR0B;
    AVR_UCSR0C  UCSR0C;

    uint8_t     RESERVED_0xC3;

    uint8_t     UBRR0L;
    uint8_t     UBRR0H;
    uint8_t     UDR0;

    uint8_t     RESERVED_0xC7;
    uint8_t     RESERVED_0xC8;
    uint8_t     RESERVED_0xC9;
    uint8_t     RESERVED_0xCA;
    uint8_t     RESERVED_0xCB;
    uint8_t     RESERVED_0xCC;
    uint8_t     RESERVED_0xCD;
    uint8_t     RESERVED_0xCE;
    uint8_t     RESERVED_0xCF;

    uint8_t     RESERVED_0xD0;
    uint8_t     RESERVED_0xD1;
    uint8_t     RESERVED_0xD2;
    uint8_t     RESERVED_0xD3;
    uint8_t     RESERVED_0xD4;
    uint8_t     RESERVED_0xD5;
    uint8_t     RESERVED_0xD6;
    uint8_t     RESERVED_0xD7;
    uint8_t     RESERVED_0xD8;
    uint8_t     RESERVED_0xD9;
    uint8_t     RESERVED_0xDA;
    uint8_t     RESERVED_0xDB;
    uint8_t     RESERVED_0xDC;
    uint8_t     RESERVED_0xDD;
    uint8_t     RESERVED_0xDE;
    uint8_t     RESERVED_0xDF;

    uint8_t     RESERVED_0xE0;
    uint8_t     RESERVED_0xE1;
    uint8_t     RESERVED_0xE2;
    uint8_t     RESERVED_0xE3;
    uint8_t     RESERVED_0xE4;
    uint8_t     RESERVED_0xE5;
    uint8_t     RESERVED_0xE6;
    uint8_t     RESERVED_0xE7;
    uint8_t     RESERVED_0xE8;
    uint8_t     RESERVED_0xE9;
    uint8_t     RESERVED_0xEA;
    uint8_t     RESERVED_0xEB;
    uint8_t     RESERVED_0xEC;
    uint8_t     RESERVED_0xED;
    uint8_t     RESERVED_0xEE;
    uint8_t     RESERVED_0xEF;

    uint8_t     RESERVED_0xF0;
    uint8_t     RESERVED_0xF1;
    uint8_t     RESERVED_0xF2;
    uint8_t     RESERVED_0xF3;
    uint8_t     RESERVED_0xF4;
    uint8_t     RESERVED_0xF5;
    uint8_t     RESERVED_0xF6;
    uint8_t     RESERVED_0xF7;
    uint8_t     RESERVED_0xF8;
    uint8_t     RESERVED_0xF9;
    uint8_t     RESERVED_0xFA;
    uint8_t     RESERVED_0xFB;
    uint8_t     RESERVED_0xFC;
    uint8_t     RESERVED_0xFD;
    uint8_t     RESERVED_0xFE;
    uint8_t     RESERVED_0xFF;

} AVRRegisterFile;


#endif // __AVR_REGISTERFILE_H__
