/** ***************************************************************************
 * @file
 * @brief Touchslider and Touchgecko
 *
 * All the functions needed to handle the touchslider and the gecko pad.
 * @n
 * Code copied from the file "capsense.c" (version 3.20.5) from the example
 * "C:\SiliconLabs\SimplicityStudio\v2\developer\sdks\efm32\v2\kits\common\drivers"
 * and adapted to own requirements.
 * @note
 * The original code uses TIMER0, ACMP1, PRS and TIMER1.
 * The code has been modified to use only TIMER1 and ACMP1.
 * @n We don't want to use TIMER1 for the touchslider
 * as the TIMER1_CCx outputs can be used for PWM signal generation
 * and are available on the expansion header.
 * @n We can't use TIMER2 as it might be used to boost LCD voltage.
 * The LCD boost capacitors are connected to the TIMER2_CCx outputs.
 * @n Instead of TIMER1 it would also be possible to use the SysTick timer.
 * @note
 * Added function CAPSENSE_getSliderValue()
 * for freely definable range/resolution of slider position
 * @note
 * When using ACMP0 somewhere in a project, be aware of the fact that ACMP0 and ACMP1 share
 * the same interrupt service routine ACMP0_IRQHandler() which is placed in this file.
 *
 * Board:  Starter Kit EFM32-G8XX-STK
 * Device: EFM32G890F128 (Gecko)
 *
 * @author Hanspeter Hochreutener (hhrt@zhaw.ch)
 * @date 8.7.2015
 *****************************************************************************/


/* EM header files */
#include "em_device.h"

/* Drivers */
#include "em_emu.h"
#include "em_acmp.h"

#include "touchslider.h"

#include "powerLEDs.h"						// ACMP interrupt is shared!


/** ***************************************************************************
 * This flag is set/reset when CAPSENSE_getSliderValue() is called.
 * @n true = touchslider is touched
 * @n The flag may be reset externally if the new value has been processed
 *****************************************************************************/
volatile bool touchsliderFlag = false;


/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/** The current channel we are sensing. */
static volatile uint8_t currentChannel;
/** Flag for measurement completion. */
static volatile bool measurementComplete;

/** ACMP interrupt count */
static volatile uint32_t ACMPcount = 0;

/**************************************************************************//**
 * @brief A bit vector which represents the channels to iterate through
 * @param ACMP_CHANNELS Vector of channels.
 *****************************************************************************/
static const bool channelsInUse[ACMP_CHANNELS] = CAPSENSE_CH_IN_USE;

/**************************************************************************//**
 * @brief This vector stores the latest read values from the ACMP
 * @param ACMP_CHANNELS Vector of channels.
 *****************************************************************************/
static volatile uint32_t channelValues[ACMP_CHANNELS] = { 0, 0, 0, 0, 0, 0, 0, 0 };

/**************************************************************************//**
 * @brief  This stores the maximum values seen by a channel
 * @param ACMP_CHANNELS Vector of channels.
 *****************************************************************************/
static volatile uint32_t channelMaxValues[ACMP_CHANNELS] = { 1, 1, 1, 1, 1, 1, 1, 1 };

/** @endcond */

/**************************************************************************//**
 * @brief TIMER1 interrupt handler.
 *        When TIMER1 expires the number of pulses on TIMER1 is inserted into
 *        channelValues. If this values is bigger than what is recorded in
 *        channelMaxValues, channelMaxValues is updated.
 *        Finally, the next ACMP channel is selected.
 *****************************************************************************/
void TIMER1_IRQHandler(void)
{
  /* Stop timer */
  TIMER1->CMD = TIMER_CMD_STOP;

  /* Clear interrupt flag */
  TIMER1->IFC = TIMER_IFC_OF;

  /* Store value in channelValues */
  channelValues[currentChannel] = ACMPcount;

  /* Update channelMaxValues */
  if (ACMPcount > channelMaxValues[currentChannel])
    channelMaxValues[currentChannel] = ACMPcount;

  measurementComplete = true;
}

/**************************************************************************//**
 * @brief Get the current channelValue for a channel
 * @param channel The channel.
 * @return The channelValue.
 *****************************************************************************/
uint32_t CAPSENSE_getVal(uint8_t channel)
{
  return channelValues[channel];
}

/**************************************************************************//**
 * @brief Get the current normalized channelValue for a channel
 * @param channel The channel.
 * @return The channel value in range (0-256).
 *****************************************************************************/
uint32_t CAPSENSE_getNormalizedVal(uint8_t channel)
{
  uint32_t max = channelMaxValues[channel];
  return (channelValues[channel] << 8) / max;
}

/**************************************************************************//**
 * @brief Get the state of the Gecko Button
 * @param channel The channel.
 * @return true if the button is "pressed"
 *         false otherwise.
 *****************************************************************************/
