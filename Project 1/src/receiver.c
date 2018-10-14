#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "protocol.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */

volatile int STOP = false;

int usage(char **argv)
{
  printf("Usage: %s <COM>\n", argv[0]);
  printf("ex: %s 0\n", argv[0]);

  return 1;
}

int main(int argc, char **argv)
{
  int fd = 0, port = 0;
  struct termios oldtio;
  char buf[255];

  if (argc != 2 || ((strcmp("0", argv[1]) != 0) && (strcmp("1", argv[1]) != 0)))
    return usage(argv);

  port = atoi(argv[1]);

  setUpPort(port, &fd, &oldtio);

  llopen(RECEIVER, fd);

  readSentence(&STOP, fd, buf);

  closeFd(fd, &oldtio);

  return 0;
}
