#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */

volatile int STOP = false;

void readStdin(char *buf)
{
  printf("Write something here...\n");

  if (fgets(buf, 255, stdin) == NULL)
  {
    printf("Error reading input!\n");
    exit(1);
  }
}

int main(int argc, char **argv)
{
  int fd = 0;
  int port = 0;
  struct termios oldtio;
  char buf[255];

  usage(argc, argv);
  
  port = atoi(argv[1]);

  setUpPort(port, &fd, &oldtio);

  if(llopen(TRANSMITTER, fd) == -1){
    fprintf(stderr, "llopen error\n");
    exit(-1);
  }

  readStdin(buf);

  writeSentence(fd, buf);

  closeFd(fd, &oldtio);

  return 0;
}
