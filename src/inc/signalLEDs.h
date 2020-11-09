/** ***************************************************************************
 * @file
 * @brief See signalLEDs.c
 *****************************************************************************/

#ifndef SIGNALLEDS_H_
#define SIGNALLEDS_H_

#include <stdbool.h>
#include "em_gpio.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
#define SL_0_PORT		gpioPortC	///< Port of signal LED 0
#define SL_0_PIN		0			///< Pin of signal LED 0
#define SL_1_PORT		gpioPortC	///< Port of signal LED 1
#define SL_1_PIN		1			///< Pin of signal LED 1
#define SL_2_PORT		gpioPortC	///< Port of signal LED 2
#define SL_2_PIN		2			///< Pin of signal LED 2
#define SL_3_PORT		gpioPortC	///< Port of signal LED 3
#define SL_3_PIN		3			///< Pin of signal LED 3

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

void SL_Init(void);

/** ***************************************************************************
 * @brief Turn on the selected signal LED
 * @param [in] port of signal LED
 * @param [in] pin of signal LED
 *****************************************************************************/
__STATIC_INLINE void SL_On(GPIO_Port_TypeDef port, unsigned int pin) {
	GPIO_PinOutSet(port, pin);
}

/** ***************************************************************************
 * @brief Turn off the selected signal LED
 * @param [in] port of signal LED
 * @param [in] pin of signal LED
 *****************************************************************************/
__STATIC_INLINE void SL_Off(GPIO_Port_TypeDef port, unsigned int pin) {
	GPIO_PinOutClear(port, pin);
}

/** ***************************************************************************
 * @brief Toggle the selected signal LED
 * @param [in] port of signal LED
 * @param [in] pin of signal LED
 *****************************************************************************/
__STATIC_INLINE void SL_Toggle(GPIO_Port_TypeDef port, unsigned int pin) {
	GPIO_PinOutToggle(port, pin);
}

/** ***************************************************************************
 * @brief Read the status of the selected signal LED
 * @param [in] port of signal LED
 * @param [in] pin of signal LED
 * @param [in] status true = turn signal LED on, false = turn signal LED off
 *****************************************************************************/
__STATIC_INLINE void SL_Set(GPIO_Port_TypeDef port, unsigned int pin,
		bool status) {
	if (status) {
		SL_On(port, pin);
	} else {
		SL_Off(port, pin);
	}
}

/** ***************************************************************************
 * @brief Read the status of the selected signal LED
 * @param [in] port of signal LED
 * @param [in] pin of signal LED
 * @return true  = signal LED is on, false = signal LED is off
 *****************************************************************************/
__STATIC_INLINE bool SL_Get(GPIO_Port_TypeDef port, unsigned int pin) {
	return GPIO_PinOutGet(port, pin);
}

#endif
