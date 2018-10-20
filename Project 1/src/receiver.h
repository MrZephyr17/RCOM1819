#pragma once

#include <stdbool.h>

int usage(char **argv);

void handleStart(unsigned char *data, unsigned char *filename);

bool handleData(unsigned char *data, FILE *file);

void readFile(int fd);

