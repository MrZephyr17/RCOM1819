/*Non-Canonical Input Processing*/

#include <stdio.h>

#include "helpers.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */
volatile int STOP = FALSE;

int main(int argc, char **argv) {
  int fdRead = 0, fdWrite = 0;
  struct termios oldtio;
  char buf[255];

  usage(argc, argv);

  setUpReceiver(argv, &fdRead, &oldtio);

  readSentence(&STOP, fdRead, buf);

  closeFd(fdRead, &oldtio);

  setUpSender(argv, &fdWrite, &oldtio);

  writeSentence(fdWrite, buf);

  closeFd(fdWrite, &oldtio);

  return 0;
}
