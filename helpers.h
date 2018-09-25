#pragma once

#include <termios.h>


void setUpReceiver(char* argv[], int fdRead, struct termios *oldtio);

void setUpSender(char* argv[], int fdWrite, struct termios *oldtio);

void readSentence(int fdRead, char* buf);

void usage(int argc, char* argv[]);

void writeSentence(int fdWrite, char* buf);

void closeFd(int fd, struct termios *oldtio);
