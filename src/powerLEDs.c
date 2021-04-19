
 /** ***************************************************************************
 * @file
 * @brief Power LEDs
 *
 * All the functions needed for the current control of the power LEDs
 *
 * Prefix: PWR
 *
 * There are two different solutions implemented:
 *
 * Solution 3 regulates the current by hardware. All that is needed in the software
 * is a timer to generate a rectangular signal with 16kHz frequency. That signal
 * sets the RS-FlipFlop. The circuit resets this FlipFlop as soon as the voltage
 * over a shunt-resistor is higher than the DAC voltage set. With the level of
 * the DAC voltage, the average LED current can be controlled.
 *
 * Solution 4 regulates the current also via the DAC output. For comparison of this
 * output to the voltage over the shunt-resistor the ACMP is used. Whenever the
 * shunt-resistor voltage is higher than the set DAC voltage an interrupt is set.
 * This interrupt sets the gate of the MOSFET to LOW. Additionally, there is a Timer
 * which sets the MOSFET gate with 16kHz frequency to HIGH.
 *
 *
 * Board:  Starter Kit EFM32-G8XX-STK
 * Device: EFM32G890F128 (Gecko)
 *
 * @author schmiaa1@students.zhaw.ch, bodenma2@students.zhaw.ch
 * @date 14.12.2020
 *****************************************************************************/
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_adc.h"
#include "em_dac.h"
#include "em_acmp.h"
#include "em_timer.h"
#include "em_letimer.h"


#include "powerLEDs.h"

#include "signalLEDs.h"		// used only to measure time of TIMER0_IRQHandler


/******************************************************************************
 * Defines and variables
 *****************************************************************************/

/** Actual set points for values */
int32_t PWR_value[PWR_SOLUTION_COUNT] = { 0, 0, 0, 0, 0 };

#define PWR_current_max			350			///< Max current in mA


/******************************************************************************
 * Solution specific defines and variables
 *****************************************************************************/

/** ports and pins for LED drivers (with number of solution) */


#define PWR_TIM0_PORT      gpioPortD   ///< Port of TIMERs (Red,Green,Blue)
#define PWR_RED_TIM_PIN      1     ///< Pin of TIMER for RED
#define PWR_GREEN_TIM_PIN    2     ///< Pin of TIMER for Green
#define PWR_BLUE_TIM_PIN     3     ///< Pin of TIMER for Blue

#define PWR_LE_TIM0_PORT   gpioPortB   ///< Port of TIMERs for White
#define PWR_WHITE_TIM_PIN    11       ///< Pin of TIMER for White

/** Avoid rounding issues in intermediate calculations
 * by left-shifting input values first,
 * then do the calculations
 * and finally right-shifting the output values.
 * @n PWR_conversion_shift 12 is equivalent to fixed point calculation in Q19.12
 * @warning: Avoid underflow and overflow of intermediate results!
 * @n Range of int32_t = -2'147'483'648 ... 2'147'483'647 */
#define PWR_conversion_shift		12


/******************************************************************************
 * Functions
 *****************************************************************************/

/** ***************************************************************************
 * @brief Set a GPIO pin
 * @param [in] port of GPIO
 * @param [in] pin of GPIO
 *****************************************************************************/
void GPIO_setPin(uint32_t port, uint32_t pin) {
  GPIO->P[port].DOUTSET = 1 << pin;
}

/** ***************************************************************************
 * @brief clear a GPIO pin
 * @param [in] port of GPIO
 * @param [in] pin of GPIO
 *****************************************************************************/
void GPIO_clearPin(uint32_t port, uint32_t pin) {
  GPIO->P[port].DOUTCLR = 1 << pin;
}



/** ***************************************************************************
 * @brief Initialize TIMER0 in PWM mode and activate overflow interrupt.
 * @param [in] value_top = PWM period time
 * @param [in] value_compare_CC0 = PWM active time of channel 0
 * duty_cycle = value_compare_CC0 / value_top
 * @n 3 compare/capture channels are available on this timer.
 *****************************************************************************/
void TIMER0_PWM_init(uint32_t value_top, uint32_t value_compare) {
  CMU_ClockEnable(cmuClock_TIMER0, true); // enable timer clock
  /* load default values for general TIMER configuration (both solutions)*/
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_Init(TIMER0, &timerInit);     // init and start the timer
  TIMER_TopSet(TIMER0, value_top);    // TOP defines PWM period

  // CC inititalization for all channels
  TIMER_InitCC_TypeDef timerInitCC = TIMER_INITCC_DEFAULT;
  /* load values for CC0 compare/capture channel configuration, solution 3 */
  timerInitCC.mode = timerCCModePWM;    // configure as PWM channel

  //CC0 - Red
  TIMER_InitCC(TIMER0, 0, &timerInitCC);  // CC channel 0 is used
  TIMER_CompareSet(TIMER0, 0, value_compare); // CC value defines PWM active time
  /* route output to location #3 and enable output CC0 */
  TIMER0 ->ROUTE |= (TIMER_ROUTE_LOCATION_LOC3 | TIMER_ROUTE_CC0PEN);

  //CC1 - Green
  TIMER_InitCC(TIMER0, 1, &timerInitCC);  // CC channel 0 is used
  TIMER_CompareSet(TIMER0, 1, value_compare); // CC value defines PWM active time
  /* route output to location #3 and enable output CC1 */
  TIMER0 ->ROUTE |= (TIMER_ROUTE_LOCATION_LOC3 | TIMER_ROUTE_CC1PEN);

  //CC2 - Blue
  TIMER_InitCC(TIMER0, 2, &timerInitCC);  // CC channel 0 is used
  TIMER_CompareSet(TIMER0, 2, value_compare); // CC value defines PWM active time
  /* route output to location #3 and enable output CC2 */
  TIMER0 ->ROUTE |= (TIMER_ROUTE_LOCATION_LOC3 | TIMER_ROUTE_CC2PEN);

  NVIC_ClearPendingIRQ(TIMER0_IRQn);    // clear pending timer interrupts
  TIMER_IntEnable(TIMER0, TIMER_IF_OF); // enable timer overflow interrupt
  NVIC_EnableIRQ(TIMER0_IRQn);      // enable timer interrupts

}

