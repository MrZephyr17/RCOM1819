/*Non-Canonical Input Processing*/

#include <stdio.h>

#include "helpers.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */
volatile int STOP = FALSE;

int main(int argc, char **argv) {

  int fd = 0, port = 0;
  struct termios oldtio;
  char buf[255];

  usage(argc, argv);

  port = atoi(argv[1]);

  setUpPort(port, &fd, &oldtio);

  llopen(RECEIVER, fd);

  readSentence(&STOP, fd, buf);

  closeFd(fd, &oldtio);

  return 0;
}
