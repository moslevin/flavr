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
  \file  emu_config.h

  \brief configuration file - used to configure features used by the
         emulator at build-time.
*/

#ifndef __EMU_CONFIG_H__
#define __EMU_CONFIG_H__

#include <stdint.h>
#include <stdbool.h>

#define CONFIG_IO_ADDRESS_BYTES        (256)                       // First bytes of address space are I/O range

/*!
    Jump-tables can be used to optimize the execution of opcodes by building
    CPU instruction decode and execute jump tables at runtime.  Once the
    tables are generated, decode/execute are reduced to a lookup table
    operation, as opposed to a complex series of if/else statements for each
    decode/execute of a 16-bit opcode.

    This comes at a cost, however, as jump-tables require RAM (one function
    pointer for each possible 16-bit value, for each lookup type).

    It's a huge speed boost though, so it is recommended to keep this feature
    enabled unless you're trying to self-host flavr on a low-resource
    microcontroller (or even self-hosting a virtual AVR on an AVR...).
*/
#define FEATURE_USE_JUMPTABLES          (1)

/*!
    Sets the "execution history" buffer to a set number of instructions.  The
    larger the number, the further back in time you can look.  Note that for
    each sample we store a CPU register context, as well as a variety of
    bookkeeping information.  Full contents of RAM are not preserved here,
    however.
*/
#define CONFIG_TRACEBUFFER_SIZE        (1000)

#endif

