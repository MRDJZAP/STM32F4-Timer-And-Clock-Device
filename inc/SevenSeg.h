#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include "stdbool.h"
#include "stdint.h"
#include "stm32f411xe.h"
#include "tim.h"
#include <stdint.h>

/*
 * @note the displayNumber4Dig and displayTime4Dig have modulo and division
 * operations
 *
 * @note used gpio pins:
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

/**
 * @brief Displays a 4-digit unsigned number for a specified duration.
 *
 * Clears the 7-segment display before and after execution. If @p reset is set
 * to true during operation, the display loop stops immediately and returns the
 * elapsed time.
 *
 * @param[in] num       The 32-bit unsigned number to display (valid range:
 * 0–9999).
 * @param[in] tim       Pointer to the hardware timer peripheral instance.
 * @param[in] delayTime Display duration in milliseconds.
 * @param[in] reset     Pointer to a flag that halts execution when true.
 *
 * @return Elapsed time in milliseconds if interrupted by @p reset; 0 otherwise.
 *
 * @pre   init7SegDisplay() must be called prior to calling this function.
 * @note  Returns immediately with 0 if @p tim is NULL or if @p
 * num > 9999.
 */
uint32_t displayNumber4Dig(uint32_t num, TIM_TypeDef *tim, uint32_t delayTime,
                           volatile bool *reset);

/**
 * @brief Displays two 2-digit numbers side-by-side as a time value.
 *
 * Displays @p fHalf on the left two digits and @p sHalf on the right two digits
 * (e.g., MM:SS or HH:MM) for the specified duration. Clears the display before
 * starting and upon completion. If @p reset becomes true, execution halts
 * immediately.
 *
 * @param[in] fHalf     First half of the time (left 2 digits, valid range:
 * 0–99).
 * @param[in] sHalf     Second half of the time (right 2 digits, valid range:
 * 0–99).
 * @param[in] tim       Pointer to the hardware timer peripheral instance.
 * @param[in] delayTime Display duration in milliseconds.
 * @param[in] reset     Pointer to a flag that halts execution when true.
 *
 * @return Elapsed time in milliseconds if interrupted by @p reset; 0 otherwise.
 *
 * @pre   init7SegDisplay() must be called prior to calling this function.
 * @note  Returns immediately with 0 if @p tim is NULL
 */
uint32_t displayTime4Dig(uint32_t fHalf, uint32_t sHalf, TIM_TypeDef *tim,
                         uint32_t delayTime, volatile bool *reset);

/**
 * @brief Displays a 4-character string on the 7-segment display for a specified
 * duration.
 *
 * Renders a static string to the display. Clears the display before starting
 * and upon completion. If @p reset is set to true during execution, operation
 * halts immediately.
 *
 * @param[in] str       Pointer to a null-terminated string to display
 * (typically 4 characters).
 * @param[in] tim       Pointer to the hardware timer peripheral instance.
 * @param[in] delayTime Display duration in milliseconds.
 * @param[in] reset     Pointer to a flag that halts execution when true.
 *
 * @return Elapsed time in milliseconds if interrupted by @p reset; 0 otherwise.
 *
 * @pre   init7SegDisplay() must be called prior to calling this function.
 * @pre   @p str must be a valid, null-terminated string.
 * @note  Returns immediately with 0 if pointer parameter (@p str, @p tim,) is
 * NULL.
 */
uint32_t displayStringStatic(const char *str, TIM_TypeDef *tim,
                             uint32_t delayTime, volatile bool *reset);

// @brief Initializes the 7 segment display by setting up the corresponding GPIO
// pins connecting to the dispalay
void init7SegDisplay();

#endif
