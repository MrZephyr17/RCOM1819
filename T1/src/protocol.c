#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "protocol.h"
#include "utils.h"

bool transmissionFlag = false;
int transmissionCounter = 0;

int flag = 0; 

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

int llopen_transmitter(int fd)
{
    bool received = false;
    unsigned char buf, C;
    unsigned char COptions[1] = {UA_C};

    setUpAlarmHandler();

    do
    {
        sendSupervisionMessage(fd, A_03, SET_C);
        alarm(TIME_OUT);
        transmissionFlag = false;
        int res = 0;
        state_t state = START;

        while (!received && !transmissionFlag)
        {
            res = read(fd, &buf, 1);

            if (res > 0)
                stateMachineSupervisionMessage(&state, buf, A_03, &C, COptions);

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

int llopen_receiver(int fd)
{
    if (receiveSupervisionMessage(fd, A_03, SET_C))
    {
        debug_print("Received SET_C\n");

        sendSupervisionMessage(fd, A_03, UA_C);

        return 0;
    }

    return -1;
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
    leitura do(s) proximo(s) caracter(es)
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

int llwrite(int fd, unsigned char *buffer, int length)
{
    int dataSize = 0, res = 0;
    bool received = false;
    unsigned char buf;
    unsigned char COptions[] = {RR0, RR1, REJ0, REJ1};
    unsigned char answer = 0;
    unsigned char BCC2 = calcBCC2(buffer, length);

    unsigned char *dataStuffed = stuffing(buffer, length, &dataSize);

    unsigned char *finalMessage = calcFinalMessage(dataStuffed, dataSize, BCC2);

    free(dataStuffed);

    do
    {
        res = write(fd, finalMessage, dataSize + 6);
        alarm(TIME_OUT);
        transmissionFlag = false;
        int nRead = 0;
        state_t state = START;

        while (!received && !transmissionFlag)
        {
            nRead = read(fd, &buf, 1);

            if (nRead > 0)
                stateMachineSupervisionMessage(&state, buf, A_03, &answer, COptions);

            if (state == END && ((flag == 1 && answer == RR0) ||
                                 (flag == 0 && answer == RR1)))
            {
                received = true;
                debug_print("Received RR\n");
                alarm(0);
                flag ^= 1;
            }
            else if (state == C_RCV && (answer == REJ0 || answer == REJ1))
            {
                debug_print("Received REJ\n");
                alarm(0);
                transmissionFlag = true;
            }
        }
    } while (transmissionFlag && transmissionCounter < MAX_RETRY_NUMBER);

    free(finalMessage);

    if (transmissionFlag && transmissionCounter == MAX_RETRY_NUMBER)
        return -1;

    transmissionFlag = false;
    transmissionCounter = 0;

    return res;
}

unsigned char *stuffing(unsigned char *data, int dataSize, int *size)
{
    int stuffedSize = dataSize;
    unsigned char *dataStuffed = malloc(stuffedSize * sizeof(unsigned char));
    int i = 0, j = 0;

    for (; i < dataSize; i++, j++)
    {
        if (data[i] == FLAG)
        {
            dataStuffed =
                realloc(dataStuffed, (++stuffedSize) * sizeof(unsigned char));
            dataStuffed[j++] = ESC;
            dataStuffed[j] = ESC_2;
        }
        else if (data[i] == ESC)
        {
            dataStuffed =
                realloc(dataStuffed, (++stuffedSize) * sizeof(unsigned char));
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

unsigned char *calcFinalMessage(unsigned char *data, int size,
                                unsigned char BCC2)
{
    unsigned char *finalMessage = malloc((size + 6) * sizeof(unsigned char));
    unsigned char C = flag == 0 ? C_I0 : C_I1;

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

int llread(int fd, unsigned char *buffer)
{
    int size = 0;

    receiveIMessage(fd, &size, buffer);

    if (buffer == NULL || size <= 0)
        return -1;

    return size;
}

bool checkBBC2(unsigned char rec_BCC2, unsigned char *data, int size)
{
    unsigned char exp_BCC2 = calcBCC2(data, size);

    return rec_BCC2 == exp_BCC2;
}

void receiveData(int fd, unsigned char buf, unsigned char *data, int *i, state_t *state, bool *wait)
{
    if (buf == ESC)
    {
        *wait = true;
        return;
    }

    if (buf == FLAG)
    {
        unsigned char bcc2 = data[*i - 1];

        unsigned char answer;
        if (checkBBC2(bcc2, data, *i - 1))
        {
            *state = END;

            answer = flag == 0 ? RR1 : RR0;
            sendSupervisionMessage(fd, A_03, answer);
        }
        else
        {
            *state = START;
            answer = flag == 0 ? REJ1 : REJ0;
            sendSupervisionMessage(fd, A_03, answer);
        }

        debug_print("Sent 0x%02X\n", answer);
    }

    if (*wait)
    {
        if (buf == ESC_2)
            data[(*i)++] = FLAG;
        else if (buf == ESC_3)
            data[(*i)++] = ESC;

        *wait = false;
    }
    else
        data[(*i)++] = buf;
}

void receiveIMessage(int fd, int *size, unsigned char *data)
{
    state_t state = START;
    int res = 0;
    unsigned char buf;
    int i = 0;
    unsigned char C;
    unsigned char COptions[2] = {C_I0, C_I1};
    bool wait = false;

    while (state != END)
    {
        res = read(fd, &buf, 1);

        if (res <= 0)
            continue;

        switch (state)
        {
        case START:
            if (buf == FLAG)
                state = FLAG_RCV;
            i = 0;
            break;
        case FLAG_RCV:
            if (buf == A_03)
                state = A_RCV;
            else if (buf != FLAG)
                state = START;
            break;
        case A_RCV:
            if (findByteOnArray(buf, COptions))
            {
                state = C_RCV;
                C = buf;
            }
            else if (buf == FLAG)
                state = FLAG_RCV;
            else if (buf != FLAG)
                state = START;
            break;
        case C_RCV:
            if (buf == (A_03 ^ C))
                state = BCC1_OK;
            else
            {
                state = START; 
                unsigned char answer = flag == 0 ? REJ1 : REJ0;
                sendSupervisionMessage(fd, A_03, answer);
            }
            break;
        case BCC1_OK:
            receiveData(fd, buf, data, &i, &state, &wait);
            break;
        case BCC2_OK:
            if (buf == FLAG)
                state = END;
            else
                state = BCC1_OK;
            break;
        case END:
            break;
        default:
            fprintf(stderr, "Invalid state\n");
            return;
        }
    }

    *size = i - 2;
    flag ^= 1;
}

int llclose(int fd, int flag)
{
    switch (flag)
    {
    case RECEIVER:
        return llclose_receiver(fd);
    case TRANSMITTER:
        return llclose_transmitter(fd);
    default:
        fprintf(stderr, "Invalid flag argument!\n");
        return -1;
    }

    return 0;
}

int llclose_receiver(int fd)
{
    if (receiveSupervisionMessage(fd, A_03, DISC))
    {
        debug_print("Received DISC\n");

        sendSupervisionMessage(fd, A_01, DISC);

        if (receiveSupervisionMessage(fd, A_01, UA_C))
        {
            debug_print("Received UA_C\n");
        }

        return 0;
    }

    return -1;
}

int llclose_transmitter(int fd)
{
    bool received = false;
    unsigned char buf, C;
    unsigned char COptions[1] = {DISC};

    do
    {
        sendSupervisionMessage(fd, A_03, DISC);
        alarm(TIME_OUT);
        transmissionFlag = false;
        int res = 0;
        state_t state = START;

        while (!received && !transmissionFlag)
        {
            res = read(fd, &buf, 1);

            if (res > 0)
                stateMachineSupervisionMessage(&state, buf, A_01, &C, COptions);

            if (state == END)
            {
                alarm(0);
                received = true;
                debug_print("Received DISC\n");
            }
        }
    } while (transmissionFlag && transmissionCounter < MAX_RETRY_NUMBER);

    if (transmissionFlag && transmissionCounter == MAX_RETRY_NUMBER)
        return -1;

    transmissionFlag = false;
    transmissionCounter = 0;

    sendSupervisionMessage(fd, A_01, UA_C);

    sleep(1);

    return 0;
}

int receiveSupervisionMessage(int fd, unsigned char A, unsigned char C)
{
    state_t state = START;
    int res = 0;
    unsigned char buf;
    unsigned char COptions[1] = {C};

    while (state != END)
    {
        res = read(fd, &buf, 1);

        if (res > 0)
            stateMachineSupervisionMessage(&state, buf, A, &C, COptions);
    }

    return 1;
}

int sendSupervisionMessage(int fd, unsigned char A, unsigned char C)
{
    unsigned char message[SUPERVISION_SIZE] = {FLAG, A, C, A ^ C, FLAG};

    return write(fd, message, SUPERVISION_SIZE) > 0;
}

int stateMachineSupervisionMessage(state_t *state, unsigned char buf,
                                   unsigned char A, unsigned char *C,
                                   unsigned char *COptions)
{
    switch (*state)
    {
    case START:
        if (buf == FLAG)
            *state = FLAG_RCV;
        break;
    case FLAG_RCV:
        if (buf == A)
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
        if (buf == (A ^ *C))
            *state = BCC1_OK;
        else if (buf == FLAG)
            *state = FLAG_RCV;
        else if (buf != FLAG)
            *state = START;
        break;
    case BCC1_OK:
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

void closeFd(int fd, struct termios *oldtio)
{
    if (tcsetattr(fd, TCSANOW, oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    close(fd);
}
