/** ***************************************************************************
 * @file
 * @brief User interface
 *
 * @todo Write the description of the effectively implemented user interface.
 *
 * The user interface is implemented as a Finite State Machine.
 *
 * The <b>states</b> are:
 * @n WHITE, AMBER, RED, GREEN, BLUE, IDLE, STOP, START
 *
 * The <b>events</b> and <b>transitions</b> are:<dl>
 * <dt>Touchgecko pressed</dt>
 * <dd>Go to state START, initialize the system and switch to state IDLE.</dd>
 * <dt>Touchslider touched</dt>
 * <dd>Adjust value of the currently active state.</dd>
 * <dt>Pushbutton 0 pressed</dt>
 * <dd>Go one state to the right, wrap around from IDLE to WHITE.</dd>
 * <dt>Pushbutton 1 pressed</dt>
 * <dd>Go one state to the left, wrap around from WHITE to IDLE.</dd>
 * <dt>Remote command from serial interface received</dt>
 * <dd>The received string is parsed and the new state and value set accordingly.</dd>
 * </dl>
 * Any changes in state or value are reflected on the <b>display</b>
 * and also sent over the serial interface to the <b>remote control</b>.
 *
 * Prefix: UI
 *
 * @author Hanspeter Hochreutener (hhrt@zhaw.ch)
 * @date 15.7.2015
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "em_emu.h"

#include "segmentlcd.h"

#include "globals.h"
#include "userinterface.h"
#include "pushbuttons.h"
#include "touchslider.h"
#include "communication.h"
#include "powerLEDs.h"


/******************************************************************************
 * Defines
 *****************************************************************************/

/** @todo Adjust number of user interface states. */
#define UI_STATE_COUNT		7				///< number of FSM states

/** @todo Adjust display and remote control text of user interface states. */
char * UI_text[UI_STATE_COUNT] = {
		"white", "amber", "red", "green", "blue", "idle", "start"
}; ///< text for display and remote control


/** @todo Adjusting required? */
#define UI_TEXT_COMPARE_LENGTH	3			///< compare first x chars of string
// Must be long enough to distinguish all the texts
// and not longer than the shortest text.

#define UI_DELAY 			10000	///< user interface delay (in TIMER0 ticks)


/******************************************************************************
 * Variables
 *****************************************************************************/

/** @todo Adjust enum names of user interface states. */
typedef enum {								///< enum with the FSM states
	WHITE = 0, AMBER, RED, GREEN, BLUE,		// colours
	IDLE, START								// special states
} UI_state_t;								// count must be = UI_STATE_COUNT

static UI_state_t UI_state_current = START;	///< current state of the FSM
UI_state_t UI_state_next = START;			///< next state of the FSM
bool UI_state_changed = true;				///< state transition of the FSM
static int32_t UI_value_current = 0;		///< current value (if applicable)
int32_t UI_value_next = 0;					///< next value (if applicable)
bool UI_value_changed = true;				///< value changed


/******************************************************************************
 * Functions
 *****************************************************************************/


/** **************************************************************************
 * @brief Part of the user interface finite state machine: Touch events
 *
 * @todo Probably this code has to be reviewed and adapted.
 *****************************************************************************/
void UI_FSM_event_Touch(void) {
	CAPSENSE_Sense();
	switch (UI_state_next) {
	case WHITE:
	case AMBER:
	case RED:
	case GREEN:
	case BLUE:
		/* Touchslider touched? */
		UI_value_next = CAPSENSE_getSliderValue(0, PWR_VALUE_MAX);	// read value
		if (touchsliderFlag) {				// touchslider touched?
			UI_value_changed = true;
			touchsliderFlag = false;		// reset the flag
		}
		break;
	default:								// no value to change in other states
		;
	}
	/* Touchgecko pressed? */
	if (CAPSENSE_getPressed(BUTTON_CHANNEL)) {	// read status of the touchgecko
		UI_state_next = START;				// initialize system
		UI_state_changed = true;			// set the flag
	}
}


/** **************************************************************************
 * @brief Part of the user interface finite state machine: Pushbutton events
 *
 * @todo Probably this code has to be reviewed and adapted.
 *****************************************************************************/
void UI_FSM_event_Pushbutton(void) {
	/* Pushbutton 0 pressed? */
	if (PB0_IRQflag) {
		switch (UI_state_current) {
		case WHITE:
			UI_state_next = IDLE;
			break;
		case AMBER:
		case RED:
		case GREEN:
		case BLUE:
			UI_state_next--;
			break;
		case IDLE:
			UI_state_next = BLUE;
			break;
		default:
			;
		}
		UI_state_changed = true;		// set the flag
		PB0_IRQflag = false;			// reset the flag
	}
	/* Pushbutton 1 pressed? */
	if (PB1_IRQflag) {
		switch (UI_state_current) {
		case WHITE:
		case AMBER:
		case RED:
		case GREEN:
			UI_state_next++;
			break;
		case BLUE:
			UI_state_next = IDLE;
			break;
		case IDLE:
			UI_state_next = WHITE;
			break;
		default:
			;
		}
		UI_state_changed = true;		// set the flag
		PB1_IRQflag = false;			// reset the flag
	}
}

