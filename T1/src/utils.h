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
 * @brief 
 * 
 * @param byte 
 * @param array 
 * @return true 
 * @return false 
 */
bool findByteOnArray(unsigned char byte, unsigned char *array);