
 /** ***************************************************************************
 * @file
 * @brief Power LEDs
 *
 * All the functions needed for the current control of the power LEDs
 *
 * Prefix: PWR
 *
 * @todo Write the description of the implemented LED driver solutions.
 *
 * Board:  Starter Kit EFM32-G8XX-STK
 * Device: EFM32G890F128 (Gecko)
 *
 * @author Hanspeter Hochreutener (hhrt@zhaw.ch)
 * @date 15.7.2015
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

/** @todo Adapt PORTs and PINs for LED driver HW inputs and outputs. */

/** ports and pins for LED drivers (with number of solution) */
#define PWR_0_PWM_PORT		gpioPortC		///< Port of PWM for solution 0
#define PWR_0_PWM_PIN			4			///< Pin of PWM for solution 0
#define PWR_1_DAC_PORT		gpioPortB		///< Port of DAC for solution 1
#define PWR_1_DAC_PIN			12			///< Pin of DAC for solution 1
#define PWR_2_ADC_PORT		gpioPortD		///< Port of ADC for solution 2
#define PWR_2_ADC_PIN			0			///< Pin of ADC for solution 2
#define PWR_2_FET_PORT		gpioPortD		///< Port of FET for solution 2
#define PWR_2_FET_PIN			1			///< Pin of FET for solution 2
#define PWR_3_START_PORT	gpioPortD		///< Port of START for solution 3
#define PWR_3_START_PIN			3			///< Pin of START for solution 3
#define PWR_3_DAC_PORT		gpioPortB		///< Port of DAC for solution 3
#define PWR_3_DAC_PIN			11			///< Pin of DAC for solution 3
#define PWR_4_FET_PORT		gpioPortD		///< Port of FET for solution 4
#define PWR_4_FET_PIN			2			///< Pin of FET for solution 4
#define PWR_4_ACMP_PORT		gpioPortC		///< Port of ACMP for solution 4
#define PWR_4_ACMP_PIN			5			///< Pin of ACMP for solution 4

/** @todo Define all the solution specific defines. */

#define DAC_MAX_VALUE     4013

/** @todo Define all the solution specific variables.  */


/** Avoid rounding issues in intermediate calculations
 * by left-shifting input values first,
 * then do the calculations
 * and finally right-shifting the output values.
 * @n PWR_conversion_shift 12 is equivalent to fixed point calculation in Q19.12
 * @warning: Avoid underflow and overflow of intermediate results!
 * @n Range of int32_t = -2'147'483'648 ... 2'147'483'647 */
#define PWR_conversion_shift		12

/** Conversion factor from user-interface-value to microcontroller-output.
 * Use integer data types only and right-shifts instead of divisions
 * for faster code execution (divisions are very time consuming).
 * @n output_value = (PWR_value * PWR_conversion_output) >> PWR_conversion_shift */
/** @todo Define solution specific conversion factors. */
const int32_t PWR_conversion_output[PWR_SOLUTION_COUNT] = { 0, 0, 0, 0, 0 };

/** Conversion factor from microcontroller-input user-interface-value.
 * Use integer data types only and right-shifts instead of divisions
 * for faster code execution (divisions are very time consuming).
 * @n PWR_value = (input_value * PWR_conversion_input) >> PWR_conversion_shift */
/** @todo Define solution specific conversion factors. */
const int32_t PWR_conversion_input[PWR_SOLUTION_COUNT] = { 0, 0, 0, 0, 0 };


/******************************************************************************
 * Functions
 *****************************************************************************/

/** ***************************************************************************
 * @brief Initialize DAC0 channel 1
 *
 *****************************************************************************/
