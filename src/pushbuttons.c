/** ***************************************************************************
 * @file
 * @brief Pushbuttons
 *
 * All the functions needed to handle the pushbuttons
 *
 * @note The pushbutton status can be polled by reading the input.
 * If interrupts are enabled it is also possible to use this as an event.
 *
 * Prefix: PB
 *
 * Board:  Starter Kit EFM32-G8XX-STK
 * Device: EFM32G890F128 (Gecko)
 *
 * @author Hanspeter Hochreutener (hhrt@zhaw.ch)
 * @date 7.5.2015
 *****************************************************************************/ 


#include "em_cmu.h"
#include "em_gpio.h"


#include "pushbuttons.h"


/******************************************************************************
 * Defines
 *****************************************************************************/
#define PB0_PORT		gpioPortB	///< Port of push button 0
#define PB0_PIN			9			///< Pin of push button 0
#define PB1_PORT		gpioPortB	///< Port of push button 1
#define PB1_PIN			10			///< Pin of push button 1


/******************************************************************************
 * Variables
 *****************************************************************************/
volatile bool PB0_IRQflag = false;		///< set with pushbutton 0 interrupt
volatile bool PB1_IRQflag = false;		///< set with pushbutton 1 interrupt


/******************************************************************************
 * Functions
 *****************************************************************************/


/** ***************************************************************************
 * @brief Setup GPIO for the pushbuttons
 * @note Interrupts are configured but not enabled
 *
 * Pending interrupt flags are cleared
 *****************************************************************************/
void PB_Init(void) {
	CMU_ClockEnable(cmuClock_GPIO, true);
	// Configure the GPIOs
	GPIO_PinModeSet(PB0_PORT, PB0_PIN, gpioModeInputPull, 1);
	GPIO_PinModeSet(PB1_PORT, PB1_PIN, gpioModeInputPull, 1);
	// Configure the interrupts
	GPIO_IntConfig(PB0_PORT, PB0_PIN, false, true, true);
	GPIO_IntConfig(PB1_PORT, PB1_PIN, false, true, true);
	// Clear pending interrupts
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
}


/** ***************************************************************************
 * @brief GPIO interrupt handler for pushbutton 0
 *
 * The HW interrupt flag is cleared immediately
 * to be able to return from interrupt handling.
 * @note The PB0_IRQflag is set and can be handled asynchronously.
 * It has to be cleared explicitly after handling.
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void) {
	if (GPIO->IF & (1 << PB0_PIN)) {	// check is IRQ flag is set
		GPIO->IFC = (1 << PB0_PIN);		// clear IRQ flag
		PB0_IRQflag = true;				// pushbutton 0 was pressed
	}
}

/** ***************************************************************************
 * @brief GPIO interrupt handler for pushbutton 1
 *
 * The HW interrupt flag is cleared immediately
 * to be able to return from interrupt handling.
 * @note The PB1_IRQflag is set and can be handled asynchronously.
 * It has to be cleared explicitly after handling.
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void) {
	if (GPIO->IF & (1 << PB1_PIN)) {	// check is IRQ flag is set
		GPIO->IFC = (1 << PB1_PIN);		// clear IRQ flag
		PB1_IRQflag = true;				// pushbutton 1 was pressed
	}
}


