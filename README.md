# UART Demo — RISC-V ACT Mentorship Coding Challenge

A C program that configures and communicates over a UART serial port
on Linux using the `termios` API, with non-blocking I/O via `select()`.

## Features
- Configures baud rate, data bits, parity, and stop bits
- Transmits a test message over UART
- Receives data with a configurable timeout using `select()`
- Graceful error handling for invalid paths, permissions, and I/O failures
- Tested with real hardware and virtual ports (socat)

## Build

```bash
make
# or
gcc -Wall -Wextra -g -o uart_demo uart.c
```

## Run

```bash
./uart_demo <device> <baud_rate>

# Real hardware:
./uart_demo /dev/ttyUSB0 115200

# Virtual ports (no hardware needed):
socat -d -d pty,raw,echo=0 pty,raw,echo=0  # note the two /dev/pts/N paths
./uart_demo /dev/pts/4 115200
```

## Testing without hardware

Install socat: `sudo apt install socat`
This creates a linked virtual serial port pair for local testing.

## References
- termios(3): https://man7.org/linux/man-pages/man3/termios.3.html
- select(2): https://man7.org/linux/man-pages/man2/select.2.html