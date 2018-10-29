#pragma once

#include <stdbool.h>
#include <termios.h>

typedef enum
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC1_OK,
    DATA_RCV,
    BCC2_OK,
    END
} state_t;

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
#define DISC 0x0B

#define TIME_OUT 3
#define MAX_RETRY_NUMBER 3

#define BAUDRATE B38400
#define SUPERVISION_SIZE 5
#define PORT_SIZE 11

/**
 * @brief 
 * 
 */
void alarm_handler();

/**
 * @brief Set the Up Alarm Handler object
 * 
 */
void setUpAlarmHandler();

/**
 * @brief 
 * 
 * @param flag 
 * @param fd 
 * @return int 
 */
int llopen(int flag, int fd);

/**
 * @brief 
 * 
 * @param fd 
 * @return int 
 */
int llopen_transmitter(int fd);

/**
 * @brief 
 * 
 * @param fd 
 * @return int 
 */
int llopen_receiver(int fd);

/**
 * @brief Set the Up Port object
 * 
 * @param port 
 * @param fd 
 * @param oldtio 
 */
void setUpPort(int port, int *fd, struct termios *oldtio);

/**
 * @brief 
 * 
 * @param fd 
 * @param buffer 
 * @param length 
 * @return int 
 */
int llwrite(int fd, unsigned char *buffer, int length);

/**
 * @brief 
 * 
 * @param data 
 * @param dataSize 
 * @param size 
 * @return unsigned char* 
 */
unsigned char *stuffing(unsigned char *data, int dataSize, int *size);

/**
 * @brief 
 * 
 * @param data 
 * @param size 
 * @return unsigned char 
 */
unsigned char calcBCC2(unsigned char *data, int size);

/**
 * @brief 
 * 
 * @param data 
 * @param size 
 * @param BCC2 
 * @return unsigned char* 
 */
unsigned char *calcFinalMessage(unsigned char *data, int size,
                                unsigned char BCC2);

/**
 * @brief 
 * 
 * @param fd 
 * @param buffer 
 * @return int 
 */
int llread(int fd, unsigned char *buffer);

/**
 * @brief 
 * 
 * @param rec_BCC2 
 * @param data 
 * @param size 
 * @return true 
 * @return false 
 */
bool checkBCC2(unsigned char rec_BCC2, unsigned char *data, int size);

/**
 * @brief 
 * 
 * @param fd 
 * @param buf 
 * @param data 
 * @param i 
 * @param state 
 * @param wait 
 */
void receiveData(int fd, unsigned char buf, unsigned char *data, int *i, state_t *state, bool *wait);

/**
 * @brief 
 * 
 * @param fd 
 * @param size 
 * @return unsigned char* 
 */
int receiveIMessage(int fd, int *size, unsigned char* data);


/**
 * @brief 
 * 
 * @param fd 
 * @param flag 
 * @return int 
 */
int llclose(int fd, int flag);

/**
 * @brief 
 * 
 * @param fd 
 * @return int 
 */
int llclose_receiver(int fd);

/**
 * @brief 
 * 
 * @param fd 
 * @return int 
 */
int llclose_transmitter(int fd);

/**
 * @brief 
 * 
 * @param fd 
 * @param A 
 * @param C 
 * @return int 
 */
int receiveSupervisionMessage(int fd, unsigned char A, unsigned char C);

/**
 * @brief 
 * 
 * @param fd 
 * @param A 
 * @param C 
 * @return int 
 */
int sendSupervisionMessage(int fd, unsigned char A, unsigned char C);

/**
 * @brief 
 * 
 * @param state 
 * @param buf 
 * @param A 
 * @param C 
 * @param COptions 
 * @return int 
 */
int stateMachineSupervisionMessage(state_t *state, unsigned char buf,
                                   unsigned char A, unsigned char *C,
                                   unsigned char *COptions);

/**
 * @brief 
 * 
 * @param fd 
 * @param oldtio 
 */
void closeFd(int fd, struct termios *oldtio);