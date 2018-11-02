# RCOM1819

Projects for FEUP MIEIC's curricular unit RCOM.

## Project 1

Sending a file through serial port to another computer.

### Build

```bash
gcc -o sender sender.c utils.c protocol.c -Wall -Wextra -lm
gcc -o receiver receiver.c utils.c protocol.c -Wall -Wextra -lm
```

On the lab's PCs, it may be needed to append `-D_POSIX_C_SOURCE -std=c99`.
In order to see debug messages, append `-D DEBUG`.