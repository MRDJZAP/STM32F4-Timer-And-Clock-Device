#ifndef STRING_MOD_H
#define STRING_MOD_H

#include <stdint.h>

// converts character c into lower case letter
// if c is already in lower case or it is not a letter,
// it would return unmodified c
char toLower(char c);

// returns the string length of str,
// if str is NULL returns 0
// PRE: str must be null terminated string
uint32_t strLen(const char *str);

#endif
