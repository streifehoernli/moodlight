
/** ***************************************************************************
 * @file
 * @brief Code snippets which show how some peripherals
 * of the Gecko microcontroller can be adressed.
 *
 * @warning Do not include this file when building!
 * @n It is meant to be used as a code quarry.
 *
 * @note The peripherals can be set up for many different configurations.
 * @n Details can be found in the <b>reference manual</b> of the microcontroller.
 * @n The <b>EMLIB</b> and the <b>CMSIS</b> documentation list predefined configurations.
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


/** ***************************************************************************
 * @brief Initialize a GPIO
 * @param [in] port of GPIO
 * @param [in] pin of GPIO
 * @see pushbuttons.c for GPIO interrupt example code
 * @see Documentation of the GPIO_Mode_TypeDef in the file em_gpio.h
 * for other possible configurations.
 *****************************************************************************/
void GPIO_init(uint32_t port, uint32_t pin) {
	CMU_ClockEnable(cmuClock_GPIO, true);	// enable GPIO clock
	/* Configure a GPIO as an output with initial value 0 */
	GPIO_PinModeSet(port, pin, gpioModePushPull, 0);
	/* Configure a GPIO as an input */
	GPIO_PinModeSet(port, pin, gpioModeInput, 0);
	/* Disable a GPIO */
	GPIO_PinModeSet(port, pin, gpioModeDisabled, 0);
}


/** ***************************************************************************
 * @brief Set, clear and toggle a GPIO
 * @param [in] port of GPIO
 * @param [in] pin of GPIO
 *****************************************************************************/
void GPIO_set_clear_toggle(uint32_t port, uint32_t pin) {
	GPIO_PinOutSet(port, pin);			// using EMLIB function
	GPIO->P[port].DOUTSET = 1 << pin;	// same but with CMSIS (faster)
	GPIO_PinOutClear(port, pin);		// using EMLIB function
	GPIO->P[port].DOUTCLR = 1 << pin;	// same but with CMSIS (faster)
	GPIO_PinOutToggle(port, pin);		// using EMLIB function
	GPIO->P[port].DOUTTGL = 1 << pin;	// same but with CMSIS (faster)
}


/** ***************************************************************************
 * @brief Initialize LETIMER0 in PWM mode.
 * @param [in] value_top = PWM period time
 * @param [in] value_compare = PWM active time
 * duty_cycle = value_compare / value_top
 *****************************************************************************/
void LETIMER0_PWM_init(uint32_t value_top, uint32_t value_compare) {
	CMU_ClockEnable(cmuClock_LETIMER0, true);	// enable timer clock
	LETIMER0->COMP0 = value_top;			// define PWM period
	LETIMER0->COMP1 = value_compare;		// define PWM active time
	LETIMER0->REP0 = 1;						// must be nonzero, probably a bug
	LETIMER0->REP1 = 1;						// must be nonzero, probably a bug
	/* enable output 0 and route it to location 3 */
	LETIMER0->ROUTE = LETIMER_ROUTE_OUT0PEN | LETIMER_ROUTE_LOCATION_LOC3;
	/* Load COMP0 (= TOP value) into CNT register on counter underflow
	 * and use PWM mode for output 0 */
	LETIMER0->CTRL = LETIMER_CTRL_COMP0TOP | LETIMER_CTRL_UFOA0_PWM;
	LETIMER0->CMD = LETIMER_CMD_START;		// start the timer
}


/** ***************************************************************************
 * @brief Change duty cycle of LETIMER0 in PWM mode.
 * @param [in] value_compare new PWM active time
 *****************************************************************************/
void LETIMER0_PWM_change(uint32_t value_compare) {
	LETIMER0->COMP1 = value_compare;		// Set PWM compare value
}


/** ***************************************************************************
 * @brief Initialize TIMER0 in PWM mode and activate overflow interrupt.
 * @param [in] value_top = PWM period time
 * @param [in] value_compare = PWM active time of channel 0
 * duty_cycle = value_compare / value_top
 * @n 3 compare/capture channels are available on this timer.
 *****************************************************************************/
