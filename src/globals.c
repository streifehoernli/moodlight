/** ***************************************************************************
 * @file
 * @brief Global variables, defines and functions
 *
 * Prefix: G
 *
 * @author schmiaa1@students.zhaw.ch, bodenma2@students.zhaw.ch
 * @date 6.5.2015
 *****************************************************************************/ 


#include "em_cmu.h"

#include "globals.h"

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
 * @brief Start crystal oscillators and use these clock sources
 *
 * High frequency clock = 32MHhz (defined by crystal on the starter kit)
 * Low frequency clock = 32.768kHz (defined by crystal on the starter kit)
 *****************************************************************************/
void INIT_XOclocks() {
	// High frequency clock
	CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	CMU_ClockEnable(cmuClock_HFPER, true);
	// Low frequency clock
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	CMU_ClockSelectSet(cmuClock_CORELE, cmuSelect_LFXO);
	CMU_ClockEnable(cmuClock_CORELE, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockEnable(cmuClock_LFA, true);
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
	CMU_ClockEnable(cmuClock_LFB, true);
}


/**************************************************************************//**
 * @brief  Convert an integer to a string (in base 10 representation)
 * @param [in] l = integer to convert
 * @param [out] string = string representation of number l
 *
 * ltostr (long to string) is the inverse function to strtol (string to long)
 * @n strtol is a standard function in stdlib.h
 * @n ltostr unfortunately is not. So it is provided here for convenience.
 * @n Make sure the string is at least [12] long to hold the maximum integer.
 * @note strtol and ltostr need less memory than sscanf and sprintf
 ******************************************************************************/
void ltostr(int32_t l, char *string) {
	const char digit[] = "0123456789";	// digit characters
	if (0 > l) {			// negative integer
		l *= -1;			// change sign
		*string++ = '-';	// output a '-' and move one digit to the right
	}
	int tmp = l;
	do {					// calculate number of digits in output string
		tmp = tmp / 10;		// next digit
		string++;			// move to the end of the output string
	} while (0 < tmp);		// do while handles also the special case l == 0
	*string-- = '\0';		// output end of string char and move to last digit
	do {					// begin with rightmost digit in output string
		*string-- = digit[l % 10];	// last digit and move to next digit left
		l = l / 10;			// remaining integer portion without last digits
	} while (0 < l);		// do while handles also the special case l == 0
}




