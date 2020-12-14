/**************************************************************************//**
 * @file
 * @brief Simple asynchronous communication with just a string as second level buffer
 *
 * The code is inspired by AN0045 USART or UART Asynchronous mode
 * and by AN0017 Low Energy UART
 * @n It uses only the HW-buffers in connection with TX- and RX-interrupts.
 * @n Overflow handling is not implemented as processing is faster than the baud rate.
 *
 * @note As the LEUART runs on a low frequency clock,
 * updating registers takes a while.
 * Synchronization with the high frequency domain may be achieved with flags.
 *
 * Prefix: COM
 *
 * Board:  Starter Kit EFM32-G8XX-STK
 * Device: EFM32G890F128 (Gecko)
 *
 * @author schmiaa1@students.zhaw.ch, bodenma2@students.zhaw.ch
 * @date 14.12.2020
 *****************************************************************************/

#include <string.h>

#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_leuart.h"

#include "communication.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
#define COM_LEUART		LEUART0				///< UART for communication
#define COM_CLOCK		cmuClock_LEUART0	///< UART clock
#define COM_LOCATION	LEUART_ROUTE_LOCATION_LOC0	///< TX RX route
#define COM_TX_PORT		gpioPortD			///< Port for TX
#define COM_TX_PIN		4					///< Pin for TX
#define COM_RX_PORT		gpioPortD			///< Port for RX
#define COM_RX_PIN		5					///< Pin for RX

/******************************************************************************
 * Variables
 *****************************************************************************/
// second level buffers and flags
bool COM_RX_Available_Flag = false;			///< received data is available
char COM_RX_Data[COM_BUF_SIZE] = "";		///< buffer for received data
bool COM_TX_Busy_Flag = false;				///< busy with sending
char COM_TX_Data[COM_BUF_SIZE] = "";		///< buffer for data to be sent

// string buffers and indices for TX and RX
char TX_buf[COM_BUF_SIZE] = "";				///< transmit buffer
char RX_buf[COM_BUF_SIZE] = "";				///< receive buffer
uint8_t TX_index = 0;						///< transmit buffer index
uint8_t RX_index = 0;						///< receive buffer index

/******************************************************************************
 * Functions
 *****************************************************************************/

/**************************************************************************//**
 * @brief  Initialize the low energy UART
 *
 * Set the clocks, configure the low energy UART, route TX and RX, enable IRQs
 ******************************************************************************/
