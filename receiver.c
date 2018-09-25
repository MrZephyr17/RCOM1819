/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#include "helpers.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

int main(int argc, char** argv)
{
  int fdRead=0, fdWrite=0;
  struct termios oldtio;
  char buf[255];

  usage(argc, argv);

  setUpReceiver(argv, fdRead, &oldtio);

  readSentence(fdRead, buf);

  closeFd(fdRead, &oldtio);

  setUpSender(argv, fdWrite, &oldtio);

  writeSentence(fdWrite, buf);

  closeFd(fdWrite, &oldtio);

  return 0;
}
