#pragma once

#include <termios.h>

enum state_t
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    END
};

#define TRANSMITTER 0
#define RECEIVER 1

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
#define START_C 0x02
#define F_C 0x01
#define END_C 0x03
#define T_LENGTH 0x00
#define T_NAME 0x01

#define TIME_OUT 3
#define MAX_RETRY_NUMBER 3

#define BAUDRATE B38400
#define SUPERVISION_SIZE 5
#define PORT_SIZE 11

int llopen(int flag, int fd);

int llopen_receiver(int fd);

int llopen_transmitter(int fd);

int llwrite(int fd, unsigned char *buffer, int length);

int llread(int fd, char *buffer);

int llclose(int fd);

unsigned char *calcFinalMessage(unsigned char *data, int size, unsigned char C,
                                unsigned char BCC2);

void updateMessage(unsigned char *message, unsigned char answer);

unsigned char calcBCC2(unsigned char *data, int size);

unsigned char *stuffing(unsigned char *data, int dataSize, int *size);

int receiveSupervisionMessage(int fd, unsigned char C);

int sendSupervisionMessage(int fd, unsigned char C);

void setUpPort(int port, int *fd, struct termios *oldtio);

int stateMachineSupervisionMessage(enum state_t *state, unsigned char buf, unsigned char *C, unsigned char *COptions);

void closeFd(int fd, struct termios *oldtio);
