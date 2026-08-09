#ifndef EXTI_H
#define EXTI_H

#include "stdbool.h"
#define LINE13 (1U << 13)

// initializes the pc13 and attaches it to the exti13 for allowing
// interputs
// NOTE: it has falling_edge detection
void pc13_exti13_init();

// disables/enables the interupt line for Exti13 based on selsect
// PRE: pc13_exti13_init() must be called beforehand
void setExti13Interupt(bool select);

#endif
