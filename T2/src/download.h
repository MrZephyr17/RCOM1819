#pragma once

#define MAX_BUF_SIZE 100
#define MAX_REPLY_SIZE 400
#define SOCKET_BUF_SIZE 1000
#define REPLY_CODE_SIZE 3
#define SERVER_PORT 21

typedef struct
{
    char serverName[MAX_BUF_SIZE];
    char filePath[MAX_BUF_SIZE];
    char user[MAX_BUF_SIZE];
    char pass[MAX_BUF_SIZE];
} info_t;

typedef enum
{
    READ_CODE,
    READ_LINE,
    READ_MULT_LINE,
    WAIT_FOR_PORT,
    FIRST_PORT_BYTE,
    SECOND_PORT_BYTE,
    END
} state_t;

typedef enum
{
    POSITIVE_PRE = 1,
    POSITIVE_INT,
    POSITIVE_COMP,
    TRANS_NEGATIVE_COMP,
    PERM_NEGATIVE_COMP
} reply_type_t;

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

int usage(char *argv[]);

int parseArgument(char *argument, info_t *info);

char *getServerIp(info_t info);

int createSocketTCP(char *server_ip, int server_port);

int readServerReply(int socketFd, char *reply);

int getServerPort(int socketFd);

int sendCommand(int socketFd, char *command, char *argument);

void createFile(int fd, char *filename);