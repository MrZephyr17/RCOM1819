#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUF_SIZE 100

typedef struct
{
    char serverName[MAX_BUF_SIZE];
    char filePath[MAX_BUF_SIZE];
    char user[MAX_BUF_SIZE];
    char pass[MAX_BUF_SIZE];
} info_t;

int usage(char *argv[])
{
    printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
    return 1;
}

int parseArgument(char *argument, info_t *info)
{
    char start[7];
    strncpy(start, argument, 6);

    char *sep;
    int index1 = 6, index2;

    if (strcmp("ftp://", start) != 0)
        return 1;

    if ((sep = strchr(argument + 6, ':')) != NULL)
    {
        int index;

        index = (int)(sep - argument);

        strncpy(info->user, argument + 6, index - 6);

        if ((sep = strchr(argument, '@')) == NULL)
            return 1;

        int new_index = (int)(sep - argument);
        index++;
        strncpy(info->pass, argument + index, new_index - index);

        index1 = ++new_index;
    }
    else if ((sep = strchr(argument, '@')) != NULL)
        return 1;

    if ((sep = strchr(argument + 6, '/')) == NULL)
        return 1;

    index2 = (int)(sep - argument);

    strncpy(info->serverName, argument + index1, index2 - index1);
    index2++;
    strncpy(info->filePath, argument + index2, strlen(argument) - index2);

    return 0;
}

int main(int argc, char *argv[])
{
    info_t info;

    if (argc != 2 || parseArgument(argv[1], &info) != 0)
        return usage(argv);

    struct hostent *h;

    if ((h = gethostbyname(info.serverName)) == NULL)
    {
        herror("gethostbyname");
        exit(1);
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)h->h_addr_list)));

    return 0;
}