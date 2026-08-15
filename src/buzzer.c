#include "stm32f411xe.h"
#include "tim.h"
#include <stdbool.h>
#include <stdint.h>

#define RCC_GPIOCEN (1U << 2)

void buzzerOn() { GPIOC->BSRR = (1U << 2); }

void buzzerOff() { GPIOC->BSRR = (1U << 18); }

void initBuzzer_PC2() {
  // enable clock for port C
  RCC->AHB1ENR |= RCC_GPIOCEN;

  GPIOC->MODER &= ~(0b11U << 4);
  GPIOC->MODER |= (1U << 4);
  GPIOC->MODER &= ~(1U << 5);

  GPIOC->OTYPER &= ~(1U << 2);

  // set the port to low speed
  GPIOC->OSPEEDR &= ~(0b11 << 4);
}

void beep(uint32_t numBeeps, uint32_t timeBeep, TIM_TypeDef *timer,
          volatile bool *onReset) {
  if (!timer) {
    return;
  }

  for (uint32_t numTimes = 0; numTimes < numBeeps; numTimes++) {
    if (*onReset) {
      break;
    }
    buzzerOn();
    delay(timeBeep, timer, onReset);
    buzzerOff();
    delay(timeBeep, timer, onReset);
  }
}
