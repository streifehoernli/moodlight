
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

#define PWR_DAC_PORT    gpioPortB   ///< Port of DAC for solution 3 & 4
#define PWR_3_DAC_PIN     11      ///< Pin of DAC for solution 3
#define PWR_4_DAC_PIN     12      ///< Pin of DAC for solution 3

#define PWR_ACMP_PORT		gpioPortC		///< Port of ACMP for solution 4
#define PWR_4_ACMP_PIN_NEG		5			///< Pin of ACMP- for solution 4
#define PWR_4_ACMP_PIN_POS    4     ///< Pin of ACMP- for solution 4

#define PWR_TIM0_PORT   gpioPortD   ///< Port of TIMERs for solution 3 & 4
#define PWR_3_TIM0_PIN    1     ///< Pin of TIMER for solution 3
#define PWR_4_TIM0_PIN    2     ///< Pin of TIMER for solution 4

#define DAC_MAX_VALUE_SOL3     1900   ///<Maximum DAC value to regulate current to 350mA for sol. 3
#define DAC_MAX_VALUE_SOL4     2293   ///<Maximum DAC value to regulate current to 350mA for sol. 4


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
 * @brief clear a GPIO pin
 * @param [in] port of GPIO
 *****************************************************************************/
void DAC0_CH0_write(uint32_t value_out) {
  if(value_out > DAC_MAX_VALUE_SOL3) {
      DAC0->CH0DATA = DAC_MAX_VALUE_SOL3;
  } else {
      DAC0->CH0DATA = value_out;        // output value for channel 0
  }
}

void DAC0_CH1_write(uint32_t value_out) {
  if(value_out > DAC_MAX_VALUE_SOL3) {
        DAC0->CH1DATA = DAC_MAX_VALUE_SOL4;
    } else {
        DAC0->CH1DATA = value_out;        // output value for channel 1
    }
}
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

    //Channel 0 (Solution 3)
    DAC_InitChannel(DAC0, &DACinitChannel, 0);  // write channel 0 configuration
    DAC_Enable(DAC0, 0, true);        // enable channel 0
    DAC0->CH0DATA = DAC_MAX_VALUE_SOL3;

    //Channel 1 (Solution 4)
    DAC_InitChannel(DAC0, &DACinitChannel, 1);  // write channel 1 configuration
    DAC_Enable(DAC0, 1, true);        // enable channel 1
    DAC0->CH1DATA = DAC_MAX_VALUE_SOL4;   // output value for channel 1
}




/** ***************************************************************************
 * @brief Initialize TIMER0 in PWM mode and activate overflow interrupt.
 * @param [in] value_top = PWM period time
 * @param [in] value_compare = PWM active time of channel 0
 * duty_cycle = value_compare / value_top
 * @n 3 compare/capture channels are available on this timer.
 *****************************************************************************/
void TIMER0_PWM_init(uint32_t value_top, uint32_t value_compare_CC0) {
  CMU_ClockEnable(cmuClock_TIMER0, true); // enable timer clock
  /* load default values for general TIMER configuration (both solutions)*/
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_Init(TIMER0, &timerInit);     // init and start the timer
  TIMER_TopSet(TIMER0, value_top);    // TOP defines PWM period

  // CC inititalization for both solutions
  TIMER_InitCC_TypeDef timerInitCC = TIMER_INITCC_DEFAULT;
  /* load values for CC0 compare/capture channel configuration, solution 3 */
  timerInitCC.mode = timerCCModePWM;    // configure as PWM channel
  TIMER_InitCC(TIMER0, 0, &timerInitCC);  // CC channel 0 is used
  TIMER_CompareSet(TIMER0, 0, value_compare_CC0); // CC value defines PWM active time
  /* route output to location #3 and enable output CC0 */
  TIMER0 ->ROUTE |= (TIMER_ROUTE_LOCATION_LOC3 | TIMER_ROUTE_CC0PEN);

  NVIC_ClearPendingIRQ(TIMER0_IRQn);    // clear pending timer interrupts
  TIMER_IntEnable(TIMER0, TIMER_IF_OF); // enable timer overflow interrupt
  NVIC_EnableIRQ(TIMER0_IRQn);      // enable timer interrupts

}

/** ***************************************************************************
 * @brief Initialize comparator ACMP0 and activate rising edge interrupt.
 * @n Compare input on channel 5 with scaled portion of VDD
 *****************************************************************************/
void ACMP0_init(void) {
  CMU_ClockEnable(cmuClock_ACMP0, true);  // enable ACMP clock
  /* load default values for general ACMP configuration */
  ACMP_Init_TypeDef ACMPinit = ACMP_INIT_DEFAULT;
  ACMPinit.interruptOnRisingEdge = true;  // detect rising edge only
  ACMPinit.hysteresisLevel = acmpHysteresisLevel1;  // 15mV hysteresis
  ACMPinit.vddLevel = 0;          // comparator level = 0
  ACMP_Init(ACMP0, &ACMPinit);      // write configuration to registers

  /* Compare scaled portion of channel4 with voltage at input channel 5 */
  ACMP_ChannelSet(ACMP0, acmpChannel4, acmpChannel5);
  NVIC_ClearPendingIRQ(ACMP0_IRQn);   // clear pending comparator interrupts
  ACMP_IntEnable(ACMP0, ACMP_IEN_EDGE); // interrupt on edge detection
  NVIC_EnableIRQ(ACMP0_IRQn);       // enable comparator interrupts
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
			/** @todo Calculate and set DAC Value of solution 3. */
		  DAC0_CH0_write((value*30000)>>PWR_conversion_shift);
			break;
		case 4:
			/** @todo Calculate and set DAC Value for solution 4. */
		  DAC0_CH1_write((value*36832)>>PWR_conversion_shift);
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

	//32Mhz Clock -> 32kHz -> value 1000
	//Dutycycle solution 3 = 1/2 -> value 500
	//Dutycycle solution 4 = 1/5 -> value 200
	TIMER0_PWM_init(1500, 750);


	//Pulldown Output for Timers (solution 3 & 4)
	GPIO_PinModeSet(PWR_TIM0_PORT, PWR_3_TIM0_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(PWR_TIM0_PORT, PWR_4_TIM0_PIN, gpioModePushPull, 0);
	//ACMP Init (solution 3 & 4)
	GPIO_PinModeSet(PWR_ACMP_PORT, PWR_4_ACMP_PIN_POS, gpioModeInput, 0);
  GPIO_PinModeSet(PWR_ACMP_PORT, PWR_4_ACMP_PIN_NEG, gpioModeInputPull, 0);

	//DAC Max value = 4013 for 0.375V when 1.25V reference is used for solution 3
	//DAC Max value = 0 for 0.0V when 1.25V reference is used for solution 4
	DAC0_init();

	ACMP0_init();

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
	//set output pin
	if(GPIO_PinOutGet(PWR_TIM0_PORT, PWR_4_TIM0_PIN) == 0) {
	    GPIO_setPin(PWR_TIM0_PORT, PWR_4_TIM0_PIN);
	} else {
	    GPIO_clearPin(PWR_TIM0_PORT, PWR_4_TIM0_PIN);
	}

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
    GPIO->P[PWR_TIM0_PORT].DOUTCLR = 1 << PWR_4_TIM0_PIN;
		ACMP0->IFC = ACMP_IFC_EDGE;			// clear interrupt flag

		/** @todo Only needed, if ACMP0 is used in one solution.
		 * The whole function can be deleted otherwise. */
	}
}


