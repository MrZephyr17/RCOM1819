#pragma once

#include <stdbool.h>

#define MAX_BUF_SIZE 600
#define NUMBER_OF_TESTS 5
#define NUMBER_OF_ALARMS 1
#define MAX_FILENAME_SIZE 50
#define DELIM_SIZE 21

typedef enum {
  INV, C, I, T_prop, FER
} test_t;

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

/**
 * @brief Gets the current time
 * 
 * @return unsigned long long 
 */
unsigned long long getTime();

/**
 * @brief Sets up time measuring variables
 * 
 */
void startTime();

/**
 * @brief Processes a -t CLI argument, checking if it's valid.
 *
 * @param argv
 * @param argNum
 * @return int
 */
int processTestArgument(char **argv, int argNum);