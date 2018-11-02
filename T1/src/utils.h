#pragma once

#include <stdbool.h>

#define MAX_BUF_SIZE 264

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

#define debug_print(fmt, ...)                                                  \
  do {                                                                         \
    if (DEBUG_TEST)                                                            \
      fprintf(stderr, fmt, ##__VA_ARGS__);                                     \
  } while (0)

/**
 * @brief Searches for byte (unsigned char) in array of bytes (unsigned chars) with name array.
 * 
 * @param byte : unsigned char to be found
 * @param array : array of bytes we want to check if has byte or not
 * @return true : if byte exists in array
 * @return false : otherwise
 */
bool findByteOnArray(unsigned char byte, unsigned char *array);