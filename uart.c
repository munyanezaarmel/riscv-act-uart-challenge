
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>

#define READ_TIMEOUT_SEC   2   
#define READ_BUF_SIZE      256  

static speed_t baud_to_speed(int baud) {
    switch (baud) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:
            fprintf(stderr, "Unsupported baud rate: %d\n", baud);
            return B0; 
    }
}

static int open_serial_port(const char *device, int baud) {

    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        fprintf(stderr, "Error opening %s: %s\n", device, strerror(errno));
        return -1;
    }

    fcntl(fd, F_SETFL, 0);


    struct termios options;
    if (tcgetattr(fd, &options) != 0) {
        fprintf(stderr, "tcgetattr failed: %s\n", strerror(errno));
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
    options.c_cflag &= ~CRTSCTS;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    options.c_oflag &= ~OPOST;

    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        fprintf(stderr, "tcsetattr failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    tcflush(fd, TCIOFLUSH);

    printf("Opened %s at %d baud (8N1, no flow control)\n", device, baud);
    return fd;
}

static int uart_send(int fd, const char *msg) {
    ssize_t len    = (ssize_t)strlen(msg);
    ssize_t written = write(fd, msg, len);
    if (written == -1) {
        fprintf(stderr, "write() failed: %s\n", strerror(errno));
        return -1;
    }
    if (written != len) {
        fprintf(stderr, "Partial write: %zd of %zd bytes sent\n", written, len);
        return -1;
    }
    printf("Sent (%zd bytes): %s\n", written, msg);
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

    printf("Waiting for data (timeout: %ds)...\n", READ_TIMEOUT_SEC);

    int ret = select(fd + 1, &read_fds, NULL, NULL, &timeout);
    if (ret == -1) {
        fprintf(stderr, "select() error: %s\n", strerror(errno));
        return -1;
    } else if (ret == 0) {
        printf("Timeout — no data received within %d seconds.\n", READ_TIMEOUT_SEC);
        return 0;
    }

    if (FD_ISSET(fd, &read_fds)) {
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        if (n == -1) {
            fprintf(stderr, "read() error: %s\n", strerror(errno));
            return -1;
        }
        buf[n] = '\0'; 
        printf("Received (%zd bytes): %s\n", n, buf);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <device> <baud_rate>\n", argv[0]);
        fprintf(stderr, "  e.g. %s /dev/ttyUSB0 115200\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *device = argv[1];
    int baud = atoi(argv[2]);
    if (baud <= 0) {
        fprintf(stderr, "Invalid baud rate: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    int fd = open_serial_port(device, baud);
    if (fd == -1) return EXIT_FAILURE;


    const char *msg = "Hello from UART demo — RISC-V ACT project\r\n";
    if (uart_send(fd, msg) != 0) {
        close(fd);
        return EXIT_FAILURE;
    }

    if (uart_receive(fd) != 0) {
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);
    printf("Done. Port closed cleanly.\n");
    return EXIT_SUCCESS;
}