void COM_Init(void) {
	CMU_ClockEnable(COM_CLOCK, true);		// Enable LEUART clock

	LEUART_Reset(COM_LEUART);
	/* change fields of leuartInit, if standard values don't fit */
	LEUART_Init_TypeDef leuartInit = LEUART_INIT_DEFAULT;
	LEUART_Init(COM_LEUART, &leuartInit);

	/* Enable TX and RX and route to GPIO pins */
	COM_LEUART->ROUTE = LEUART_ROUTE_TXPEN | LEUART_ROUTE_RXPEN | COM_LOCATION;

	/* Configure the GPIOs */
	CMU_ClockEnable(cmuClock_GPIO, true);	// Enable clock for GPIO module
	GPIO_PinModeSet(COM_TX_PORT, COM_TX_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(COM_RX_PORT, COM_RX_PIN, gpioModeInput, 0);
	// GPIO_PinModeSet(COM_RX_PORT, COM_RX_PIN, gpioModeInputPull, 1); 	// with pullup

	/* Configure interrupts */
	LEUART_IntEnable(COM_LEUART, LEUART_IEN_RXDATAV);	// enable RX interrupt
	NVIC_ClearPendingIRQ(LEUART0_IRQn);
	NVIC_EnableIRQ(LEUART0_IRQn);
}

/**************************************************************************//**
 * @brief Flush all the buffers
 *
 * Could be useful if the parser encounters a syntax error.
 ******************************************************************************/
void COM_Flush_Buffers(void) {
	RX_buf[0] = '\0';
	RX_index = 0;
	COM_RX_Data[0] = '\0';
	COM_RX_Available_Flag = false;
	TX_buf[0] = '\0';
	TX_index = 0;
	COM_TX_Data[0] = '\0';
	COM_TX_Busy_Flag = false;
}

/**************************************************************************//**
 * @brief Check if a new string has been received.
 *
 * @return true = a string has been received and is ready for processing.
 ******************************************************************************/
bool COM_RX_Available(void) {
	return COM_RX_Available_Flag;
}

/**************************************************************************//**
 * @brief Get the received data from the serial interface
 *
 * @param [out] string which has been received
 * @param [in] n = maximum number of chars
 * @note A (subsequent) call to COM_RX_GetData() returns an empty string
 * if no new data has been received in the meantime.
 * @n This can be avoided by checking COM_RX_Available();
 ******************************************************************************/
void COM_RX_GetData(char * string, uint32_t n) {
	if (COM_RX_Available_Flag) {
		strncpy(string, COM_RX_Data, n);
		COM_RX_Available_Flag = false;
	} else {
		string[0] = '\0';
	}
}

/**************************************************************************//**
 * @brief Check if a string is currently being sent.
 *
 * @return true = sending of a string is still under way.
 ******************************************************************************/
bool COM_TX_Busy(void) {
	return COM_TX_Busy_Flag;
}

/**************************************************************************//**
 * @brief Start sending data on serial interface
 *
 * @param [in] string to be sent
 * @param [in] n = maximum number of chars
 *
 * @note A call to COM_TX_PutData() starts sending the new string immediately
 * even if an previous string has not been completely sent.
 * @n This can be avoided by checking COM_TX_Busy();
 ******************************************************************************/
void COM_TX_PutData(char * string, uint32_t n) {
	strncpy(TX_buf, string, n);				// Copy string to TX buffer
	TX_index = 0;							// Start to transmit with first char
	COM_TX_Busy_Flag = true;				// Set the busy flag
	LEUART_IntEnable(COM_LEUART, LEUART_IEN_TXBL);// Enable TX interrupt to start
}

/**************************************************************************//**
 * @brief LEUART0 RX IRQ Handler
 *
 * <b>RX Data Valid</b> is handled as follows:
 * @n Copies one char from the RX-HW to the RX buffer.
 * @n '\\r' and '\\n' are considered as end of string character and
 * COM_Received_Request() is called.
 * @n If the buffer is full, it is also handled as end of string.
 *
 * <b>TX Buffer Level</b> is handled as follows:
 * @n If COM_TX_Busy_Flag is set
 * @n one char at a time is copied from the TX buffer to the TX-HW.
 * @n After sending the string COM_TX_Busy_Flag is cleared
 * and the buffer level interrupt is disabled.
 *****************************************************************************/
void LEUART0_IRQHandler(void) {
	if (COM_LEUART->STATUS & LEUART_STATUS_RXDATAV) {// Check for RX data valid
		char new_char = COM_LEUART->RXDATA;		// Fetch the newly received char
		if ((COM_END_OF_STRING != new_char) && (COM_BUF_SIZE > RX_index)) {	// A "normal" char
			RX_buf[RX_index] = new_char;	// store the received char
			RX_index++;						// and increment the index
		} else {							// End of string is reached
			RX_buf[RX_index] = '\0';		// Put end of string char
			strncpy(COM_RX_Data, RX_buf, COM_BUF_SIZE);	// copy new request
			COM_RX_Available_Flag = true;	// ready for processing
			RX_index = 0;					// Start a new string
		}
	}

	if (COM_LEUART->STATUS & LEUART_STATUS_TXBL) {// Check TX buffer level status
		if (COM_TX_Busy_Flag) {					// For synchronization
			char next_char = TX_buf[TX_index];
			if (('\0' != next_char) && (COM_BUF_SIZE > TX_index)) {	// A "normal" char
				COM_LEUART->TXDATA = (uint8_t) next_char;	// Send the char
				TX_index++;						// and increment the index
			} else {							// End of string is reached
				COM_LEUART->TXDATA = (uint8_t) COM_END_OF_STRING;// Send end of string char
				COM_LEUART->IEN &= ~(LEUART_IEN_TXBL);	// Disable interrupt
				TX_index = 0;					// and start anew
				COM_TX_Busy_Flag = false;// Clear flag to avoid restart of sending
			}
		}
	}
}

