#include "stdbool.h"
#include "stm32f411xe.h"
#include "stringMod.h"
#include "tim.h"
#include <stdint.h>

#define RCC_GPIOCEN (1U << 2)
#define RCC_GPIOBEN (1U << 1)

// Array for digits 0 through 7
bool numBuffer[10][7] = {
    // a    b     c     d     e     f     g
    {true, true, true, true, true, true, false},     // 0
    {false, true, true, false, false, false, false}, // 1
    {true, true, false, true, true, false, true},    // 2
    {true, true, true, true, false, false, true},    // 3
    {false, true, true, false, false, true, true},   // 4
    {true, false, true, true, false, true, true},    // 5
    {true, false, true, true, true, true, true},     // 6
    {true, true, true, false, false, false, false},  // 7
    {true, true, true, true, true, true, true},      // 8
    {true, true, true, true, false, true, true}      // 9
};

bool charBuffer[26][7] = {
    // a      b      c      d      e      f      g
    {true,  true,  true,  false, true,  true,  true }, // A
    {false, false, true,  true,  true,  true,  true }, // B (lowercase 'b' style for clarity)
    {true,  false, false, true,  true,  true,  false}, // C
    {false, true,  true,  true,  true,  false, true }, // D (lowercase 'd' style for clarity)
    {true,  false, false, true,  true,  true,  true }, // E
    {true,  false, false, false, true,  true,  true }, // F
    {true,  false, true,  true,  true,  true,  false}, // G
    {false, true,  true,  false, true,  true,  true }, // H
    {false, true,  true,  false, false, false, false}, // I (uses right side 'b' and 'c')
    {false, true,  true,  true,  true,  false, false}, // J
    {false, false, false, false, false, false, false}, // K (Not possible on 7-segment)
    {false, false, false, true,  true,  true,  false}, // L
    {false, false, false, false, false, false, false}, // M (Not possible on 7-segment)
    {false, false, true,  false, true,  false, true }, // N (lowercase 'n' style for clarity)
    {true,  true,  true,  true,  true,  true,  false}, // O
    {true,  true,  false, false, true,  true,  true }, // P
    {true,  true,  true,  false, false, true,  true }, // Q
    {false, false, false, false, true,  false, true }, // R (lowercase 'r' style for clarity)
    {true,  false, true,  true,  false, true,  true }, // S
    {false, false, false, false, false, true,  true }, // T (Not possible on 7-segment)
    {false, true,  true,  true,  true,  true,  false}, // U
    {false, false, false, false, false, false, false}, // V (Not possible on 7-segment)
    {false, false, false, false, false, false, false}, // W (Not possible on 7-segment)
    {false, false, false, false, false, false, false}, // X (Not possible on 7-segment)
    {false, true,  true,  true,  false, true,  true }, // Y
    {true,  true,  false, true,  true,  false, true }  // Z (Same representation as '2')
};

// Sets all 7-segment display pins (digits + segments) to HIGH
// PRE: init7SegDisplay() must be called beforehand
static void resetDisplay4Dig(void) {
  // Turn OFF all digit lines (PC0, PC1, PC4, PC5 -> LOW)
  GPIOC->BSRR =
      (1U << (0 + 16)) | (1U << (1 + 16)) | (1U << (4 + 16)) | (1U << (5 + 16));

  // Turn OFF all segment lines (Set HIGH for Common Anode)
  // Port C segments (b, c, d)
  GPIOC->BSRR = (1U << 6) | (1U << 7) | (1U << 8);
  // Port B segments (a, e, f, g, dp)
  GPIOB->BSRR = (1U << 0) | (1U << 1) | (1U << 2) | (1U << 6) | (1U << 7);
}

// selects or deslects the specific digit based on val
// PRE: init7SegDisplay() must be called beforehand
static void configDig(uint8_t digit, bool select) {
  switch (digit) {
  case 0:
    GPIOC->BSRR = (1U << (select ? 0 : 16));
    break;
  case 1:
    GPIOC->BSRR = (1U << (select ? 1 : 17));
    break;
  case 2:
    GPIOC->BSRR = (1U << (select ? 4 : 20));
    break;
  case 3:
    GPIOC->BSRR = (1U << (select ? 5 : 21));
    break;
  default:
    return;
  }
}

// modify the specific segment by either turning it on (val = true) or off
// (val = false), if di
// PRE: init7SegDisplay() must be called beforehand
static void configSeg(char seg, bool select) {
  switch (seg) {
  case 'a':
    GPIOB->BSRR = (1U << (select ? 18 : 2));
    break;
  case 'b':
    GPIOC->BSRR = (1U << (select ? 24 : 8));
    break;
  case 'c':
    GPIOC->BSRR = (1U << (select ? 22 : 6));
    break;
  case 'd':
    GPIOC->BSRR = (1U << (select ? 23 : 7));
    break;
  case 'e':
    GPIOB->BSRR = (1U << (select ? 22 : 6));
    break;
  case 'f':
    GPIOB->BSRR = (1U << (select ? 23 : 7));
    break;
  case 'g':
    GPIOB->BSRR = (1U << (select ? 16 : 0));
    break;
  case 'h': // h for decimal
    GPIOB->BSRR = (1U << (select ? 17 : 1));
    break;
  default:
    return;
  }
}

