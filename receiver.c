/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
  int fdRead, fdWrite, res;
  struct termios oldtio,newtio;
  char buf[255];
  int i = 0;

  if ( (argc < 2) ||
  (strcmp("/dev/ttyS0", argv[1])!=0)) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0\n");
    exit(1);
  }


  /*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
  */


  fdRead = open(argv[1], O_RDWR | O_NOCTTY );
  if (fdRead <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(fdRead,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 1 ;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */



  /*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  leitura do(s) pr�ximo(s) caracter(es)
  */



  tcflush(fdRead, TCIOFLUSH);

  if ( tcsetattr(fdRead,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");


  while (STOP==FALSE) {       /* loop for input */
    res = read(fdRead,buf + i,1);   /* returns after 5 chars have been input */
    if (res>0)
    {
      if(buf[i]=='\0')
       STOP=TRUE;

       i++;
    }
  }

  printf("%s\n", buf);
  tcsetattr(fdRead,TCSANOW,&oldtio);
  close(fdRead);

  fdWrite = open(argv[1], O_RDWR | O_NOCTTY );
  if (fdWrite <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(fdWrite,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

  tcflush(fdWrite, TCIOFLUSH);

  if ( tcsetattr(fdWrite,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  res = write(fdWrite,buf,strlen(buf)+1);
  printf("%d bytes written\n", res);




  return 0;
}
