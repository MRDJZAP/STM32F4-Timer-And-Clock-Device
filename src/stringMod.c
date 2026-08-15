#include <stdint.h>

char toLower(char c) {
  if (c >= 'A' && c <= 'Z') {
    return c + 32;
  }

  return c;
}

uint32_t strLen(const char *str) {
  if (!str) {
    return 0;
  }

  uint32_t len = 0;

  while (*(str++)) {
    len++;
  }

  return len;
}
