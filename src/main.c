/** ***************************************************************************
 * @mainpage Moodlight
 *
 * is the subject of the <b>Electonics-Project (ETP)</b>
 * starting with fall term 2020
 * in the <b>second year of the electronics engineering curriculum</b>
 * at the School of Engineering of the Zurich University of Applied Sciences.
 * 
 * Colour space approximated with power LEDs (white, amber, red, green and blue)
 * @image html colour_space_LEDs.png
 *
 *
 * LEDs driven by HW (step-down converter) and microcontroller SW (written in C)
 * @image html LEDdriverHW.png
 *
 * User interface on microcontroller evalboard:
 * pushbuttons to chose colour and mode,
 * touchslider to change value and touchgecko to restart (or stop)
 * @image html gecko.png
 *
 * For solution specific description see powerLEDs.c
 *
 * Remote control over serial interface with a protocol based on command strings
 * @image html putty.png
 *
 * User interface as smartphone app (written in Java) which uses the serial protocol over Bluetooth
 * @image html app.png
 * 
 * Colours may be: white, amber, red, green and blue<br>
 * Colour space preferably: hue, saturation, lightness<br>
 * Modes could be: midday, sunset moodlight, submarine, doze, awake, party, yoga, ...
 *
 * Board:  Starter Kit EFM32-G8XX-STK
 * Device: EFM32G890F128 (Gecko)
 *
 * @author schmiaa1@students.zhaw.ch, bodenma2@students.zhaw.ch
 * @date 14.12.2020
 *****************************************************************************/
  

/** ***************************************************************************
 * @file
 * @brief main file for the Moodlight
 *
 * Sets up uC, clocks, peripherals and user interface.
 * Starts the powerLEDs, etc.
 *
 * Loops in the user interface
 *
 * @author schmiaa1@students.zhaw.ch, bodenma2@students.zhaw.ch
 * @date 8.7.2015
 *****************************************************************************/ 


#include "em_chip.h"
#include "em_device.h"
#include "em_system.h"
#include "em_cmu.h"
#include "em_emu.h"

#include "segmentlcd.h"

#include "globals.h"
#include "communication.h"
#include "pushbuttons.h"
#include "powerLEDs.h"
#include "touchslider.h"
#include "userinterface.h"
#include "signalleds.h"


/******************************************************************************
 * Defines
 *****************************************************************************/
 


/******************************************************************************
 * Variables
 *****************************************************************************/



/******************************************************************************
 * Functions
 *****************************************************************************/


/**************************************************************************//**
 * @brief main function
 *
 * @return nothing, as it runs in an endless loop
 * Sets up uC, clocks, peripherals and user interface.
 * Waits in low energy mode for an interrupt.
 *****************************************************************************/
int main(void) {
  CHIP_Init();                  		// Chip revision alignment and errata fixes
  INIT_XOclocks();						// Start crystal oscillators

  COM_Init();							// Initialize serial communication

  PB_Init();							// Initialize the pushbuttons
  PB_EnableIRQ();						// and enable pushbuttons interrupts

  SL_Init();							// Initialize the signal LEDs

  CAPSENSE_Init();						// Init touch slider and touch gecko

  SegmentLCD_Init(false);				// Initialize the LCD
  SegmentLCD_Write("PowerUp");
  SegmentLCD_Number(0);

  PWR_init();							// Initialize the power LEDs

  while(1) {							// loop forever
	  SL_Toggle(SL_3_PORT, SL_3_PIN);	// can be used for oscilloscope synch.
	  UI_FSM_event();					// check for events
	  UI_FSM_state_value();				// handles the events
  }

}
