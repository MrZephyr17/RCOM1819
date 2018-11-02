#include "receiver.h"
#include "protocol.h"
#include "utils.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int alarm_flag = 1, count = 1;
int length = 0, test_no = 0;
test_t test = INV;
int alarm_times[NUMBER_OF_TESTS] = {1, 2, 3, 4, 5};
speed_t baud_rates[NUMBER_OF_TESTS] = {B2400, B4800, B9600, B19200, B38400};

void delay_alarm_handler() // atende alarme
{
  printf("alarme # %d\n", count);
  alarm_flag = 1;
  count++;
}

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
    {
      while (count < NUMBER_OF_ALARMS)
      {
        if (alarm_flag)
        {
          alarm(alarm_times[test_no]);
          alarm_flag = 0;
        }
      }
    }

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

void setUpDelayAlarmHandler()
{
  struct sigaction action;

  action.sa_handler = alarm_handler;
  sigemptyset(&action.sa_mask); // all signals are delivered
  action.sa_flags = 0;
  sigaction(SIGALRM, &action, NULL);

  debug_print("Installed alarm handler\n");
}

int main(int argc, char **argv)
{
  int numTests = 1;
  FILE *stats;
  unsigned long long begin, end;
  double time_spent, R;

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
    fprintf(stats, "TEST TYPE %d\n", test);

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
    fprintf(stats, "test no.%d: time taken - %.2f (s) --- R - %f\n", test_no,
            time_spent, R);
  }

  if (test > INV)
    printf("Please consult test results on stats.txt\n");

  fclose(stats);

  return 0;
}
