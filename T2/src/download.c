#include <stdio.h>

typedef struct
{
    char *serverName;
    char *filePath;
    char *user;
    char *pass;
} info_t;

int usage(char *argv[])
{
    printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>", argv[0]);
    return 1;
}

int parseArgument(char *argument, info_t *info)
{
    (void)argument;
    (void)info;
    return 0;
}

int main(int argc, char *argv[])
{   
    info_t info;

    if (argc != 2 || parseArgument(argv[1], &info) != 0)
        return usage(argv);

    return 0;
}