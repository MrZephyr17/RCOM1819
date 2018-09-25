#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "helpers.h"

#define BAUDRATE B38400
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

void setUpReceiver(char *argv[], int fdRead, struct termios *oldtio) {
  struct termios newtio;

  fdRead = open(argv[1], O_RDWR | O_NOCTTY);
  if (fdRead < 0) {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fdRead, oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

  /*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  leitura do(s) pr�ximo(s) caracter(es)
  */

  tcflush(fdRead, TCIOFLUSH);

  if (tcsetattr(fdRead, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");
}

void setUpSender(char *argv[], int fdWrite, struct termios *oldtio) {

  struct termios newtio;

  fdWrite = open(argv[1], O_RDWR | O_NOCTTY);
  if (fdWrite < 0) {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fdWrite, oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

  tcflush(fdWrite, TCIOFLUSH);

  if (tcsetattr(fdWrite, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");
}

void readSentence(int fdRead, char *buf) {
  int i = 0;
  int res;

  while (STOP == FALSE) {           /* loop for input */
    res = read(fdRead, buf + i, 1); /* returns after 5 chars have been input */
    if (res > 0) {
      if (buf[i] == '\0')
        STOP = TRUE;

      i++;
    }
  }

  printf("%s\n", buf);
}

void usage(int argc, char *argv[]) {
  if ((argc < 2) || (strcmp("/dev/ttyS0", argv[1]) != 0)) {
    printf("Usage:\t %s SerialPort\n\tex: %s /dev/ttyS0\n", argv[0], argv[0]);
    exit(1);
  }
}

void writeSentence(int fdWrite, char *buf) {
  int res;

  res = write(fdWrite, buf, strlen(buf) + 1);
  printf("%d bytes written\n", res);
}

void closeFd(int fd, struct termios *oldtio) {
  if (tcsetattr(fd, TCSANOW, oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  close(fd);
}
