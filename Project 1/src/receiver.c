#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"
#include "receiver.h"
#include "utils.h"

int usage(char **argv)
{
  printf("Usage: %s <COM>\n", argv[0]);
  printf("ex: %s 0\n", argv[0]);

  return 1;
}

void handleStart(unsigned char *data, unsigned char *filename)
{
  unsigned char T = data[1];
  unsigned char size = data[2];
  int length;

  if (T == T_LENGTH)
  {
    memcpy(&length, data + 3, size);
    memcpy(filename, data + size + 3 + 2, data[3 + size + 1]);
  }
  else if (T == T_NAME)
  {
    memcpy(filename, data + 3, size);
    memcpy(&length, data + size + 3 + 2, data[3 + size + 1]);
  }
}

bool handleData(unsigned char *data, FILE *file)
{
  unsigned char C = data[0];

  if (C == F_C)
  {
    int K = 256 * data[3] + data[2];

    fwrite(data + 4, 1, K, file);
  }

  return C == END_C;
}

void normal(int fd, unsigned char *buffer)
{
  FILE *file;
  unsigned char filename[MAX_BUF_SIZE];
  bool end = false;
  int size = 0;
  llread(fd, buffer);

  handleStart(buffer, filename);
  file = fopen("copy", "wb+");

  while (!end)
  {
    size = llread(fd, buffer);

    printf("\n\n");

    end = handleData(buffer, file);
  }

  fclose(file);
}

int main(int argc, char **argv)
{
  int fd = 0, port = 0;
  struct termios oldtio;
  unsigned char buffer[400];

  if (argc != 2 || ((strcmp("0", argv[1]) != 0) && (strcmp("1", argv[1]) != 0)))
    return usage(argv);

  port = atoi(argv[1]);

  setUpPort(port, &fd, &oldtio);

  llopen(RECEIVER, fd);

  normal(fd, buffer);

  // llread(fd, buffer);

  // for (int i = 4; i < 10 + 4; i++)
  //   printf("0x%02X\n", buffer[i]);

  llclose(fd, RECEIVER);

  closeFd(fd, &oldtio);

  return 0;
}
