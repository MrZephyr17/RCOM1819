#pragma once

#include <termios.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE 1
#define TRANSMITTER 0
#define RECEIVER 1
#define BAUDRATE B38400
#define MAX_BUF_SIZE 255
#define FLAG 0x7E
#define A 0x03
#define SET_C 0x03
#define UA_C 0x07
#define SUPERVISION_SIZE 5
#define TIME_OUT 3
#define MAX_RETRY_NUMBER 3
#define PORT_SIZE 11

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

int receiveSupervisionByte(int fd, unsigned char C);

int sendSupervisionByte(int fd, unsigned char C);

void setUpPort(int port, int *fd, struct termios *oldtio);

void readSentence(volatile int *STOP, int fd, char *buf);

void usage(int argc, char *argv[]);

void writeSentence(int fd, char *buf);

void closeFd(int fd, struct termios *oldtio);
