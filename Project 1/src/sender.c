#include "protocol.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define _POSIX_SOURCE 1 /* POSIX compliant source */

volatile int STOP = false;

void readStdin(char *buf) {
  printf("Write something here...\n");

  if (fgets(buf, 255, stdin) == NULL) {
    printf("Error reading input!\n");
    exit(1);
  }
}

int usage(char **argv) {
  printf("Usage: %s <COM> <FILENAME>\n", argv[0]);
  printf("ex: %s 0 pinguim.gif\n", argv[0]);

  return 1;
}

unsigned char *getTLVLength(int fileLength) {
  int size = sizeof(fileLength);
  unsigned char *TLV = malloc(2 + size);

  TLV[0] = T_LENGTH;
  TLV[1] = size;
  memcpy(TLV + 2, &fileLength, size);

  return TLV;
}

unsigned char *getTLVName(char *fileName, int stringLength) {
  unsigned char *TLV = malloc(2 + stringLength + 1);

  TLV[0] = T_LENGTH;
  TLV[1] = stringLength + 1;
  memcpy(TLV + 2, fileName, stringLength + 1);

  return TLV;
}

unsigned char *getDelimPackage(unsigned char C, int fileLength, char *fileName,
                               int stringLength, int *size) {
  *size = 1 + 2 + 2 + stringLength + 1 + sizeof(fileLength);
  unsigned char *delim = malloc(*size * sizeof(unsigned char));

  delim[0] = C;

  unsigned char *TLVLength = getTLVLength(fileLength);
  unsigned char *TLVName = getTLVName(fileName, stringLength);

  memcpy(delim + 1, TLVLength, 2 + sizeof(fileLength));

  memcpy(delim + 1 + 2 + sizeof(fileLength), TLVName, 2 + stringLength + 1);

  free(TLVLength);
  free(TLVName);

  return delim;
}

unsigned char *readImageFile(const char *fileName, off_t *size) {
  unsigned char *data;
  FILE *file;
  struct stat info;

  if ((file = fopen(fileName, "rb")) == NULL) {
    perror("fopen");
    exit(1);
  }

  if (stat(fileName, &info) != 0) {
    perror("stat");
    exit(1);
  }

  *size = info.st_size;
  data = malloc(*size);

  fread(data, sizeof(unsigned char), *size, file);

  fclose(file);

  return data;
}

unsigned char *getFragment(int seqNum, char *data, int K) {
  unsigned char *fragment = malloc((4 + K) * sizeof(unsigned char));

  fragment[0] = F_C;
  fragment[1] = seqNum % 255;
  fragment[2] = K % 256;
  fragment[3] = K / 256;
  memcpy(fragment + 4, data, K);

  // for (int i = 0; i < K + 4; i++)
  //   printf("0x%02X\n", fragment[i]);

  return fragment;
}

int main(int argc, char **argv) {
  int fd = 0;
  int port = 0;
  struct termios oldtio;
  char buf[255];
  char filename[MAX_BUF_SIZE];
  unsigned char *fileData;
  off_t fileSize = 0;
  int delimSize = 0;

  if ((argc != 3) ||
      ((strcmp("0", argv[1]) != 0) && (strcmp("1", argv[1]) != 0)))
    usage(argv);

  port = atoi(argv[1]);
  strcpy(filename, argv[2]);
  fileData = readImageFile(filename, &fileSize);

  // setUpPort(port, &fd, &oldtio);

  // if (llopen(TRANSMITTER, fd) == -1)
  // {
  //   fprintf(stderr, "llopen error\n");
  //   exit(-1);
  // }

  unsigned char *start = getDelimPackage(START_C, fileSize, filename,
                                         strlen(filename), &delimSize);

  // for (int i = 0; i < delimSize; i++)
  //   printf("%d - 0x%02X\n", i, start[i]);

  unsigned char *end = malloc(delimSize * sizeof(unsigned char));
  memcpy(end, start, delimSize);
  end[0] = END_C;

  for (int i = 0; i < delimSize; i++)
    printf("%d - 0x%02X\n", i, end[i]);

  unsigned char *fragment = getFragment(0, "ok", 3);

  free(start);
  free(end);
  free(fragment);
  // free(fileData);

  // readStdin(buf);

  // writeSentence(fd, buf);

  // closeFd(fd, &oldtio);

  return 0;
}