void LETIMER0_PWM_init(uint32_t value_top, uint32_t value_compare) {
  CMU_ClockEnable(cmuClock_LETIMER0, true); // enable timer clock
  LETIMER0->COMP0 = value_top;      // define PWM period
  LETIMER0->COMP1 = value_compare;    // define PWM active time
  LETIMER0->REP0 = 1;           // must be nonzero, probably a bug
  LETIMER0->REP1 = 1;           // must be nonzero, probably a bug
  /* enable output 0 and route it to location 3 */
  LETIMER0->ROUTE = LETIMER_ROUTE_OUT0PEN | LETIMER_ROUTE_LOCATION_LOC3;
  /* Load COMP0 (= TOP value) into CNT register on counter underflow
   * and use PWM mode for output 0 */
  LETIMER0->CTRL = LETIMER_CTRL_COMP0TOP | LETIMER_CTRL_UFOA0_PWM;
  LETIMER0->CMD = LETIMER_CMD_START;    // start the timer
}


/** ***************************************************************************
 * @brief Change duty cycle of LETIMER0 in PWM mode.
 * @param [in] value_compare new PWM active time
 *****************************************************************************/
void LETIMER0_PWM_change(uint32_t value_compare) {
  LETIMER0->COMP1 = value_compare;    // Set PWM compare value
}


/** ***************************************************************************
 * @brief Set the set point of the selected power LED driver.
 * @param [in] solution number
 * @param [in] value of set point
 *
 * The LED driver current is also adjusted accordingly.
 *****************************************************************************/
void PWR_set_value(uint32_t solution, int32_t value) {
	if (solution < PWR_SOLUTION_COUNT) {	// solution number in valid range?
		if (value < 0) { value = 0; }
		if (value > PWR_VALUE_MAX) { value = PWR_VALUE_MAX; }
		PWR_value[solution] = value;
		switch (solution) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
//		  DAC0_CH0_write((value*30000)>>PWR_conversion_shift); ///< calculated and set DAC value of solution 3
			break;
		case 4:
//		  DAC0_CH1_write((value*36832)>>PWR_conversion_shift);  ///< calculated and set DAC value of solution 4
			break;
		}
	}
}


/** ***************************************************************************
 * @brief Get the set point of the selected power LED driver.
 * @param [in] solution number
 * @return value of set point
 *****************************************************************************/
uint32_t PWR_get_value(uint32_t solution) {
	uint32_t value = 0;
	if (solution < PWR_SOLUTION_COUNT) {	// solution number in valid range?
		value = PWR_value[solution];
	}
	return value;
}


/** ***************************************************************************
 * @brief Start all the power LED drivers.
 *
 * All the peripherals are initialized and started
 * for each HW/SW solution for the LED driver.
 *****************************************************************************/
void PWR_init(void) {
	CMU_ClockEnable(cmuClock_GPIO, true);	// enable GPIO clock

	//32Mhz Clock -> 500Hz -> 64000, with dutycycle 1/2 (32000)
	TIMER0_PWM_init(64000, 32000);

  //32768 Hz Clock -> 500Hz -> 66, with dutycycle 1/2 (33)
	LETIMER0_PWM_init(66,33);


	//Pulldown Output for Timers
	GPIO_PinModeSet(PWR_TIM0_PORT, PWR_RED_TIM_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(PWR_TIM0_PORT, PWR_GREEN_TIM_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(PWR_TIM0_PORT, PWR_BLUE_TIM_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(PWR_LE_TIM0_PORT, PWR_WHITE_TIM_PIN, gpioModePushPull, 0);

}

/** ***************************************************************************
 * @brief TIMER0 interrupt handler.
 *
 * @n Current measurements, control loops, adjusting PWM active times, etc.
 * is done here for several solutions.
 * @note This interrupt handler is time critical.
 * Make sure that the CPU time used is shorter than the timer period.
 * @n CMSIS commands are used instead of EMLIB functions, because they run faster.
 *****************************************************************************/
void TIMER0_IRQHandler(void) {
	SL_On(SL_0_PORT, SL_0_PIN);				// start for timing measurement

	TIMER0->IFC = TIMER_IFC_OF;				// clear overflow interrupt flag

	SL_Off(SL_0_PORT, SL_0_PIN);				// stop for timing measurement
}


/** ***************************************************************************
 * @brief ACMP interrupt handler
 * @note Be aware of the fact that ACMP0 and ACMP1 share the same
 * interrupt service routine ACMP0_IRQHandler().<br>
 * This implies that there is no ACMP1_IRQHandler().<br>
 * ACMP1 is used for the touchslider and handled by ACMP0_IRQHandler() directly.
 * @n This PWR_ACMP_IRQHandler() handles the ACMP0 interrupt
 * when it is called by ACMP0_IRQHandler() in touchslider.c
 * @n As part of an interrupt handler this is a time critical section.
 *****************************************************************************/
void PWR_ACMP_IRQHandler(void) {
  if (ACMP0->IF & ACMP_IFC_EDGE) {    // edge on ACMP0 detected
    ACMP0->IFC = ACMP_IFC_EDGE;     // clear interrupt flag
  }
}