void TIMER0_PWM_init(uint32_t value_top, uint32_t value_compare) {
	CMU_ClockEnable(cmuClock_TIMER0, true);	// enable timer clock
	/* load default values for general TIMER configuration */
	TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
	TIMER_Init(TIMER0, &timerInit);			// init and start the timer
	TIMER_TopSet(TIMER0, value_top);		// TOP defines PWM period
	/* load default values for compare/capture channel configuration */
	TIMER_InitCC_TypeDef timerInitCC = TIMER_INITCC_DEFAULT;
	timerInitCC.mode = timerCCModePWM;		// configure as PWM channel
	TIMER_InitCC(TIMER0, 0, &timerInitCC);	// CC channel 0 is used
	TIMER_CompareSet(TIMER0, 0, value_compare);	// CC value defines PWM active time
	/* route output to location #3 and enable output CC0 */
	TIMER0 ->ROUTE |= (TIMER_ROUTE_LOCATION_LOC3 | TIMER_ROUTE_CC0PEN);
	NVIC_ClearPendingIRQ(TIMER0_IRQn);		// clear pending timer interrupts
	TIMER_IntEnable(TIMER0, TIMER_IF_OF);	// enable timer overflow interrupt
	NVIC_EnableIRQ(TIMER0_IRQn);			// enable timer interrupts
}


/** ***************************************************************************
 * @brief Change duty cycle of channel 0 of TIMER0 in PWM mode.
 * @param [in] value_compare new PWM active time
 *****************************************************************************/
void TIMER0_PWM_change(uint32_t value_compare) {
	TIMER0->CC[0].CCV = value_compare;		// Set PWM compare value on CC channel 0
}


/** ***************************************************************************
 * @brief TIMER0 interrupt handler.
 * @note This interrupt handler is time critical.
 * @n Make sure that the CPU time used is shorter than the timer period.
 * @n Prefer CMSIS commands over EMLIB functions, because they run faster.
 *****************************************************************************/
void TIMER0_IRQHandler(void) {
	TIMER0->IFC = TIMER_IFC_OF;				// clear overflow interrupt flag
	// Maybe set a GPIO to start timing measurement with an oscilloscope

	// Here goes your code.

	// Maybe clear a GPIO to stop timing measurement with an oscilloscope
}


/** ***************************************************************************
 * @brief Initialize ADC0 for one single conversion on channel 0
 *
 * The ADC can also be configured to run continuously
 * and issue an interrupt each time that a new reading is available.
 *****************************************************************************/
void ADC0_init(void) {
	/* ADC_clock_frequency > (bit_resolution + 1) * sampling_frequency */
	uint32_t ADC_clock_frequency = 1000000;	// 13MHz is the upper limit
	CMU_ClockEnable(cmuClock_ADC0, true);	// enable ADC clock
	/* load default values for general ADC configuration */
	ADC_Init_TypeDef ADCinit = ADC_INIT_DEFAULT;
	ADCinit.warmUpMode = adcWarmupKeepADCWarm;	// mode for ADC reference
	ADCinit.lpfMode = adcLPFilterDeCap;		// best SNR with this filter
	/* calculate the prescaler value */
	ADCinit.prescale = ADC_PrescaleCalc(ADC_clock_frequency, 0); //
	ADC_Init(ADC0, &ADCinit);				// write configuration registers
	/* load default values for ADC channel configuration */
	ADC_InitSingle_TypeDef ADCsingleInit = ADC_INITSINGLE_DEFAULT;
	ADCsingleInit.reference = adcRef2V5;	// use internal 2.5V reference
	ADCsingleInit.input = adcSingleInpCh0;	// ADC channel 0
	ADCsingleInit.resolution = adcRes12Bit;	// 12 bit resolution
	ADC_InitSingle(ADC0, &ADCsingleInit);	// write input configuration
}


/** ***************************************************************************
 * @brief Start ADC0 for one single conversion
 *
 *****************************************************************************/
void ADC0_start_one(void) {
	ADC0->CMD = ADC_CMD_SINGLESTART;		// start ADC conversion
}


/** ***************************************************************************
 * @brief Read the converted value from ADC0
 * @return ADC reading
 *****************************************************************************/
uint32_t ADC0_read_one(void) {
	return ADC0->SINGLEDATA;				// read converted ADC value
}


/** ***************************************************************************
 * @brief Initialize DAC0 channel 1
 *
 *****************************************************************************/
