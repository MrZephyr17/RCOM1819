#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "protocol.h"
#include "sender.h"
#include "utils.h"

speed_t baud_rates[NUMBER_OF_TESTS] = {B2400, B4800, B9600, B19200, B38400};
int messageSizes[NUMBER_OF_TESTS] = {32, 64, 128, 256, 512};
test_t test = INV;
int test_no = 0;

int usage(char **argv)
{
  printf("Usage: %s <COM> <FILENAME> [-t ...]\n", argv[0]);
  printf("Option -t: allows to test effiency. Only one argument allowed, don\'t\nforget to use it on both sides.\n");
  printf("    Arguments: I - vary I message length\n");
  printf("               C - vary baudrate\n");
  printf("               T_prop - vary processing time\n");
  printf("               FER - vary frame error ratio\n");
  printf("ex: %s 0 pinguim.gif\n", argv[0]);

  return 1;
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

unsigned char *getTLVName(char *fileName, int stringLength)
{
  unsigned char *TLV = malloc(2 + stringLength + 1);

  TLV[0] = T_LENGTH;
  TLV[1] = stringLength + 1;
  memcpy(TLV + 2, fileName, stringLength + 1);

  return TLV;
}

unsigned char *getDelimPackage(unsigned char C, int fileLength, char *fileName,
                               int stringLength, int *size)
{
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

unsigned char *readFile(const char *fileName, off_t *size)
{
  unsigned char *data;
  FILE *file;
  struct stat info;

  if ((file = fopen(fileName, "rb+")) == NULL)
  {
    perror("fopen");
    exit(1);
  }

  if (stat(fileName, &info) != 0)
  {
    perror("stat");
    exit(1);
  }

  *size = info.st_size;
  data = malloc(*size);

  fread(data, sizeof(unsigned char), *size, file);

  fclose(file);

  return data;
}

unsigned char *getFragment(int seqNum, unsigned char *data, int K)
{
  unsigned char *fragment = malloc((4 + K) * sizeof(unsigned char));

  fragment[0] = F_C;
  fragment[1] = seqNum % 255;
  fragment[2] = K % 256;
  fragment[3] = K / 256;
  memcpy(fragment + 4, data, K);

  return fragment;
}

void writeFile(int fd, char *filename, int messageSize, unsigned char *fileData, off_t fileSize)
{
  int delimSize = 0;

  unsigned char *start = getDelimPackage(START_C, fileSize, filename,
                                         strlen(filename), &delimSize);

  if (llwrite(fd, start, delimSize) <= 0)
  {
    fprintf(stderr, "llwrite error\n");
    exit(-1);
  }

  int rest = fileSize % messageSize;
  int numPackages = fileSize / messageSize;

  unsigned char *fragment;

  int i = 0;
  for (; i < numPackages; i++)
  {
    fragment = getFragment(i, fileData + messageSize * i, messageSize);

    if (llwrite(fd, fragment, messageSize + 4) <= 0)
    {
      fprintf(stderr, "llwrite error\n");
      exit(-1);
    }
  }

  if (rest != 0)
  {
    fragment = getFragment(i, fileData + messageSize * i, rest);
    if (llwrite(fd, fragment, rest + 4) <= 0)
    {
      fprintf(stderr, "llwrite error\n");
      exit(-1);
    }
  }

  unsigned char *end = malloc(delimSize * sizeof(unsigned char));
  memcpy(end, start, delimSize);
  end[0] = END_C;

  if (llwrite(fd, end, delimSize) <= 0)
  {
    fprintf(stderr, "llwrite error\n");
    exit(-1);
  }

  free(start);
  free(end);
  free(fragment);
}

int transferFile(char *fileName, char *port, unsigned char *fileData, off_t fileSize)
{
  int fd = 0;
  struct termios oldtio;
  int messageSize = FRAG_K;
  speed_t baudrate = B38400;

  if (test == I)
    messageSize = messageSizes[test_no];
  else if (test == C)
    baudrate = baud_rates[test_no];

  printf("rate: %d\n", baudrate);
  printf("I size: %d\n", messageSize);

  setUpPort(atoi(port), &fd, &oldtio, baudrate);

  if (llopen(TRANSMITTER, fd) != 0)
  {
    fprintf(stderr, "llopen error\n");
    exit(-1);
  }

  writeFile(fd, fileName, messageSize, fileData, fileSize);

  if (llclose(fd, TRANSMITTER) != 0)
  {
    fprintf(stderr, "llclose error\n");
    exit(-1);
  }

  closeFd(fd, &oldtio);

  return 0;
}

int main(int argc, char **argv)
{
  int numTests = 1;
  off_t fileSize = 0;
  unsigned char *fileData;

  if ((argc != 3 && argc != 5) ||
      ((strcmp("0", argv[1]) != 0) &&
       (strcmp("1", argv[1]) != 0)))
    return usage(argv);
  else if (argc == 5 && strcmp("-t", argv[3]) != 0)
    return usage(argv);
  else if (argc == 5 && ((test = processTestArgument(argv, 4)) == INV))
    return usage(argv);

  if (test > INV)
    numTests = NUMBER_OF_TESTS;

  fileData = readFile(argv[2], &fileSize);

  for (; test_no < numTests; test_no++)
    transferFile(argv[2], argv[1], fileData, fileSize);

  if (test > INV)
    printf("Please consult test results on stats.txt\n");

  free(fileData);

  return 0;
}