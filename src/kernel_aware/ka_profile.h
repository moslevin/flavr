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
    \file   ka_profile.h

    \brief  Mark3 RTOS Kernel-Aware Profilng
*/

#ifndef __KA_PROFILE_H__
#define __KA_PROFILE_H__

//---------------------------------------------------------------------------
/*!
 * \brief KA_Profile_Init
 *
 * Initialize the kernel-aware profiling code.
 *
 */
void KA_Profile_Init(void);

//---------------------------------------------------------------------------
/*!
 * \brief KA_Command_Profle_Begin
 */
void KA_Command_Profile_Begin(void);

//---------------------------------------------------------------------------
/*!
 * \brief KA_Command_Profile_Start
 */
void KA_Command_Profile_Start(void);

//---------------------------------------------------------------------------
/*!
 * \brief KA_Command_Profile_Stop
 */
void KA_Command_Profile_Stop(void);

//---------------------------------------------------------------------------
/*!
 * \brief KA_Command_Profile_Report
 */
void KA_Command_Profile_Report(void);

#endif
