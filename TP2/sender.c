#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"
#include "protocol.h"

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

unsigned char *getTLVLength(int fileLength)
{
  int size = sizeof(fileLength);
  unsigned char *TLV = malloc(2 + size);

  TLV[0] = T_LENGTH;
  TLV[1] = size;
  memcpy(TLV + 2, &fileLength, size);

  return TLV;
}

unsigned char* getTLVName(char *fileName, int stringLength)
{
  unsigned char *TLV = malloc(2 + stringLength);

  TLV[0] = T_LENGTH;
  TLV[1] = stringLength;
  memcpy(TLV + 2, fileName, stringLength);

  return TLV;
}

unsigned char *getDelimPackage(unsigned char C, int fileLength, char *fileName, int stringLength)
{
  unsigned char *delim = malloc((1 + 2 + 2 + stringLength + sizeof(fileLength)) * sizeof(unsigned char));

  delim[0] = C;

  unsigned char * TLVLength = getTLVLength(fileLength);
  unsigned char* TLVName = getTLVName(fileName, stringLength);

  memcpy(delim + 1, TLVLength, sizeof(*TLVLength));
  memcpy(delim + sizeof(TLVLength), TLVName, sizeof(*TLVName));

  free(TLVLength);
  free(TLVName);

  return delim;
}

unsigned char* getFragment(int seqNum, unsigned char* data, int K){
    unsigned char *fragment = malloc((4 + K) * sizeof(unsigned char));

    fragment[0] = F_C;
    fragment[1] = seqNum % 255;
    fragment[2] = K % 256;
    fragment[3] = K / 256;
    memcpy(fragment + 4, data, K);

    return fragment;
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

  if (llopen(TRANSMITTER, fd) == -1)
  {
    fprintf(stderr, "llopen error\n");
    exit(-1);
  }

  readStdin(buf);

  writeSentence(fd, buf);

  closeFd(fd, &oldtio);

  return 0;
}
