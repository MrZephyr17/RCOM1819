#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "protocol.h"
#include "helpers.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */

int transmissionFlag = false, transmissionCounter = 0;

void alarm_handler()
{
    transmissionFlag = true;
    ++transmissionCounter;
    debug_print("Received alarm signal\n");
}

void setUpAlarmHandler()
{
    struct sigaction action;

    action.sa_handler = alarm_handler;
    sigemptyset(&action.sa_mask); // all signals are delivered
    action.sa_flags = 0;
    sigaction(SIGALRM, &action, NULL);

    debug_print("Installed alarm handler\n");
}

int llopen_receiver(int fd)
{
    if (receiveSupervisionMessage(fd, SET_C))
    {
        debug_print("Received SET_C\n");

        sendSupervisionMessage(fd, UA_C);
    }

    return 0;
}

int llopen_transmitter(int fd)
{
    int received = 0;

    setUpAlarmHandler();

    unsigned char buf, C;
    unsigned char COptions[1] = {UA_C};

    do
    {
        sendSupervisionMessage(fd, SET_C);
        alarm(TIME_OUT);
        transmissionFlag = false;
        int res = 0;
        enum state_t state = START;

        while (!received && !transmissionFlag)
        {
            res = read(fd, &buf, 1);

            if (res > 0)
                stateMachineSupervisionMessage(&state, buf, &C, COptions);

            if (state == END)
            {
                alarm(0);
                received = true;
                debug_print("Received UA_C\n");
            }
        }
    } while (transmissionFlag && transmissionCounter < MAX_RETRY_NUMBER);

    if (transmissionFlag && transmissionCounter == MAX_RETRY_NUMBER)
        return -1;

    transmissionFlag = false;
    transmissionCounter = 0;

    return 0;
}

int llopen(int flag, int fd)
{
    switch (flag)
    {
    case RECEIVER:
        return llopen_receiver(fd);
    case TRANSMITTER:
        return llopen_transmitter(fd);
    default:
        fprintf(stderr, "Invalid flag argument!\n");
        return -1;
    }

    return 0;
}

int llwrite(int fd, unsigned char *buffer, int length, unsigned char *answer)
{

    int dataSize = 0, received = 0;
    unsigned char C;
    unsigned char buf;
    unsigned char COptions[] = {RR0, RR1, REJ0, REJ1};

    unsigned char BCC2 = calcBCC2(buffer, length);

    unsigned char *dataStuffed = stuffing(buffer, length, &dataSize);

    free(dataStuffed);

    if (answer == NULL || *answer == 0)
        C = C_I0;
    else
        C = C_I1;

    unsigned char *finalMessage = calcFinalMessage(buffer, dataSize, C, BCC2);

    do
    {
        write(fd, finalMessage, dataSize + 6);
        alarm(TIME_OUT);
        transmissionFlag = false;
        int res = 0;
        enum state_t state = START;

        while (!received && !transmissionFlag)
        {
            res = read(fd, &buf, 1);

            if (res > 0)
              stateMachineSupervisionMessage(&state, buf, answer, COptions);

            if (state == END)
            {
                alarm(0);
                received = true;
                debug_print("Received RR\n");
            }
            else if (state == C_RCV &&
                     (*answer == REJ0 || *answer == REJ1))
            {
                transmissionCounter++;
                transmissionFlag = true;
                break;
            }
        }
    } while (transmissionFlag && transmissionCounter < MAX_RETRY_NUMBER);

    if (transmissionFlag && transmissionCounter == MAX_RETRY_NUMBER)
        return -1;

    free(finalMessage);

    transmissionFlag = false;
    transmissionCounter = 0;
}

int llread(int fd, char *buffer)
{
}

int llclose(int fd) {}

