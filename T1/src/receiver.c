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

void readFile(int fd)
{
  FILE *file;
  unsigned char filename[MAX_BUF_SIZE+4];
  unsigned char fragment[264];
  unsigned char delim[21];
  bool end = false;
  int size = 0;

  if (llread(fd, delim) <= 0)
  {
    fprintf(stderr, "llread error\n");
    exit(-1);
  }
  
  handleStart(delim, filename);
  
  // TODO: filename
  file = fopen("copy.gif" /*filename*/, "wb+");

  while (!end)
  {
    size = llread(fd, fragment);

    if (size <= 0)
    {
      debug_print("llread error\n");
      continue;
    }

    end = handleData(fragment, file);
  }

  fclose(file);
}

int main(int argc, char **argv)
{
  int fd = 0;
  struct termios oldtio;

  if (argc != 2 || ((strcmp("0", argv[1]) != 0) && (strcmp("1", argv[1]) != 0)))
    return usage(argv);

  setUpPort(atoi(argv[1]), &fd, &oldtio);

  if (llopen(RECEIVER, fd) != 0)
  {
    fprintf(stderr, "llopen error\n");
    exit(-1);
  }

  readFile(fd);

  if (llclose(fd, RECEIVER) != 0)
  {
    fprintf(stderr, "llclose error\n");
    exit(-1);
  }

  closeFd(fd, &oldtio);

  return 0;
}
