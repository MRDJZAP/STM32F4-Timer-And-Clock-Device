#ifndef USER_BUTTON_H
#define USER_BUTTON_H

#include "stdbool.h"
#include "stm32f411xe.h"

// NOTE: uses PC13 which is connected to the user button provided by the
// NucloeF411RE board

// intitializes the userbutton
void initUserButton();

// checks weather the user button is pressed or not
bool isUserButtonPressed();

#endif
