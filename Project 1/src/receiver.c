#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"
#include "receiver.h"
#include "utils.h"

int usage(char **argv) {
  printf("Usage: %s <COM>\n", argv[0]);
  printf("ex: %s 0\n", argv[0]);

  return 1;
}

bool handleData(char *data) {
  unsigned char C = data[0];
  FILE *file = NULL;

  if (C == START_C) {
    unsigned char T = data[1];
    unsigned char size = data[2];
    char *filename = malloc(size);
    int length;

    if (T == T_LENGTH) {
      memcpy(&length, data + 3, size);
      memcpy(filename, data + size + 3 + 2, data[3 + size + 1]);
    } else if (T == T_NAME) {
      memcpy(filename, data + 3, size);
      memcpy(&length, data + size + 3 + 2, data[3 + size + 1]);
    }

    file = fopen(filename, "w");
  } else if (C == F_C) {
    int K = 256 * data[2] + data[1];

    fwrite(data + 4, 1, K, file);
  } else if (C == END_C)
    return true;

  return false;
}

int main(int argc, char **argv) {
  int fd = 0, port = 0;
  struct termios oldtio;
  char buffer[MAX_BUF_SIZE];
  bool end = false;

  if (argc != 2 || ((strcmp("0", argv[1]) != 0) && (strcmp("1", argv[1]) != 0)))
    return usage(argv);

  port = atoi(argv[1]);

  setUpPort(port, &fd, &oldtio);

  llopen(RECEIVER, fd);

  while (!end) {
    llread(fd, buffer);
    end = handleData(buffer);
  }

  llclose(fd, RECEIVER);

  closeFd(fd, &oldtio);

  return 0;
}
