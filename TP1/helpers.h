#pragma once

#include <termios.h>

#define FALSE 0
#define TRUE 1

void setUpPort(char* argv[], int *fd, struct termios *oldtio);

void readSentence(volatile int *STOP, int fdRead, char* buf);

void usage(int argc, char* argv[]);

void writeSentence(int fdWrite, char* buf);

void closeFd(int fd, struct termios *oldtio);
