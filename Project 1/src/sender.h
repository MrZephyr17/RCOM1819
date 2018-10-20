#pragma once

#include <unistd.h>

int usage(char **argv);

unsigned char *getTLVLength(int fileLength);

unsigned char *getTLVName(char *fileName, int stringLength);

unsigned char *getDelimPackage(unsigned char C, int fileLength, char *fileName,
                               int stringLength, int *size);

unsigned char *readFile(const char *fileName, off_t *size);

unsigned char *getFragment(int seqNum, unsigned char *data, int K);

void writeFile(int fd, char *filename);