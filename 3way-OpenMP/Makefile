CC = gcc
CFLAGS = -O2 -fopenmp -Wall -Wextra
TARGET = OpenMP
SRC = OpenMP.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
