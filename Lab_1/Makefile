CC=gcc
CFLAGS=-c
LIBS=-lblkid

all: main

main: device.o main.o
	$(CC) device.o main.o -o main $(LIBS)

device.o: device.h device.c
	$(CC) $(CFLAGS) device.c

main.o: main.c
	$(CC) $(CFLAGS) main.c

clean:
	rm -rf *.o main

start:
	./main