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

## Screenshots

### Build
<img width="747" height="68" alt="image" src="https://github.com/user-attachments/assets/6df02ded-9f00-458a-9777-e3fd1809a9e3" />


### Test Results (no hardware needed)
<img width="747" height="402" alt="image" src="https://github.com/user-attachments/assets/13a2cda3-255b-45d7-8c0a-76baf8c49672" />


### Full TX/RX Demo (virtual serial ports)
<img width="1131" height="129" alt="image" src="https://github.com/user-attachments/assets/f1503fcd-3300-45e3-bdec-d41d3e9e3c8c" />
<img width="1100" height="236" alt="image" src="https://github.com/user-attachments/assets/a51cb28d-8ce1-48b1-9ded-5ea366c00bb5" />



### Error Handling
<img width="859" height="155" alt="image" src="https://github.com/user-attachments/assets/bae57ead-bad4-4a65-9099-a8ea48cf295a" />



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
