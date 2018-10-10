#pragma once

#include <stdbool.h>

#define MAX_BUF_SIZE 255

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

#define debug_print(fmt, ...)                    \
    do                                           \
    {                                            \
        if (DEBUG_TEST)                          \
            fprintf(stderr, fmt, ##__VA_ARGS__); \
    } while (0)

bool findByteOnArray(unsigned char byte, unsigned char *array);

void readSentence(volatile int *STOP, int fd, char *buf);

void usage(int argc, char *argv[]);

void writeSentence(int fd, char *buf);