bool CAPSENSE_getPressed(uint8_t channel)
{
  uint32_t threshold;					// lower threshold = lower sensitivity
  /* Threshold is set to 12.5% below the maximum value */
  /* This calculation is performed in two steps because channelMaxValues is
   * volatile. */
  threshold  = channelMaxValues[channel];
  //threshold -= channelMaxValues[channel] >> 3;
  threshold -= threshold >> 2;				// set threshold to 75%

  if (channelValues[channel] < threshold)
  {
    return true;
  }
  return false;
}

/**************************************************************************//**
 * @brief Get the position of the slider
 * @return The position of the slider if it can be determined,
 *         -1 otherwise.
 *****************************************************************************/
int32_t CAPSENSE_getSliderPosition(void)
{
  int      i;
  int      minPos = -1;
  uint32_t minVal = 224; /* 0.875 * 256 */
  /* Values used for interpolation. There is two more which represents the edges.
   * This makes the interpolation code a bit cleaner as we do not have to make special
   * cases for handling them */
  uint32_t interpol[6] = { 255, 255, 255, 255, 255, 255 };

  /* The calculated slider position. */
  int position;

  /* Iterate through the 4 slider bars and calculate the current value divided by
   * the maximum value multiplied by 256.
   * Note that there is an offset of 1 between channelValues and interpol.
   * This is done to make interpolation easier.
   */
  for (i = 1; i < 5; i++)
  {
    /* interpol[i] will be in the range 0-256 depending on channelMax */
    interpol[i]  = channelValues[i - 1] << 8;
    interpol[i] /= channelMaxValues[i - 1];
    /* Find the minimum value and position */
    if (interpol[i] < minVal)
    {
      minVal = interpol[i];
      minPos = i;
    }
  }
  /* Check if the slider has not been touched */
  if (minPos == -1)
    return -1;

  /* Start position. Shift by 4 to get additional resolution. */
  /* Because of the interpol trick earlier we have to substract one to offset that effect */
  position = (minPos - 1) << 4;

  /* Interpolate with pad to the left */
  position -= ((256 - interpol[minPos - 1]) << 3)
              / (256 - interpol[minPos]);

  /* Interpolate with pad to the right */
  position += ((256 - interpol[minPos + 1]) << 3)
              / (256 - interpol[minPos]);

  return position;
}

/**************************************************************************//**
 * @brief This function iterates through all the capsensors and reads and
 *        initiates a reading. Uses EM1 while waiting for the result from
 *        each sensor.
 *****************************************************************************/
void CAPSENSE_Sense(void)
{
  /* Use the default STK capacative sensing setup and enable it */
  ACMP_Enable(ACMP_CAPSENSE);

  uint8_t ch;
  /* Iterate trough all channels */
  for (currentChannel = 0; currentChannel < ACMP_CHANNELS; currentChannel++)
  {
    /* If this channel is not in use, skip to the next one */
    if (!channelsInUse[currentChannel])
    {
      continue;
    }

    /* Set up this channel in the ACMP. */
    ch = currentChannel;
    ACMP_CapsenseChannelSet(ACMP_CAPSENSE, (ACMP_Channel_TypeDef) ch);

    /* Reset timer and counter */
    TIMER1->CNT = 0;
    ACMPcount = 0;

    measurementComplete = false;

    /* Start timers */
    TIMER1->CMD = TIMER_CMD_START;

    /* Wait for measurement to complete */
    while ( measurementComplete == false )
    {
      EMU_EnterEM1();
    }
  }

  /* Disable ACMP while not sensing to reduce power consumption */
  ACMP_Disable(ACMP_CAPSENSE);
}

/**************************************************************************//**
 * @brief Initializes the capacative sense system.
 *        Capacative sensing uses two timers: TIMER1 and TIMER1 as well as ACMP.
 *        ACMP is set up in cap-sense (oscialltor mode).
 *        TIMER1 counts the number of pulses generated by ACMP_CAPSENSE.
 *        When TIMER1 expires it generates an interrupt.
 *        The number of pulses counted by TIMER1 is then stored in channelValues
 *****************************************************************************/
void CAPSENSE_Init(void)
{
  /* Use the default STK capacative sensing setup */
  ACMP_CapsenseInit_TypeDef capsenseInit = ACMP_CAPSENSE_INIT_DEFAULT;

  /* Enable TIMER1, ACMP1, ACMP_CAPSENSE and PRS clock */
  CMU->HFPERCLKDIV |= CMU_HFPERCLKDIV_HFPERCLKEN;
  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER1
                      | CMU_HFPERCLKEN0_TIMER1
                      | ACMP_CAPSENSE_CLKEN
                      | CMU_HFPERCLKEN0_PRS;

  /* Initialize TIMER1 - Prescaler 2^9, top value 10, interrupt on overflow */
  TIMER1->CTRL = TIMER_CTRL_PRESC_DIV512;
  TIMER1->TOP  = 100;
  TIMER1->IEN  = TIMER_IEN_OF;
  TIMER1->CNT  = 0;

  /* Set up ACMP1 in capsense mode */
  ACMP_CapsenseInit(ACMP_CAPSENSE, &capsenseInit);

  /* Enable TIMER1 interrupt */
  NVIC_EnableIRQ(TIMER1_IRQn);

  /* Enable ACMP1 interrupt */
  /* Interrupt on rising edge */
  ACMP1->CTRL |= ACMP_CTRL_IRISE;
  /* Enable edge interrupt */
  ACMP1->IEN |= ACMP_IF_EDGE;
  /* Enable ACMP interrupt: ACMP0 and ACMP1 are both connected to ACMP0_IRQn */
  NVIC_EnableIRQ(ACMP0_IRQn);	// ACMP1 and ACMP1 on same interrupt line

}


