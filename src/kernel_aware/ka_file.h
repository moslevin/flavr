#ifndef __KA_FILE__
#define __KA_FILE__

//---------------------------------------------------------------------------
/*!
 * \brief KA_Command_Open
 *
 * Open a file on behalf of the emulated target
 */
void KA_Command_Open(void);

//---------------------------------------------------------------------------
/*!
 * \brief KA_Command_Close
 *
 * Close a file on behalf of the emulated target
 */
void KA_Command_Close(void);

//---------------------------------------------------------------------------
/*!
 * \brief KA_Command_Read
 *
 * Read data from a file on behalf of an emulated target
 */
void KA_Command_Read(void);

//---------------------------------------------------------------------------
/*!
 * \brief KA_Command_Write
 *
 * Write data to a file on behalf of an emulated target
 */
void KA_Command_Write(void);

//---------------------------------------------------------------------------
/*!
 * \brief KA_Command_Blocking
 *
 * Make an open file blocking or non-blocking on behalf of an emulated target
 */
void KA_Command_Blocking(void);

#endif //__KA_FILE__
