#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"
#include "receiver.h"
#include "utils.h"

#define NUMBER_OF_TESTS 5

speed_t baud_rates[NUMBER_OF_TESTS] = {B38400, B2400, B4800, B9600, B19200};

int usage(char **argv) {
  printf("Usage: %s <COM>\n", argv[0]);
  printf("Option -t: allows to test effiency. Only one argument allowed, "
         "don\'t\nforget to use it on both sides, if necessary.\n");
  printf("    Arguments: FER - vary frame error ratio\n");
  printf("               C - vary baudrate\n");
  printf("               T_prop - vary processing time\n");
  printf("ex: %s 0\n", argv[0]);

  return 1;
}

void handleStart(unsigned char *data, unsigned char *filename) {
  unsigned char T = data[1];
  unsigned char size = data[2];
  int length;

  if (T == T_LENGTH) {
    memcpy(&length, data + 3, size);
    memcpy(filename, data + size + 3 + 2, data[3 + size + 1]);
  } else if (T == T_NAME) {
    memcpy(filename, data + 3, size);
    memcpy(&length, data + size + 3 + 2, data[3 + size + 1]);
  }
}

bool handleData(unsigned char *data, FILE *file) {
  unsigned char C = data[0];

  if (C == F_C) {
    int K = 256 * data[3] + data[2];

    fwrite(data + 4, 1, K, file);
  }

  return C == END_C;
}

void readFile(int fd) {
  FILE *file;
  unsigned char filename[MAX_BUF_SIZE + 4];
  unsigned char fragment[264];
  unsigned char delim[21];
  bool end = false;
  int size = 0;

  if (llread(fd, delim) <= 0) {
    fprintf(stderr, "llread error\n");
    exit(-1);
  }

  handleStart(delim, filename);

  file = fopen((char *)"copy.gif", "wb+");

  while (!end) {
    size = llread(fd, fragment);

    if (size == 0 || size == -1) {
      debug_print("llread error\n");
      continue;
    }

    if (size != -2)
      end = handleData(fragment, file);
  }

  fclose(file);
}

int receiveFile(char *port, int test, int i) {

  int fd = 0;
  struct termios oldtio;
  speed_t baudrate = B38400;

  if (test == 1)
    baudrate = baud_rates[i];

  debug_print("rate: %d\n", baudrate);

  setUpPort(atoi(port), &fd, &oldtio, baudrate);

  if (llopen(RECEIVER, fd) != 0) {
    fprintf(stderr, "llopen error\n");
    exit(-1);
  }

  readFile(fd);

  if (llclose(fd, RECEIVER) != 0) {
    fprintf(stderr, "llclose error\n");
    exit(-1);
  }

  closeFd(fd, &oldtio);

  return 0;
}

int processTestArgument(char **argv) {
  if (strcmp(argv[3], "C") == 0)
    return 1;

  if (strcmp(argv[3], "T_prop") == 0)
    return 2;

  if (strcmp(argv[3], "FER") == 0)
    return 3;

  return -1;
}

int main(int argc, char **argv) {
  int numTests = 1;
  int test = 0;

  if ((argc != 2 && argc != 4) ||
      ((strcmp("0", argv[1]) != 0) && (strcmp("1", argv[1]) != 0)))
    return usage(argv);
  else if (argc == 4 && strcmp(argv[2], "-t") != 0)
    return usage(argv);
  else if (argc == 4 && ((test = processTestArgument(argv)) == -1))
    return usage(argv);

  if (test == 1 || test == 2 || test == 3)
    numTests = NUMBER_OF_TESTS;

  for (int i = 0; i < numTests; i++)
    receiveFile(argv[1], i, test);

  return 0;
}
