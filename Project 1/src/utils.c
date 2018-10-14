#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */

bool findByteOnArray(unsigned char byte, unsigned char *array)
{
  for (int i = 0; (array + i) != NULL; i++)
  {
    if (array[i] == byte)
      return true;
  }

  return false;
}

void readSentence(volatile int *STOP, int fd, char *buf)
{
  int i = 0;
  int res;

  while (*STOP == false)
  {                             /* loop for input */
    res = read(fd, buf + i, 1); /* returns after 5 chars have been input */
    if (res > 0)
    {
      if (buf[i] == '\0')
        *STOP = true;

      i++;
    }
  }

  printf("%s\n", buf);
}

void writeSentence(int fd, char *buf)
{
  int res;

  res = write(fd, buf, strlen(buf) + 1);
  tcflush(fd, TCIOFLUSH);
  printf("%d bytes written\n", res);
}