#ifndef BUZZER_H
#define BUZZER_H

#include "stdint.h"
#include "stm32f411xe.h"

/*
 * NOTE: uses GPIO pin C2
 */

// turns buzzer on
// PRE: initBuzzer_PC2() must be called beforehand
void buzzerOn();

// turns buzzer on
// PRE: initBuzzer_PC2() must be called beforehand
void buzzerOff();

// initialises the PC2 pin connected to the buzzer
void initBuzzer_PC2();

// beeps for numBeeps, where each beep lasts for timeBeep in ms
// silence between each beep would also be determined by timeBeep
// if timer == NULL, does nothing
// PRE: initBuzzer_PC2() must be called beforehand
void beep(uint32_t numBeeps, uint32_t timeBeep, TIM_TypeDef *timer,
          volatile bool *onReset);

#endif
