CC = gcc
CFLAGS = -O2 -Wall -Wextra -pthread
TARGET = pthreads
SRC = pthreads.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
