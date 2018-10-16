#include <stdlib.h>

#include "utils.h"

bool findByteOnArray(unsigned char byte, unsigned char *array) {
  for (int i = 0; (array + i) != NULL; i++) {
    if (array[i] == byte)
      return true;
  }

  return false;
}