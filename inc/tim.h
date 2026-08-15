#ifndef TIM_H
#define TIM_H

#include "stdbool.h"
#include "stm32f411xe.h"
#include <stddef.h>

extern TIM_TypeDef *timer2Ptr;
extern TIM_TypeDef *timer3Ptr;
extern TIM_TypeDef *timer4Ptr;
extern TIM_TypeDef *timer5Ptr;

// initializes the specific timer and initializes the timer2Ptr
// timer must only be one of TIM5, TIM4, TIM3, TIM2, otherwise will
// do nothing
void initMiliSecTimer(TIM_TypeDef *timer);

void startTimer(TIM_TypeDef *timer);

void resetTimer(TIM_TypeDef *timer);
void stopTimer(TIM_TypeDef *timer);
bool hasHEV(TIM_TypeDef *timer);

// Delays for delay times, note delay is in ms
// if tim == NULL, it returns immediately
// PRE: initMiliSecTimer() must be called before hand
void delay(uint32_t delay, TIM_TypeDef *tim, volatile bool *onReset);

// sets the interupt for timer
void setInterupt(bool InteruptEn, TIM_TypeDef *tim);

#endif
