#pragma once

#include <termios.h>

#define FALSE 0
#define TRUE 1
#define TRANSMITTER 0
#define RECEIVER 1
#define MAX_BUF_SIZE 255
#define FLAG 0x7E
#define A 0x03
#define SET_C 0x03
#define UA_C 0x07
#define SUPERVISION_SIZE 5

enum state_t
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
};

int receiveSupervisionByte(int fdRead, unsigned char C);

void setUpReceiver(char *argv[], int *fdRead, struct termios *oldtio);

void setUpSender(char *argv[], int *fdWrite, struct termios *oldtio);

void readSentence(volatile int *STOP, int fdRead, char *buf);

void usage(int argc, char *argv[]);

void writeSentence(int fdWrite, char *buf);

void closeFd(int fd, struct termios *oldtio);
