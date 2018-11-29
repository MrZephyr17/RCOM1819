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

/**
 * @brief 
 * 
 * @param argv 
 * @return int 
 */
int usage(char *argv[]);

/**
 * @brief 
 * 
 * @param argument 
 * @param info 
 * @return int 
 */
int parseArgument(char *argument, info_t *info);

/**
 * @brief Get the Server Ip object
 * 
 * @param info 
 * @return char* 
 */
char *getServerIp(info_t info);

/**
 * @brief Create a Socket T C P object
 * 
 * @param server_ip 
 * @param server_port 
 * @return int 
 */
int createSocketTCP(char *server_ip, int server_port);

/**
 * @brief 
 * 
 * @param socketFd 
 * @param reply 
 * @return int 
 */
int readServerReply(int socketFd, char *reply);

/**
 * @brief
 * 
 * @param socketFd 
 * @return int 
 */
int getServerPort(int socketFd);

/**
 * @brief 
 * 
 * @param socketFd 
 * @param command 
 * @param argument 
 * @return int 
 */
int sendCommand(int socketFd, char *command, char *argument);

/**
 * @brief 
 * 
 * @param fd 
 * @param filename 
 */
void createFile(int fd, char *filename);