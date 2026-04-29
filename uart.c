/*
 * uart.c — UART communication demo using termios API (Linux)
 * Configured for testing with virtual serial ports (socat/pipes)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>

#define READ_TIMEOUT_SEC   3
#define READ_BUF_SIZE      256

static speed_t baud_to_speed(int baud) {
    switch (baud) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:
            fprintf(stderr, "[ERROR] Unsupported baud rate: %d\n", baud);
            return B0;
    }
}

static int open_serial_port(const char *device, int baud) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        fprintf(stderr, "[ERROR] Cannot open %s: %s\n", device, strerror(errno));
        return -1;
    }
    fcntl(fd, F_SETFL, 0);

    struct termios options;
    if (tcgetattr(fd, &options) != 0) {
        fprintf(stderr, "[ERROR] tcgetattr failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    speed_t speed = baud_to_speed(baud);
    if (speed == B0) { close(fd); return -1; }
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    options.c_cflag |=  (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |=  CS8;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    options.c_oflag &= ~OPOST;
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        fprintf(stderr, "[ERROR] tcsetattr failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    tcflush(fd, TCIOFLUSH);
    printf("[OK]   Port opened: %s at %d baud (8N1)\n", device, baud);
    return fd;
}

static int uart_send(int fd, const char *msg) {
    ssize_t len = (ssize_t)strlen(msg);
    ssize_t written = write(fd, msg, len);
    if (written == -1) {
        fprintf(stderr, "[ERROR] write() failed: %s\n", strerror(errno));
        return -1;
    }
    if (written != len) {
        fprintf(stderr, "[WARN]  Partial write: %zd of %zd bytes\n", written, len);
        return -1;
    }
    printf("[TX]   Sent (%zd bytes): %s", written, msg);
    return 0;
}

static int uart_receive(int fd) {
    fd_set read_fds;
    struct timeval timeout;
    char buf[READ_BUF_SIZE];

    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    timeout.tv_sec  = READ_TIMEOUT_SEC;
    timeout.tv_usec = 0;

    printf("[WAIT] Listening for incoming data (timeout: %ds)...\n", READ_TIMEOUT_SEC);
    fflush(stdout);

    int ret = select(fd + 1, &read_fds, NULL, NULL, &timeout);
    if (ret == -1) {
        fprintf(stderr, "[ERROR] select() failed: %s\n", strerror(errno));
        return -1;
    } else if (ret == 0) {
        printf("[TIMEOUT] No data received within %d seconds.\n", READ_TIMEOUT_SEC);
        return 0;
    }

    if (FD_ISSET(fd, &read_fds)) {
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        if (n == -1) {
            fprintf(stderr, "[ERROR] read() failed: %s\n", strerror(errno));
            return -1;
        }
        buf[n] = '\0';
        printf("[RX]   Received (%zd bytes): %s", n, buf);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    printf("=== UART Demo — RISC-V ACT Mentorship Challenge ===\n");
    if (argc != 3) {
        fprintf(stderr, "[USAGE] %s <device> <baud_rate>\n", argv[0]);
        fprintf(stderr,"         e.g. %s /dev/ttyUSB0 115200\n", argv[0]);
        fprintf(stderr,"         e.g. %s /dev/pts/4   115200  (virtual port)\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *device = argv[1];
    int baud = atoi(argv[2]);
    if (baud <= 0) {
        fprintf(stderr, "[ERROR] Invalid baud rate: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    int fd = open_serial_port(device, baud);
    if (fd == -1) return EXIT_FAILURE;

    const char *msg = "Hello from UART demo — RISC-V ACT Mentorship\r\n";
    if (uart_send(fd, msg) != 0) { close(fd); return EXIT_FAILURE; }

    uart_receive(fd);

    close(fd);
    printf("[DONE] Port closed cleanly.\n");
    return EXIT_SUCCESS;
}
