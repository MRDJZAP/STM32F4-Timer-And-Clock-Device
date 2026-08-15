#include "stdbool.h"
#include "stm32f411xe.h"

#define BUTTON_FLAG (1U << 13)

void initUserButton() {
  RCC->AHB1ENR |= (1U << 2);
  GPIOC->MODER &= ~(0b11UL << 26);
}

bool isUserButtonPressed() {
  uint32_t inputVal = GPIOC->IDR;

  // button is not pressed
  if (inputVal & BUTTON_FLAG) {
    return false;
  }

  // button is pressed
  return true;
}
