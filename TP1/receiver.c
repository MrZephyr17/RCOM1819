/*Non-Canonical Input Processing*/

#include <stdio.h>

#include "helpers.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */
volatile int STOP = FALSE;

int main(int argc, char **argv) {
  int fd = 0;
  struct termios oldtio;
  char buf[255];

  usage(argc, argv);

  setUpPort(argv, &fd, &oldtio);

  readSentence(&STOP, fd, buf);

  writeSentence(fd, buf);

  closeFd(fd, &oldtio);

  return 0;
}
