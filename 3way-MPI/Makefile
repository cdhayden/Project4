CC = mpicc
TARGET = MPICode
SRC = MPICode.c
CFLAGS = -O2 -fopenmp -Wall -Wextra

all: $(TARGET) 

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	/bin/rm -f $(TARGET)
