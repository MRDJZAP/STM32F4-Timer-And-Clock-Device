#include "stdbool.h"
#include "chip_headers/CMSIS/Device/ST/STM32F4xx/Include/stm32f411xe.h"

#define RCC_GPIOCEN (1U << 2)
#define RCC_SYSCFGEN (1U << 14)

void pc13_exti13_init() {
  // Disable global interrupts
  __disable_irq();

  // Enable clock access for GPIOC
  RCC->AHB1ENR |= RCC_GPIOCEN;

  // Set PC13 as input for button
  GPIOC->MODER &= ~(1U << 26);
  GPIOC->MODER &= ~(1U << 27);

  // enable clock access to syscfg
  RCC->APB2ENR |= RCC_SYSCFGEN;

  // Enable portc for pin 13 for exti 13
  SYSCFG->EXTICR[3] |= (1U << 5);

  // Enable intrupts for exti 13
  EXTI->IMR |= (1U << 13);

  // Select falling edge for exti 13
  EXTI->FTSR |= (1U << 13);

  // Enable EXTI13 line in NVIC
  // this would enable the IRQ lines for EXTI lines 15-10
  NVIC_EnableIRQ(EXTI15_10_IRQn);

  // Enable global interrupts
  __enable_irq();
}

// disables/enables the interupt line for Exti13 based on select
// PRE: pc13_exti13_init() must be called beforehand
void setExti13Interupt(bool select) {
  if (select) {
    EXTI->IMR |= (1U << 13);
  } else {
    EXTI->IMR &= ~(1U << 13);
  }
}