unsigned char *stuffing(unsigned char *data, int dataSize, int *size)
{
    int stuffedSize = dataSize;
    unsigned char *dataStuffed =
        malloc(stuffedSize * sizeof(unsigned char));
    int i = 0, j = 0;

    for (; i < dataSize; i++, j++)
    {
        if (data[i] == FLAG)
        {
            dataStuffed = realloc(dataStuffed, (++stuffedSize) * sizeof(unsigned char));
            dataStuffed[j++] = ESC;
            dataStuffed[j] = ESC_2;
        }
        else if (data[i] == ESC)
        {
            dataStuffed = realloc(dataStuffed, (++stuffedSize) * sizeof(unsigned char));
            dataStuffed[j++] = ESC;
            dataStuffed[j] = ESC_3;
        }
        else
        {
            dataStuffed[j] = data[i];
        }
    }

    *size = stuffedSize;

    return dataStuffed;
}

unsigned char calcBCC2(unsigned char *data, int size)
{
    unsigned char BCC2 = data[0];
    int i;

    for (i = 1; i < size; ++i)
        BCC2 ^= data[i];

    return BCC2;
}

unsigned char *calcFinalMessage(unsigned char *data, int size, unsigned char C,
                                unsigned char BCC2)
{
    unsigned char *finalMessage = malloc((size + 6) * sizeof(unsigned char));

    finalMessage[0] = FLAG;
    finalMessage[1] = A_03;
    finalMessage[2] = C;
    finalMessage[3] = A_03 ^ C;

    int i = 0;

    for (; i < size; i++)
        finalMessage[i + 4] = data[i];

    i += 4;

    finalMessage[i++] = BCC2;
    finalMessage[i++] = FLAG;

    return finalMessage;
}

int receiveSupervisionMessage(int fd, unsigned char C)
{
    enum state_t state = START;
    int res = 0;
    unsigned char buf;
    unsigned char COptions[1] = {C};

    while (state != END)
    {
        res = read(fd, &buf, 1);

        if (res > 0)
          stateMachineSupervisionMessage(&state, buf, &C, COptions);
    }

    return 1;
}

int sendSupervisionMessage(int fd, unsigned char C)
{
    unsigned char message[SUPERVISION_SIZE] = {FLAG, A_03, C, A_03 ^ C, FLAG};

    return write(fd, message, SUPERVISION_SIZE) > 0;
}

int stateMachineSupervisionMessage(enum state_t *state, unsigned char buf, unsigned char *C, unsigned char *COptions)
{
    switch (*state)
    {
    case START:
        if (buf == FLAG)
            *state = FLAG_RCV;
        break;
    case FLAG_RCV:
        if (buf == A_03)
            *state = A_RCV;
        else if (buf != FLAG)
            *state = START;
        break;
    case A_RCV:
        if (findByteOnArray(buf, COptions))
        {
            *state = C_RCV;
            *C = buf;
        }
        else if (buf == FLAG)
            *state = FLAG_RCV;
        else if (buf != FLAG)
            *state = START;
        break;
    case C_RCV:
        if (buf == (A_03 ^ *C))
            *state = BCC_OK;
        else if (buf == FLAG)
            *state = FLAG_RCV;
        else if (buf != FLAG)
            *state = START;
        break;
    case BCC_OK:
        if (buf == FLAG)
            *state = END;
        else
            *state = START;
        break;
    case END:
        break;
    default:
        fprintf(stderr, "Invalid state\n");
        return -1;
    }

    return 0;
}

void setUpPort(int port, int *fd, struct termios *oldtio)
{
    struct termios newtio;
    char *portPath = malloc(PORT_SIZE);

    if (port == 0)
        strcpy(portPath, "/dev/ttyS0");
    else
        strcpy(portPath, "/dev/ttyS1");

    *fd = open(portPath, O_RDWR | O_NOCTTY);
    if (*fd < 0)
    {
        perror(portPath);
        exit(-1);
    }

    free(portPath);

    if (tcgetattr(*fd, oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

    /*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(*fd, TCIOFLUSH);

    if (tcsetattr(*fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    sleep(1);

    debug_print("New termios structure set\n");
}

void closeFd(int fd, struct termios *oldtio)
{
    if (tcsetattr(fd, TCSANOW, oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    close(fd);
}
