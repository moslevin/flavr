/****************************************************************************
 *     (     (                      (     |
 *    )\ )  )\ )    (              )\ )   |
 *   (()/( (()/(    )\     (   (  (()/(   | -- [ Funkenstein ] -------------
 *    /(_)) /(_))((((_)(   )\  )\  /(_))  | -- [ Litle ] -------------------
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
  \file  options.c

  \brief Module for managing command-line options.
*/

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

/*!
 * \brief Options_Init
 *
 * Initialize command-line options for the emulator based on argc/argv input.
 *
 * \param argc_ argc, passed in from main
 * \param argv_ argv, passed in from main
 */
void Options_Init( int argc_, char **argv_ );

/*!
 * \brief Options_GetByName
 *
 * Return the parameter value associated with an option attribute.
 *
 * \param szAttribute_ Name of the attribute to look up
 * \return Pointer to the attribute string, or NULL if attribute is invalid,
 *         or parameter has not been set.
 */
const char *Options_GetByName(const char *szAttribute_);

#endif
