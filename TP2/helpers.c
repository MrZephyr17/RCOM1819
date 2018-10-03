#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "helpers.h"

int alarmFlag = 1, alarmCounter = 1;

void alarm_handler()
{
  alarmFlag = 1;
  ++alarmCounter;
}

void setUpAlarmHandler()
{
  struct sigaction action;

  action.sa_handler = alarm_handler;
  sigemptyset(&action.sa_mask); // all signals are delivered
  action.sa_flags = 0;
  sigaction(SIGALRM, &action, NULL);
}

int llopen_receiver(int port)
{
  int fd = 0;
  struct termios oldtio;
  char *portPath = malloc(PORT_SIZE);

  portPath = (port == 0) ? "/dev/ttyS0" : "dev/ttyS1";

  setUpPort(portPath, &fd, &oldtio);

  if (receiveSupervisionByte(fd, SET_C))
    sendSupervisionByte(fd, UA_C);

  free(portPath);

  return 0;
}

int llopen_transmitter(int port)
{

  int received = 0;
  int fd = 0;
  struct termios oldtio;
  char *portPath = malloc(PORT_SIZE);

  portPath = (port == 0) ? "/dev/ttyS0"
                         : "dev/ttyS1";

  setUpPort(portPath, &fd, &oldtio);
  setUpAlarmHandler();

  while (alarmFlag && alarmCounter < MAX_RETRY_NUMBER)
  {
    sendSupervisionByte(fd, SET_C);
    alarm(TIME_OUT);
    alarmFlag = 0;

    while (!received && !alarmFlag)
    {
      if (receiveSupervisionByte(fd, UA_C))
        alarm(0);
    }
  }

  free(portPath);

  if (alarmCounter == MAX_RETRY_NUMBER)
    return -1;

  return 0;
}

int llopen(int port, int flag)
{
  switch (flag)
  {
  case RECEIVER:
    return llopen_receiver(port);
  case TRANSMITTER:
    return llopen_transmitter(port);
  default:
    fprintf(stderr, "Invalid flag argument!\n");
    return -1;
  }

  return 0;
}

int sendSupervisionByte(int fd, unsigned char C)
{
  unsigned char message[SUPERVISION_SIZE] = {FLAG, A, C, A ^ C, FLAG};

  return write(fd, message, SUPERVISION_SIZE) > 0;
}

int receiveSupervisionByte(int fd, unsigned char C)
{
  enum state_t state = START;
  int res = 0;
  unsigned char buf;

  while (state != END)
  {
    res = read(fd, &buf, 1);

    switch (state)
    {
    case START:
      if (res > 0 && buf == FLAG)
        state = FLAG_RCV;
      break;
    case FLAG_RCV:
      if (res > 0 && buf == A)
        state = A_RCV;
      else if (res > 0 && buf != FLAG)
        state = START;
      break;
    case A_RCV:
      if (res > 0 && buf == C)
        state = C_RCV;
      else if (res > 0 && buf == FLAG)
        state = FLAG_RCV;
      else if (res > 0 && buf != FLAG)
        state = START;
      break;
    case C_RCV:
      if (res > 0 && buf == (A ^ C))
        state = BCC_OK;
      else if (res > 0 && buf == FLAG)
        state = FLAG_RCV;
      else if (res > 0 && buf != FLAG)
        state = START;
      break;
    case BCC_OK:
      if (res > 0 && buf == FLAG)
        state = END;
      else if (res > 0)
        state = START;
      break;
    default:
      fprintf(stderr, "Invalid set state\n");
      return -1;
    }
  }

  return 0;
}

void setUpPort(char *port, int *fd, struct termios *oldtio)
{
  struct termios newtio;

  *fd = open(port, O_RDWR | O_NOCTTY);
  if (*fd < 0)
  {
    perror(port);
    exit(-1);
  }

  if (tcgetattr(*fd, oldtio) == -1)
  { /* save current port settings */
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

  tcflush(*fd, TCIOFLUSH);

  if (tcsetattr(*fd, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");
}

void readSentence(volatile int *STOP, int fd, char *buf)
{
  int i = 0;
  int res;

  while (*STOP == FALSE)
  {                             /* loop for input */
    res = read(fd, buf + i, 1); /* returns after 5 chars have been input */
    if (res > 0)
    {
      if (buf[i] == '\0')
        *STOP = TRUE;

      i++;
    }
  }

  printf("%s\n", buf);
}

void usage(int argc, char *argv[])
{
  if ((argc < 2) || (strcmp("/dev/ttyS0", argv[1]) != 0))
  {
    printf("Usage:\t %s SerialPort\n\tex: %s /dev/ttyS0\n", argv[0], argv[0]);
    exit(1);
  }
}

void writeSentence(int fd, char *buf)
{
  int res;

  res = write(fd, buf, strlen(buf) + 1);
  printf("%d bytes written\n", res);
}

void closeFd(int fd, struct termios *oldtio)
{
  if (tcsetattr(fd, TCSANOW, oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }
  close(fd);
}
