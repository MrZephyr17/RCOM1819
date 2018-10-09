#pragma once

#include <termios.h>
#include <stdlib.h>
#include <stdbool.h>

#define TRANSMITTER 0
#define RECEIVER 1
#define BAUDRATE B38400
#define MAX_BUF_SIZE 255
#define FLAG 0x7E
#define ESC 0x7D
#define ESC_2 0x5E
#define ESC_3 0x5D
#define A_03 0x03
#define A_01 0x01
#define SET_C 0x03
#define UA_C 0x07
#define RR0 0x05
#define RR1 0x85
#define REJ0 0x01
#define REJ1 0x81
#define C_I0 0x00
#define C_I1 0x40
#define SUPERVISION_SIZE 5
#define TIME_OUT 3
#define MAX_RETRY_NUMBER 3
#define PORT_SIZE 11

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

enum state_t
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    END
};

int llopen(int flag, int fd);

int llwrite(int fd, unsigned char *buffer, int length);

int llread(int fd, char *buffer);

int llclose(int fd);

int receiveSupervisionByte(int fd, unsigned char C);

int sendSupervisionByte(int fd, unsigned char C);

void setUpPort(int port, int *fd, struct termios *oldtio);

void readSentence(volatile int *STOP, int fd, char *buf);

void usage(int argc, char *argv[]);

void writeSentence(int fd, char *buf);

void closeFd(int fd, struct termios *oldtio);