void DAC0_init(void) {
	/* DAC_clock_frequency < 1MHz */
	uint32_t ADC_clock_frequency = 1000000;	// 1MHz is the upper limit
	CMU_ClockEnable(cmuClock_DAC0, true);	// enable DAC clock
	/* load default values for general DAC configuration */
	DAC_Init_TypeDef DACinit = DAC_INIT_DEFAULT;
	/* calculate the prescaler value */
	DACinit.prescale = DAC_PrescaleCalc(ADC_clock_frequency, 0);
	DACinit.reference = dacRef2V5;			// use internal 2.5V reference
	DAC_Init(DAC0, &DACinit);				// write configuration registers
	/* load default values for DAC channel configuration */
	DAC_InitChannel_TypeDef DACinitChannel = DAC_INITCHANNEL_DEFAULT;
	DAC_InitChannel(DAC0, &DACinitChannel, 1);	// write channel 1 configuration
	DAC_Enable(DAC0, 1, true);				// enable channel 1
	DAC0->CH1DATA = 0;						// output value for channel 1
}


/** ***************************************************************************
 * @brief Output new value to DAC0 channel 1
 * @param [in] value_out
 *****************************************************************************/
void DAC0_write(uint32_t value_out) {
	DAC0->CH1DATA = value_out;				// output value for channel 1
}


/** ***************************************************************************
 * @brief Initialize comparator ACMP0 and activate rising edge interrupt.
 * @n Compare input on channel 5 with scaled portion of VDD
 *****************************************************************************/
void ACMP0_init(void) {
	CMU_ClockEnable(cmuClock_ACMP0, true);	// enable ACMP clock
	/* load default values for general ACMP configuration */
	ACMP_Init_TypeDef ACMPinit = ACMP_INIT_DEFAULT;
	ACMPinit.interruptOnRisingEdge = true;	// detect rising edge only
	ACMPinit.hysteresisLevel = acmpHysteresisLevel1;	// 15mV hysteresis
	ACMPinit.vddLevel = 0;					// comparator level = 0
	ACMP_Init(ACMP0, &ACMPinit);			// write configuration to registers
	/* Compare scaled portion of VDD with voltage at input channel 5 */
	ACMP_ChannelSet(ACMP0, acmpChannelVDD, acmpChannel5);
	NVIC_ClearPendingIRQ(ACMP0_IRQn);		// clear pending comparator interrupts
	ACMP_IntEnable(ACMP0, ACMP_IEN_EDGE);	// interrupt on edge detection
	NVIC_EnableIRQ(ACMP0_IRQn);				// enable comparator interrupts
}


/** ***************************************************************************
 * @brief Set new ACMP0 comparator level to scaled portion of VDD
 * @param [in] value_VDDlevel must be in the range = 0 .. 63
 *****************************************************************************/
void ACMP0_set_level(uint32_t value_VDDlevel) {
	// Maybe add some code to check if value_VDDlevel is in the valid range
	/* clear the register bits for VDDlevel first */
	ACMP0->INPUTSEL &= ~_ACMP_INPUTSEL_VDDLEVEL_MASK;
	/* and write the new value to the VDDlevel bits in the register */
	ACMP0->INPUTSEL |= (value_VDDlevel <<_ACMP_INPUTSEL_VDDLEVEL_SHIFT) & _ACMP_INPUTSEL_VDDLEVEL_MASK;
}


/**************************************************************************//**
 * @brief Can be called from the ACMP interrupt handler ACMP0_IRQHandler().
 * @note Be aware of the fact that ACMP0 and ACMP1 share the same
 * interrupt service routine ACMP0_IRQHandler().<br>
 * This implies that there is no ACMP1_IRQHandler().<br>
 * ACMP1 is used for the touchslider and handled by ACMP0_IRQHandler() directly.
 * @n This ACMPadditional_IRQHandler() handles the ACMP0 interrupt
 * when it is called by ACMP0_IRQHandler() in touchslider.c
 * @n As part of an interrupt handler this is a time critical section.
 *****************************************************************************/
void ACMP0additional_IRQHandler(void) {
	if (ACMP0->IF & ACMP_IFC_EDGE) {		// edge on ACMP0 detected
		ACMP0->IFC = ACMP_IFC_EDGE;			// clear interrupt flag
		// Maybe set a GPIO to start timing measurement with an oscilloscope

		// Here goes your code.

		// Maybe clear a GPIO to stop timing measurement with an oscilloscope
	}
}

