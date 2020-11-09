/** ***************************************************************************
 * @file
 * @brief See pushbuttons.c
 *****************************************************************************/ 


#ifndef PUSHBUTTONS_H_
#define PUSHBUTTONS_H_

#include "em_gpio.h"


/******************************************************************************
 * Defines
 *****************************************************************************/
 

/******************************************************************************
 * Variables
 *****************************************************************************/
extern volatile bool PB0_IRQflag;		///< set with pushbutton 0 interrupt
extern volatile bool PB1_IRQflag;		///< set with pushbutton 1 interrupt


/******************************************************************************
 * Functions
 *****************************************************************************/


void PB_Init(void);


/** ***************************************************************************
 * @brief Enable GPIO for pushbutton interrupt
 * @note Already pending interrupt flags will also evoke the interrupt handler
 *****************************************************************************/
__STATIC_INLINE void PB_EnableIRQ(void) {
	NVIC_EnableIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}


/** ***************************************************************************
 * @brief Disable GPIO for pushbutton interrupt
 * @note Status of the pushbuttons can still be read
 *
 * Pushbuttons can't interrupt a task or wake the cpu anymore.
 *****************************************************************************/
__STATIC_INLINE void PB_DisableIRQ(void) {
	NVIC_DisableIRQ(GPIO_ODD_IRQn);
	NVIC_DisableIRQ(GPIO_EVEN_IRQn);
}


/** ***************************************************************************
 * @brief Check the current status of the selected pushbutton
 * @param [in] port of pushbutton
 * @param [in] pin of pushbutton
 * @return true = pushbutton is pressed at this moment
 *
 * This works independently of an interrupt, it just reads the input.
 *****************************************************************************/
__STATIC_INLINE bool PB_Status(GPIO_Port_TypeDef port, unsigned int pin) {
	return !GPIO_PinInGet(port,pin);
}


#endif
