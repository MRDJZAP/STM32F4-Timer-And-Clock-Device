#ifndef USER_BUTTON_H
#define USER_BUTTON_H

#include "stdbool.h"
#include "chip_headers/CMSIS/Device/ST/STM32F4xx/Include/stm32f411xe.h"

// NOTE: uses PC13 which is connected to the user button provided by the
// NucloeF411RE board

// intitializes the userbutton, PC13 enabled
void initUserButton();

// checks weather the user button is pressed or not
// PRE: initUserButton() must be called beforehand
bool isUserButtonPressed();

#endif