// displays 4 digit unsigned number for delayTime in ms,
// if num > 9999 it does not display, if tim == NULL it would use a backup loop
// (not precise)
// resets the screen before starting and returning
uint32_t displayNumber4Dig(uint32_t num, TIM_TypeDef *tim, uint32_t delayTime,
                           volatile bool *reset) {
  if (num > 9999)
    return 0;

  uint16_t digits[4] = {num / 1000U, (num / 100U) % 10U, (num / 10U) % 10U,
                        num % 10U};

  // Convert total delay time (ms) into rapid 1ms multiplex iterations
  for (uint32_t elapsed = 0; elapsed < delayTime; elapsed += 4) {

    if (*reset) {
      resetDisplay4Dig();
      return elapsed;
    }

    for (uint8_t i = 0; i < 4; i++) {
      resetDisplay4Dig(); // Prevent ghosting
      configDig(i, true); // Turn on current digit
      // Update segments for current digit
      for (uint8_t j = 0; j < 7; j++) {
        configSeg('a' + j, numBuffer[digits[i]][j]);
      }

      delay(1, tim, reset); // Hold digit on for 1 ms
    }
  }

  resetDisplay4Dig();
  return 0;
}

uint32_t displayTime4Dig(uint32_t fHalf, uint32_t sHalf, TIM_TypeDef *tim,
                         uint32_t delayTime, volatile bool *reset) {
  if (fHalf > 99 || sHalf > 99) {
    return 0;
  }

  uint16_t digits[4] = {(fHalf / 10U) % 10U, fHalf % 10U, (sHalf / 10U) % 10U,
                        sHalf % 10U};

  for (uint32_t elapsed = 0; elapsed < delayTime; elapsed += 4) {

    if (*reset) {
      resetDisplay4Dig();
      return elapsed;
    }

    for (uint8_t i = 0; i < 4; i++) {
      resetDisplay4Dig();
      configDig(i, true);

      for (uint8_t j = 0; j < 7; j++) {
        configSeg('a' + j, numBuffer[digits[i]][j]);
      }

      if (i == 1) {
        configSeg('h', true);
      }

      delay(1, tim, reset);
    }
  }

  resetDisplay4Dig();
  return 0;
}

uint32_t displayStringStatic(const char *str, TIM_TypeDef *tim,
                             uint32_t delayTime, volatile bool *reset) {
  if (!str || !tim || !reset) {
    return 0;
  }

  // this should contain only alphabetical letters
  char lowerCasedBuff[4] = {0, 0, 0, 0};

  // convert string to all lowercased, Good for performance
  for (uint32_t i = 0; i < strLen(str); i++) {
    if (str[i] >= 'A' && str[i] <= 'z') {
      lowerCasedBuff[i] = toLower(str[i]);
      continue;
    }

    lowerCasedBuff[i] = 0;
  }

  for (uint32_t elapsed = 0; elapsed < delayTime;
       elapsed += 4) { // +4 to accomodate for 4ms in display

    if (*reset) {
      resetDisplay4Dig();
      return elapsed;
    }

    for (uint32_t i = 0; i < 4; i++) {
      resetDisplay4Dig();
      configDig(i, true);

      for (uint32_t j = 0; j < 7; j++) {
        // k is default to empty digit
        char letter = lowerCasedBuff[i] != 0 ? lowerCasedBuff[i] : 'k';
        configSeg('a' + j, charBuffer[letter - 'a'][j]);
      }

      delay(1, tim, reset);
    }
  }

  resetDisplay4Dig();
  return 0;
}

// initializes the 7 segment display by setting up the corresponding GPIO pins
// connecting to the dispalay
void init7SegDisplay() {
  // enable the clocks
  RCC->AHB1ENR |= RCC_GPIOBEN;
  RCC->AHB1ENR |= RCC_GPIOCEN;

  // set them to push_pull mode
  GPIOC->OTYPER &= ~((1U << 8) | (1U << 7) | (1U << 6) | (1U << 5) | (1U << 4) |
                     (1U << 1) | (1U << 0));
  GPIOB->OTYPER &= ~((1U << 7) | (1U << 6) | (1U << 2) | (1U << 1) | (1U << 0));

  // 1. Clear mode bits for pins 0, 1, 4, 5, 6, 7, 8
  GPIOC->MODER &=
      ~((3U << (0 * 2)) | (3U << (1 * 2)) | (3U << (4 * 2)) | (3U << (5 * 2)) |
        (3U << (6 * 2)) | (3U << (7 * 2)) | (3U << (8 * 2)));

  // 2. Set mode to Output (1U = 01b) for pins 0, 1, 4, 5, 6, 7, 8
  GPIOC->MODER |=
      ((1U << (0 * 2)) | (1U << (1 * 2)) | (1U << (4 * 2)) | (1U << (5 * 2)) |
       (1U << (6 * 2)) | (1U << (7 * 2)) | (1U << (8 * 2)));

  // Clear mode bits for pins 0, 1, 2, 6, 7
  GPIOB->MODER &= ~((3U << (0 * 2)) | (3U << (1 * 2)) | (3U << (2 * 2)) |
                    (3U << (6 * 2)) | (3U << (7 * 2)));

  // Set mode to Output (1U) for pins 0, 1, 2, 6, 7
  GPIOB->MODER |= ((1U << (0 * 2)) | (1U << (1 * 2)) | (1U << (2 * 2)) |
                   (1U << (6 * 2)) | (1U << (7 * 2)));

  resetDisplay4Dig();
}