void DAC0_init(void) {
  /* DAC_clock_frequency < 1MHz */
    uint32_t ADC_clock_frequency = 1000000; // 1MHz is the upper limit
    CMU_ClockEnable(cmuClock_DAC0, true); // enable DAC clock
    /* load default values for general DAC configuration */
    DAC_Init_TypeDef DACinit = DAC_INIT_DEFAULT;
    /* calculate the prescaler value */
    DACinit.prescale = DAC_PrescaleCalc(ADC_clock_frequency, 0);
    DACinit.reference = dacRef1V25;      // use internal 1.25V reference
    DAC_Init(DAC0, &DACinit);       // write configuration registers
    /* load default values for DAC channel configuration */
    DAC_InitChannel_TypeDef DACinitChannel = DAC_INITCHANNEL_DEFAULT;

    //Channel 1 (Solution 4)
    DAC_InitChannel(DAC0, &DACinitChannel, 1);  // write channel 1 configuration
    DAC_Enable(DAC0, 1, true);        // enable channel 1
    DAC0->CH1DATA = 0;            // output value for channel 1

    //Channel 0 (Solution 3)
    DAC_InitChannel(DAC0, &DACinitChannel, 0);  // write channel 0 configuration
    DAC_Enable(DAC0, 0, true);        // enable channel 0
    DAC0->CH0DATA = 4013;
}

void DAC0_write(uint32_t value_out) {
  DAC0->CH1DATA = value_out;        // output value for channel 1
}


/** ***************************************************************************
 * @brief Initialize TIMER0 in PWM mode and activate overflow interrupt.
 * @param [in] value_top = PWM period time
 * @param [in] value_compare = PWM active time of channel 0
 * duty_cycle = value_compare / value_top
 * @n 3 compare/capture channels are available on this timer.
 *****************************************************************************/
void TIMER0_PWM_init(uint32_t value_top, uint32_t value_compare) {
  CMU_ClockEnable(cmuClock_TIMER0, true); // enable timer clock
  /* load default values for general TIMER configuration */
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_Init(TIMER0, &timerInit);     // init and start the timer
  TIMER_TopSet(TIMER0, value_top);    // TOP defines PWM period
  /* load default values for compare/capture channel configuration */
  TIMER_InitCC_TypeDef timerInitCC = TIMER_INITCC_DEFAULT;
  timerInitCC.mode = timerCCModePWM;    // configure as PWM channel
  TIMER_InitCC(TIMER0, 0, &timerInitCC);  // CC channel 0 is used
  TIMER_CompareSet(TIMER0, 0, value_compare); // CC value defines PWM active time
  /* route output to location #3 and enable output CC0 */
  TIMER0 ->ROUTE |= (TIMER_ROUTE_LOCATION_LOC3 | TIMER_ROUTE_CC0PEN);
  NVIC_ClearPendingIRQ(TIMER0_IRQn);    // clear pending timer interrupts
  TIMER_IntEnable(TIMER0, TIMER_IF_OF); // enable timer overflow interrupt
  NVIC_EnableIRQ(TIMER0_IRQn);      // enable timer interrupts
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
			/** @todo Calculate and set LED driver current for this HW solution. */
			break;
		case 1:
			/** @todo Calculate and set LED driver current for this HW solution. */
			break;
		case 2:
			/** @todo Calculate and set LED driver current for this HW solution. */
			break;
		case 3:
			/** @todo Calculate and set LED driver current for this HW solution. */
			break;
		case 4:
			/** @todo Calculate and set LED driver current for this HW solution. */
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

	/** @todo Initialize the microcontroller peripherals
	 * for each LED driver solution. */


	//32Mhz Clock -> 32kHz with Dutycycle 1/2 -> values 1000 and 500
	TIMER0_PWM_init(1000, 500);
	//DAC Max value = 4013 when 1.25V reference is used
	DAC0_init();

	//Solution 4


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

	/** @todo Execute solution specific control loops,
	 * calculate and set LED driver current for the HW,
	 * if applicable. */


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
	if (ACMP0->IF & ACMP_IFC_EDGE) {		// edge on ACMP0 detected
		ACMP0->IFC = ACMP_IFC_EDGE;			// clear interrupt flag

		/** @todo Only needed, if ACMP0 is used in one solution.
		 * The whole function can be deleted otherwise. */

	}
}