/** **************************************************************************
 * @brief Part of the user interface finite state machine: Remote control events
 *
 * If a string is available from the serial interface read and analyze it.
 * @n The core is a parser which analyzes the received command string.
 * @n It extracts the state and the value (if any).
 * The value is separated by a ' ' from the state.
 * @n There are 3 possible outcomes:
 * @n - No valid state is recognized
 * => nothing happens
 * @n - A valid state is recognized, but no valid value
 * => switch to that state, don't change the value
 * @n - Valid state and value received
 * => switch to that state and change the value
 *****************************************************************************/
void UI_FSM_event_RemoteControl(void) {
	char command[COM_BUF_SIZE] = "";		// receive buffer
	if (COM_RX_Available()) {				// check for a new command string
		COM_RX_GetData(command, COM_BUF_SIZE);
		/* loop through all valid states to check if one matches */
		for (uint32_t state = 0; state < UI_STATE_COUNT; state++) {	// loop through states
			/* check if the command contains a valid state */
			if (0 == strncmp(UI_text[state], command, UI_TEXT_COMPARE_LENGTH)) {
				UI_state_next = state;		// change to that state
				/* check if the command contains a valid number */
				char *pos, *end;	// temporary variables for char position and end
				/* the number (if any) is separated by a ' ' from the state */
				pos = strchr(command, ' ');	// so look for a ' '
				if (pos) {					// a ' ' has been found
					/* try to convert the remainder of the command into a number */
					int32_t number = strtol(pos, &end, 10);	// convert, base 10
					if ('\0' == *end) {		// conversion was successful
						UI_value_next = number;
						UI_value_changed = true;	// set the value changed flag
					}
				}
			}
		}
		UI_state_changed = true;			// set the state changed flag
	}
}


/** **************************************************************************
 * @brief User interface finite state machine: Checks for events
 *
 * Check if an event has occurred and define next state and value.
 *****************************************************************************/
void UI_FSM_event(void) {
	UI_FSM_event_Touch();
	UI_FSM_event_Pushbutton();
	UI_FSM_event_RemoteControl();
}


/** **************************************************************************
 * @brief User interface finite state machine: Handled the events
 *
 * @n Switch to the next state and/or adjust the value.
 *
 * @todo Probably this code has to be reviewed and adapted.
 *****************************************************************************/
void UI_FSM_state_value(void) {
	/* store new value for power LEDs */
	if (UI_value_changed) {
		switch (UI_state_next) {
		case WHITE:
		case AMBER:
		case RED:
		case GREEN:
		case BLUE:
			PWR_set_value(UI_state_next, UI_value_next);
			break;
		default:
			;
		}
	}
	/* display new state and value and send this infos also to the remote control */
	if (UI_state_changed || UI_value_changed) {
		switch (UI_state_next) {
		case WHITE:
		case AMBER:
		case RED:
		case GREEN:
		case BLUE:
			UI_value_next = PWR_get_value(UI_state_next);	// get the actual value
			/* display state and value */
			SegmentLCD_Write(UI_text[UI_state_next]);
			SegmentLCD_Number(UI_value_next);
			/* send state and value to the remote control over the serial interface */
			char value_string[COM_BUF_SIZE];
			char message[COM_BUF_SIZE];
			ltostr(UI_value_next, value_string);	// convert number to string
			strncpy(message, UI_text[UI_state_next], COM_BUF_SIZE);
			strncat(message, " ", COM_BUF_SIZE);
			strncat(message, value_string, COM_BUF_SIZE);
			COM_TX_PutData(message, COM_BUF_SIZE);	// send the string
			break;
		case IDLE:
		case START:
			/* display state, blank display for value*/
			SegmentLCD_Write(UI_text[UI_state_next]);
			SegmentLCD_NumberOff();
			/* send state to the remote control over the serial interface */
			COM_TX_PutData(UI_text[UI_state_next], COM_BUF_SIZE);
			break;
		default:
			;
		}
	}
	/* Update current state, value and flags */
	UI_state_current = UI_state_next;
	UI_state_changed = false;
	UI_value_current = UI_value_next;
	UI_value_changed = false;

	/* delay for UI scan and update */
	// RTCDRV_Delay(UI_delay, false);	// can't be used as this stops TIMER0 for about 2ms
	for (uint32_t delay = 0; delay < UI_DELAY; delay++) {
		/** @todo Adjust UI_DELAY to get a sensible responsiveness.
		 * If a timer interrupt is activated, uncomment the following line. */
		// EMU_EnterEM1();			// wakes up on an interrupt (mostly a timer)
	}

	/* treat START and STOP specifically */
	if (START == UI_state_current){
		for (uint32_t solution = 0; solution < PWR_SOLUTION_COUNT; solution++) {
			PWR_set_value(solution, PWR_START_VALUE);
		}
		UI_state_next = IDLE;
		UI_state_changed = true;
		// no break as we want to update the display
	}

}