/**************************************************************************//**
 * @brief ACMP interrupt handler.
 * @note Be aware of the fact that ACMP0 and ACMP1 share the same
 * interrupt service routine ACMP0_IRQHandler().<br>
 * This implies that there is no ACMP1_IRQHandler().<br>
 * ACMP1 is used for the touchslider and handled here.
 * @note PWR_ACMP_IRQHandler() in powerLEDs.c is called to handle the ACMP0 interrupt
 *****************************************************************************/
void ACMP0_IRQHandler(void)
{
	PWR_ACMP_IRQHandler();					// Handle ACMP0 interrupt in powerLEDs.c

	if (ACMP_CAPSENSE->IF & ACMP_IFC_EDGE) {	// edge from capacitive sense
		ACMP_CAPSENSE->IFC = ACMP_IFC_EDGE;
		ACMPcount++;
	}


}


/** ***************************************************************************
 * @brief Get the position of the slider with respect to minimum and maximum values
 * @param [in] sliderMin sliderposition range is sliderMin ... sliderMax
 * @param [in] sliderMax sliderposition range is sliderMin ... sliderMax
 * @return position of the slider if it can be determined, -11111 otherwise
 * @note The variable touchsliderFlag is set/reset and
 * can then be handled asynchronously and cleared explicitly after handling.
 *
 * Function "CAPSENSE_getSliderPosition()" copied from "caplesense.c"
 * and slightly modified for freely definable range/resolution of sliderposition
 *****************************************************************************/
int32_t CAPSENSE_getSliderValue(int32_t sliderMin, int32_t sliderMax)
{
	const int32_t notDetected = -11111;   // slider position if not detected
	const int32_t resolution = 10000;     // increase resolution for calculations
	const int32_t reserve = resolution/32;// helps eliminating round errors
	int      i;
	int      minPos = notDetected;
	uint32_t minVal = 120; /* lower value for less sensitivity */
	/* Values used for interpolation. There are two more which represent the edges.
	 * This makes the interpolation code a bit cleaner as we do not have to make special
	 * cases for handling them */
	uint32_t interpol[6]      = { 255, 255, 255, 255, 255, 255 };
	uint32_t channelPattern[] = { 0,                        SLIDER_PART0_CHANNEL + 1,
			SLIDER_PART1_CHANNEL + 1,
			SLIDER_PART2_CHANNEL + 1,
			SLIDER_PART3_CHANNEL + 1 };

	/* The calculated slider position. */
	int position;

	/* Iterate through the 4 slider bars and calculate the current value divided by
	 * the maximum value multiplied by 256.
	 * Note that there is an offset of 1 between channelValues and interpol.
	 * This is done to make interpolation easier.
	 */
	for (i = 1; i < 5; i++)
	{
		/* interpol[i] will be in the range 0-256 depending on channelMax */
		interpol[i]  = channelValues[channelPattern[i] - 1] << 8;
		interpol[i] /= channelMaxValues[channelPattern[i] - 1];
		/* Find the minimum value and position */
		if (interpol[i] < minVal)
		{
			minVal = interpol[i];
			minPos = i;
		}
	}
	/* Check if the slider has not been touched */
	if (minPos == notDetected) {
		touchsliderFlag = false;		// touchslider is not touched
		return notDetected;
	}

	/* Start position. Multiply with resolution to get additional resolution. */
	/* Because of the interpol trick earlier we have to substract one to offset that effect */
	position = (minPos - 1) *2 *resolution/6;

	/* Interpolate with pad to the left */
	position -= ((256 - interpol[minPos - 1]) *resolution/6)
            		  / (256 - interpol[minPos]);

	/* Interpolate with pad to the right */
	position += ((256 - interpol[minPos + 1]) *resolution/6)
            		  / (256 - interpol[minPos]);

	position -= reserve;                  // eliminate rounding issue at low end
	position *= (sliderMax - sliderMin);  // map to desired range and
	position /= (resolution -2*reserve);  // eliminate rounding issue at high end
	position += sliderMin;                // and add offset
	if (position < sliderMin) { position = sliderMin; }
	if (position > sliderMax) { position = sliderMax; }

	touchsliderFlag = true;				// touchslider was touched
	return position;
}


