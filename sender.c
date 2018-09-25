/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <stdio.h>

#include "helpers.h"

#define MODEMDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

void readStdin(char* buf)
{
        printf("Write something here...\n");

        if(fgets(buf,255,stdin)==NULL)
        {
          printf("Error reading input!\n");
          exit(1);
        }
}

int main(int argc, char** argv)
{
    int fdWrite=0, fdRead=0;
    struct termios oldtio;
    char buf[255];

    usage(argc, argv);

    setUpSender(argv, fdWrite, &oldtio);

    readStdin(buf);

    writeSentence(fdWrite, buf);

    closeFd(fdWrite, &oldtio);

    setUpReceiver(argv, fdRead, &oldtio);

    readSentence(fdRead, buf);

    closeFd(fdWrite, &oldtio);

    return 0;
}
