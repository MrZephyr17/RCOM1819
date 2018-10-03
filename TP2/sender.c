/*Non-Canonical Input Processing*/

#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */
volatile int STOP = FALSE;

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

  return 0;
}
