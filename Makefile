CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = uart_demo
SRC = uart.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)