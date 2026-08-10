#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include "stdbool.h"
#include "stdint.h"
#include "stm32f411xe.h"
#include "tim.h"
#include <stdint.h>

/*
 * NOTE: the displayNumber4Dig and displayTime4Dig have modulo and division
 * operations
 *
 * used gpio pins:
 * PC0 -> D1
 * PC1 -> D2
 * PC4 -> D3
 * PC5 -> D4
 * PB2 -> a
 * PC8 -> b
 * PC6 -> c
 * PC7 -> d
 * PB6 -> e
 * PB7 -> f
 * PB0 -> g
 * PB1 -> dp
 */

// displays 4 digit unsigned number for delayTime in ms,
// if num > 9999 it does not display, if tim == NULL it would use a backup loop
// (not precise)
// resets the screen before starting and returning
// if reset is true it would stop all operations and returns the elapsed time,
// otherwise will return 0
uint32_t displayNumber4Dig(uint32_t num, TIM_TypeDef *tim, uint32_t delayTime,
                           volatile bool *reset);

// displays 4 digit time with fHalf on the left two digits and sHalf on the
// right two digits for delaytime in ms
// if fHalf and sHalf > 99, it does not display
// resets the screen before starting and returning
// if reset is true it would stop all operations and returns the elapsed time,
// otherwise will return 0
uint32_t displayTime4Dig(uint32_t fHalf, uint32_t sHalf, TIM_TypeDef *tim,
                         uint32_t delayTime, volatile bool *reset);

// Displays a 4 digit string for delayTime in ms,
// if any of the pointers are NULL, returns immediately
// PRE: init7SegDispaly() must be called beforehand,
//      str must be a null terminated string
uint32_t displayStringStatic(const char *str, TIM_TypeDef *tim,
                             uint32_t delayTime, volatile bool *reset);

// initializes the 7 segment display by setting up the corresponding GPIO pins
// connecting to the dispalay
void init7SegDisplay();
#endif
