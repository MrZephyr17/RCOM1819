# RCOM1819

Projects for FEUP MIEIC's curricular unit RCOM.

## Project 1

Sending a file through serial port to another computer.

### Build

```bash
gcc -o sender sender.c helpers.c protocol.c -Wall -Wextra
gcc -o receiver receiver.c helpers.c protocol.c -Wall -Wextra
```

In order to see debug messages, append `-D DEBUG`.