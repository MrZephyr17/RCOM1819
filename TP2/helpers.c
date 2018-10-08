#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "helpers.h"

int alarmFlag = 0, alarmCounter = 0;

void print_debug(char *msg)
{
  (void)msg;

#ifdef DEBUG
  printf("%s", msg);
#endif
}

void alarm_handler()
{
  alarmFlag = 1;
  ++alarmCounter;
  print_debug("Received alarm signal\n");
}

void setUpAlarmHandler()
{
  struct sigaction action;

  action.sa_handler = alarm_handler;
  sigemptyset(&action.sa_mask); // all signals are delivered
  action.sa_flags = 0;
  sigaction(SIGALRM, &action, NULL);

  print_debug("Installed alarm handler\n");
}

int stateMachineSByte(enum state_t *state, unsigned char buf, int res, unsigned char C)
{
  switch (*state)
  {
  case START:
    if (res > 0 && buf == FLAG)
      *state = FLAG_RCV;
    break;
  case FLAG_RCV:
    if (res > 0 && buf == A)
      *state = A_RCV;
    else if (res > 0 && buf != FLAG)
      *state = START;
    break;
  case A_RCV:
    if (res > 0 && buf == C)
      *state = C_RCV;
    else if (res > 0 && buf == FLAG)
      *state = FLAG_RCV;
    else if (res > 0 && buf != FLAG)
      *state = START;
    break;
  case C_RCV:
    if (res > 0 && buf == (A ^ C))
      *state = BCC_OK;
    else if (res > 0 && buf == FLAG)
      *state = FLAG_RCV;
    else if (res > 0 && buf != FLAG)
      *state = START;
    break;
  case BCC_OK:
    if (res > 0 && buf == FLAG)
      *state = END;
    else if (res > 0)
      *state = START;
    break;
  case END:
    break;
  default:
    fprintf(stderr, "Invalid set state\n");
    return -1;
  }

  return 0;
}

int llopen_receiver(int fd)
{
  if (receiveSupervisionByte(fd, SET_C))
  {
    print_debug("Received SET_C\n");

    sendSupervisionByte(fd, UA_C);
  }

  return 0;
}

int llopen_transmitter(int fd)
{
  int received = 0;

  setUpAlarmHandler();

  unsigned char buf;

  do
  {
    sendSupervisionByte(fd, SET_C);
    alarm(TIME_OUT);
    alarmFlag = 0;
    int res = 0;
    enum state_t state = START;

    while (!received && !alarmFlag)
    {
      res = read(fd, &buf, 1);

      stateMachineSByte(&state, buf, res, UA_C);

      if (state == BCC_OK)
      {
        alarm(0);
        received = TRUE;
        print_debug("Received UA_C\n");
      }
    }
  } while (alarmFlag && alarmCounter < MAX_RETRY_NUMBER);

  if (alarmFlag && alarmCounter == MAX_RETRY_NUMBER)
    return -1;

  alarmFlag = FALSE;
  alarmCounter = 0;

  return 0;
}

int llopen(int flag, int fd)
{
  switch (flag)
  {
  case RECEIVER:
    return llopen_receiver(fd);
  case TRANSMITTER:
    return llopen_transmitter(fd);
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

    stateMachineSByte(&state, buf, res, C);
  }

  return 1;
}

void setUpPort(int port, int *fd, struct termios *oldtio)
{
  struct termios newtio;
  char *portPath = malloc(PORT_SIZE);

  if (port == 0)
    strcpy(portPath, "/dev/ttyS0");
  else
    strcpy(portPath, "/dev/ttyS1");

  *fd = open(portPath, O_RDWR | O_NOCTTY);
  if (*fd < 0)
  {
    perror(portPath);
    exit(-1);
  }

  free(portPath);

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

  sleep(1);

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
  if ((argc < 2) || ((strcmp("0", argv[1]) != 0) && (strcmp("1", argv[1]) != 0)))
  {
    printf("Usage:\t %s SerialPort\n\tex: %s 0\n", argv[0], argv[0]);
    exit(1);
  }
}

void writeSentence(int fd, char *buf)
{
  int res;

  res = write(fd, buf, strlen(buf) + 1);
  tcflush(fd, TCIOFLUSH);
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
