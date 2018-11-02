#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils.h"

unsigned long long start_ms;
time_t start_s;

bool findByteOnArray(unsigned char byte, unsigned char *array)
{
  for (int i = 0; (array + i) != NULL; i++)
  {
    if (array[i] == byte)
      return true;
  }

  return false;
}

unsigned long long getTime() {
  unsigned long long now_ms;
  time_t now_s;

  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  now_ms = round(spec.tv_nsec / 1.0e4);
  now_s = spec.tv_sec;

  if (now_ms > 99999) {
    now_s++;
    now_ms = 0;
  }

  return ((now_s - start_s) * 100000) + (now_ms - start_ms);
}

void startTime() {
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  start_ms = round(spec.tv_nsec / 1.0e4);
  start_s = spec.tv_sec;

  if (start_ms > 99999) {
    start_s++;
    start_ms = 0;
  }
}

int processTestArgument(char **argv, int argNum) {
  if (strcmp(argv[argNum], "C") == 0)
    return C;

  if (strcmp(argv[argNum], "I") == 0)
    return I;

  if (strcmp(argv[argNum], "T_prop") == 0)
    return T_prop;

  if (strcmp(argv[argNum], "FER") == 0)
    return FER;

  return INV;
}

