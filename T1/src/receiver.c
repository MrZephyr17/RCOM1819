#include "receiver.h"
#include "protocol.h"
#include "utils.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int length = 0, test_no = 0;
test_t test = INV;
useconds_t delays[NUMBER_OF_TESTS] = {50000, 100000, 150000, 200000, 250000};
speed_t baud_rates[NUMBER_OF_TESTS] = {B2400, B4800, B9600, B19200, B38400};
const char *baud_rates_s[NUMBER_OF_TESTS] = {"B2400", "B4800", "B9600", "B19200", "B38400"};
int messageSizes[NUMBER_OF_TESTS] = {32, 64, 128, 256, 512};

extern int fail_prob[NUMBER_OF_TESTS];

int usage(char **argv)
{
  printf("Usage: %s <COM>\n", argv[0]);
  printf("Option -t: allows to test effiency. Only one argument allowed, "
         "don\'t\nforget to use it on both sides.\n");
  printf("    Arguments: I - vary I message length\n");
  printf("               C - vary baudrate\n");
  printf("               T_prop - vary processing time\n");
  printf("               FER - vary frame error ratio\n");
  printf("ex: %s 0\n", argv[0]);

  return 1;
}

void handleStart(unsigned char *data, unsigned char *filename)
{
  unsigned char T = data[1];
  unsigned char size = data[2];

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
  unsigned char filename[MAX_FILENAME_SIZE];
  unsigned char fragment[MAX_BUF_SIZE];
  unsigned char delim[DELIM_SIZE];
  bool end = false;
  int size = 0;

  if (llread(fd, delim) <= 0)
  {
    fprintf(stderr, "llread error\n");
    exit(-1);
  }

  handleStart(delim, filename);

  file = fopen((char *)filename, "wb+");

  while (!end)
  {
    if (test == T_prop)
      usleep(delays[test_no]);

    size = llread(fd, fragment);

    if (size == 0 || size == -1)
    {
      debug_print("llread error\n");
      continue;
    }

    if (size != -2)
      end = handleData(fragment, file);
  }

  fclose(file);
}

int receiveFile(char *port)
{
  int fd = 0;
  struct termios oldtio;
  speed_t baudrate = B38400;

  if (test == C)
    baudrate = baud_rates[test_no];

  printf("rate: %d\n", baudrate);

  setUpPort(atoi(port), &fd, &oldtio, baudrate);

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

void log_test(FILE *stats, double time_spent, double R)
{
  char test_value[20];
  char test_type[50];

  switch (test)
  {
  case C:
    strcpy(test_type, "baudrate (bits/s)");
    strcpy(test_value, baud_rates_s[test_no]);
    break;
  case I:
    strcpy(test_type, "size (bytes)");
    sprintf(test_value, "%d", messageSizes[test_no]);
    break;
  case FER:
    strcpy(test_type, "prob (%)");
    sprintf(test_value, "%d", fail_prob[test_no]);
    break;
  case T_prop:
    strcpy(test_type, "delay (ms)");
    sprintf(test_value, "%d", delays[test_no] / 1000);
    break;
  default:
    break;
  }

  fprintf(stats,
          "test no.%d: %s - %s --- time taken (s) - %.2f  --- R (bit/s) - %f \n",
          test_no, test_type, test_value, time_spent, R);
}

int main(int argc, char **argv)
{
  int numTests = 1;
  FILE *stats;
  unsigned long long begin, end;
  double time_spent, R;
  const char *testNames[] = {"INV", "C", "I", "T_prop", "FER"};

  if ((argc != 2 && argc != 4) ||
      ((strcmp("0", argv[1]) != 0) && (strcmp("1", argv[1]) != 0)))
    return usage(argv);
  else if (argc == 4 && strcmp(argv[2], "-t") != 0)
    return usage(argv);
  else if (argc == 4 && ((test = processTestArgument(argv, 3)) == INV))
    return usage(argv);

  srand(time(NULL));
  startTime();
  stats = fopen("stats.txt", "w");

  if (test > INV)
  {
    numTests = NUMBER_OF_TESTS;
    fprintf(stats, "TEST TYPE %s\n", testNames[test]);

    if (test == T_prop)
      setUpAlarmHandler();
  }

  for (; test_no < numTests; test_no++)
  {
    begin = getTime();
    receiveFile(argv[1]);
    end = getTime();
    time_spent = (end - begin) / 100000.0;
    R = length * 8.0 / time_spent;
    log_test(stats, time_spent, R);
  }

  if (test > INV)
    printf("Please consult test results on stats.txt\n");

  fclose(stats);

  return 0;
}
