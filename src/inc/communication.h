/** ***************************************************************************
 * @file
 * @brief See communication.c
 *****************************************************************************/

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

/******************************************************************************
 * Defines
 *****************************************************************************/
#define COM_BUF_SIZE 16		///< buffer size, incl. '\0' for string termination

/** @todo Maybe change the end of string character.
 * It has to be the same as in the remote device.
 * Change it also in the putty terminal on the PC.
 * Change it also in the android app for the smartphone. */
#define COM_END_OF_STRING '\r'		///< end of string character: return
// #define COM_END_OF_STRING '\n'		// end of string character: new line
// #define COM_END_OF_STRING '.'
// end of string character '.' is easier for debugging as it is visible

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/
void COM_Init(void);
void COM_Flush_Buffers(void);
bool COM_RX_Available(void);
void COM_RX_GetData(char * string, uint32_t n);
bool COM_TX_Busy(void);
void COM_TX_PutData(char * string, uint32_t n);

#endif
