# RCOM1819

Projects for FEUP MIEIC's curricular unit RCOM.
NEVER USE THE NAME OF PROFESSOR VIDAL IN VAIN

## Project 1

Sending a file through serial port to another computer.

### Build

```bash
gcc -o sender sender.c utils.c protocol.c -Wall -Wextra -lm
gcc -o receiver receiver.c utils.c protocol.c -Wall -Wextra -lm
```

On the lab's PCs, it may be needed to append `-D _BSD_SOURCE -std=c99`.
In order to see debug messages, append `-D DEBUG`.

## Project 2

Implementing a download application with File Transfer Protocol, and doing experiments
on RCOM lab.

### Build

```bash
gcc -o download download.c -Wall -Wextra
```
