#include "SevenSeg.h"
#include "buzzer.h"
#include "exti.h"
#include "rtc.h"
#include "stdbool.h"
#include "stddef.h"
#include "stm32f411xe.h"
#include "tim.h"
#include "userButton.h"
#include <stdint.h>

int32_t timerSecs = 0;
int32_t timerMins = 0;

TimeStruct time = {0, 0, 0, 0};
DateStruct date = {1, 1, 1, 0};

bool isTiming = false;
bool isTimerFinished = true;

enum { CLOCK, TIMER } mode = CLOCK;
volatile bool isButtonPressed = false;

// handles userButton and sets the curFunc based on the button pressed
void handleButton();

// // sets the isButtonPressed Flag
// void EXTI15_10_IRQHandler();

// Resets/sets the timer or clock based on the mode
// Uses the user button for setting the individual seconds and minutes
// PRE: interrupts to the button must be turned off
void timerClockSetMode();

// Counts down the timer when in timer mode
void timerMode();

// shows the clock in clock mode
void clockMode();

// helpers
// counts the timer if isTiming is true,
// if timer reaches zero it would switch to timermode to sound the alarmI
void timerCount();

void (*timerFunc)() = timerMode;
void (*curFunc)() = clockMode;

int main() {
  initMiliSecTimer(TIM2);
  pc13_exti13_init();
  init7SegDisplay();
  initBuzzer_PC2();
  rtc_init(&date, &time, 0);

  while (1) {
    if (isButtonPressed) {
      handleButton();
    }

    curFunc();
  }
}

void clockMode() {
  displayTime4Dig(rtc_time_get_hour(), rtc_time_get_minute(), timer2Ptr, 1000,
                  &isButtonPressed);

  if (isButtonPressed) { // return immediately
    return;
  }

  timerCount(); // keep time
}

void timerMode(void) {

  static uint32_t timeElapsed = 0;

  if (!isTiming) {
    displayTime4Dig(timerMins, timerSecs, timer2Ptr, 1000, &isButtonPressed);
  } else if (timerMins > 0 || timerSecs > 0) { // start/continue timing

    timeElapsed = displayTime4Dig(timerMins, timerSecs, timer2Ptr,
                                  1000 - timeElapsed, &isButtonPressed);

    if (isButtonPressed) {
      return;
    }

    timerCount();

  } else { // start/continue alarm
    displayNumber4Dig(0, timer2Ptr, 500, &isButtonPressed);
    beep(4, 100, timer2Ptr, &isButtonPressed);
  }
}

void timerClockSetMode(void) {

  // turn off interupts
  setExti13Interupt(false);
  isButtonPressed = false;

  // First half and Second half
  enum { FIR_HALF, SEC_HALF } setMode = SEC_HALF;

  int32_t firHalf = 0;
  int32_t secHalf = 0;

  while (1) {
    uint32_t timePressed = 0;

    resetTimer(timer2Ptr);
    startTimer(timer2Ptr);

    // Wait and measure how long user holds button
    while (isUserButtonPressed()) {
      while (!hasHEV(timer2Ptr))
        ;
      timePressed++;

      displayTime4Dig(firHalf, secHalf, timer2Ptr, 1, &isButtonPressed);

      if (timePressed == 500) {
        beep(2, 50, timer2Ptr, &isButtonPressed);
      }
    }

    displayTime4Dig(firHalf, secHalf, timer2Ptr, 1, &isButtonPressed);

    // Process button action after release
    if (timePressed > 0) {
      if (timePressed >= 500) { // Long press switches setMode or exits
        if (setMode == FIR_HALF) {
          break; // Done setting timer
        }
        setMode = FIR_HALF;
      } else { // Short press increments value
        if (setMode == FIR_HALF) {
          if (mode == TIMER) {
            firHalf = (firHalf + 1) % 60;
          } else {
            firHalf = (firHalf + 1) % 24;
          }
        } else {
          secHalf = (secHalf + 1) % 60;
        }
      }
    }
  }

  if (mode == TIMER) {
    timerSecs = secHalf;
    timerMins = firHalf;
    curFunc = timerMode;
    isTiming = true;
    isTimerFinished = false;
  } else {
    time.hours = firHalf;
    time.minutes = secHalf;
    change_time(&time, 0); // set to 24 hour mode
    curFunc = clockMode;
  }

  setExti13Interupt(true); // enable interupts again
}

// Clean ISR - sets state flag quickly and returns immediately
void EXTI15_10_IRQHandler(void) {
  if (EXTI->PR & LINE13) {
    EXTI->PR = LINE13; // Clear pending registe

    (void)EXTI->PR;

    isButtonPressed = true;
  }
}

void handleButton() {
  setExti13Interupt(false);
  isButtonPressed = false;

  uint32_t timePressed = 0;
  resetTimer(timer2Ptr);
  startTimer(timer2Ptr);

  while (isUserButtonPressed()) {
    while (!hasHEV(timer2Ptr))
      ;

    timePressed++;

    if (mode == TIMER) {
      displayTime4Dig(timerMins, timerSecs, timer2Ptr, 1, &isButtonPressed);
    } else {
      displayTime4Dig(rtc_time_get_hour(), rtc_time_get_minute(), timer2Ptr, 1,
                      &isButtonPressed);
    }

    if (timePressed == 500 && mode == TIMER) { // equivalant to 2 seconds
      // entered paused mode in timer
      beep(2, 50, timer2Ptr, &isButtonPressed);
    }

    if (timePressed == 800) { // equivalant to 4 seconds
      // entered reset mode
      beep(4, 50, timer2Ptr, &isButtonPressed);
    }
  }

  if (timePressed >= 1000) {
    curFunc = timerClockSetMode;
  } else if (timePressed >= 500 && mode == TIMER) {
    if (isTiming) {
      isTiming = false;
    } else if (!isTimerFinished) {
      isTiming = true;
    }

    timerFunc = curFunc; // presist state of timer when in clock mode
  } else {
    beep(1, 50, timer2Ptr, &isButtonPressed);
    if (mode == TIMER) {
      curFunc = clockMode;
      mode = CLOCK;
      displayStringStatic("C", timer2Ptr, 500, &isButtonPressed);
    } else {
      curFunc = timerFunc;
      mode = TIMER;
      displayStringStatic("P", timer2Ptr, 500, &isButtonPressed);
    }
  }

  setExti13Interupt(true);
}

// helper
void timerCount() {
  if (isTiming) {
    timerSecs--;
    if (timerSecs < 0) {
      if (timerMins > 0) {
        timerSecs = 59;
        timerMins--;
      } else {
        timerSecs = 0; // Stopped at 00:00
        curFunc = timerMode;
        mode = TIMER;
        isTimerFinished = true;
      }
    }
  }
}
