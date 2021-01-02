/** ***************************************************************************
 * @file
 * @brief See powerLEDs.c
 *****************************************************************************/

#ifndef PWRLEDS_H_
#define PWRLEDS_H_

/******************************************************************************
 * Defines
 *****************************************************************************/
#define PWR_SOLUTION_COUNT		5			///< number of power LED drivers

#define PWR_VALUE_MAX			255			///< max value for set points
#define PWR_START_VALUE		PWR_VALUE_MAX/4	///< start value for set points

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

void PWR_set_value(uint32_t channel, int32_t value);

uint32_t PWR_get_value(uint32_t channel);

void PWR_init(void);

void PWR_ACMP_IRQHandler(void);

#endif
