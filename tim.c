#include "tim.h"
#include "stdbool.h"
#include "chip_headers/CMSIS/Device/ST/STM32F4xx/Include/stm32f411xe.h"
#include <stdint.h>

#define TIM2_CLOCK_ENABLE (1U << 0)
#define TIM3_CLOCK_ENABLE (1U << 1)
#define TIM4_CLOCK_ENABLE (1U << 2)
#define TIM5_CLOCK_ENABLE (1U << 3)

#define TIM_ENABLE (1U << 0)
#define UIF_HEV_FLAG (1U << 0)
#define UIE_ENABLE (1U << 0)

TIM_TypeDef *timer2Ptr = NULL;
TIM_TypeDef *timer3Ptr = NULL;
TIM_TypeDef *timer4Ptr = NULL;
TIM_TypeDef *timer5Ptr = NULL;

void startTimer(TIM_TypeDef *timer);
void resetTimer(TIM_TypeDef *timer);
void stopTimer(TIM_TypeDef *timer);
bool hasHEV(TIM_TypeDef *timer);

void initMiliSecTimer(TIM_TypeDef *timer) {
  // enable clock
  if (timer == TIM2) {
    RCC->APB1ENR |= TIM2_CLOCK_ENABLE;
    timer2Ptr = timer;
  } else if (timer == TIM3) {
    RCC->APB1ENR |= TIM3_CLOCK_ENABLE;
    timer3Ptr = timer;
  } else if (timer == TIM4) {
    RCC->APB1ENR |= TIM4_CLOCK_ENABLE;
    timer4Ptr = timer;
  } else if (timer == TIM5) {
    RCC->APB1ENR |= TIM5_CLOCK_ENABLE;
    timer5Ptr = timer;
  } else {
    return;
  }

  // clock is default to 16MHZ, prescale to 10000HZ
  timer->PSC = 1600 - 1;
  // reload value set to 10 to accomodate 10 operations per miliseconds
  timer->ARR = 10 - 1;

  // reset the value of counter
  timer->CNT = 0;
}

void startTimer(TIM_TypeDef *timer) { timer->CR1 |= TIM_ENABLE; }
void resetTimer(TIM_TypeDef *timer) {
  timer->CR1 &= ~TIM_ENABLE;
  timer->CNT = 0;
}
void stopTimer(TIM_TypeDef *timer) { timer->CR1 &= ~TIM_ENABLE; }
bool hasHEV(TIM_TypeDef *timer) {
  if (timer->SR & UIF_HEV_FLAG) {
    timer->SR &= ~UIF_HEV_FLAG;
    return true;
  }
  return false;
}

void delay(uint32_t delay, TIM_TypeDef *tim, volatile bool *onReset) {
  if (tim) {
    resetTimer(tim);
    startTimer(tim);

    for (uint32_t i = 0; i < delay; i++) {
      if (*onReset) {
        break;
      }
      while (!hasHEV(tim))
        ;
    }
  }
}

void setInterupt(bool interEn, TIM_TypeDef *tim) {
  int32_t TIMx_IRQn;

  if (tim == TIM2) {
    TIMx_IRQn = TIM2_IRQn;
  } else if (tim == TIM3) {
    TIMx_IRQn = TIM3_IRQn;
  } else if (tim == TIM4) {
    TIMx_IRQn = TIM4_IRQn;
  } else if (tim == TIM5) {
    TIMx_IRQn = TIM5_IRQn;
  } else {
    return;
  }

  if (interEn) {
    tim->DIER |= UIE_ENABLE;
    NVIC_EnableIRQ(TIMx_IRQn);
  } else {
    tim->DIER &= ~UIE_ENABLE;
    NVIC_DisableIRQ(TIMx_IRQn);
  }
}
