/** ***************************************************************************
 * @file
 * @brief Signal LEDs
 *
 * All the functions needed to switch the signal LEDs on the evalboard
 *
 * Prefix: SL
 *
 * Board:  Starter Kit EFM32-G8XX-STK
 * Device: EFM32G890F128 (Gecko)
 *
 * @author Hanspeter Hochreutener (hhrt@zhaw.ch)
 * @date 6.5.2015
 *****************************************************************************/

#include "em_cmu.h"
#include "em_gpio.h"

#include "signalLEDs.h"

/******************************************************************************
 * Defines
 *****************************************************************************/

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

/** ***************************************************************************
 * @brief Setup GPIO for the signal LEDs
 *
 * Enable clocks and configure digital outputs
 *****************************************************************************/
void SL_Init(void) {
	CMU_ClockEnable(cmuClock_HFPER, true);  // enable peripheral clock
	CMU_ClockEnable(cmuClock_GPIO, true);   // enable GPIO clock
	GPIO_PinModeSet(SL_0_PORT, SL_0_PIN, gpioModePushPull, 0); // and signal LED0
	GPIO_PinModeSet(SL_1_PORT, SL_1_PIN, gpioModePushPull, 0); // and signal LED1
	GPIO_PinModeSet(SL_2_PORT, SL_2_PIN, gpioModePushPull, 0); // and signal LED2
	GPIO_PinModeSet(SL_3_PORT, SL_3_PIN, gpioModePushPull, 0); // and signal LED3
}

