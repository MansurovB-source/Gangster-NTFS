CC=gcc
CFLAGS=-c 
LIBS=-lblkid

all: main

main: device.o ntfs.o util.o main.o 
	$(CC) device.o ntfs.o util.o main.o -o main $(LIBS)

ntfs.o: ./core/src/ntfs.c
	$(CC) $(CFLAGS) ./core/src/ntfs.c

util.o: ./core/src/util.c
	$(CC) $(CFLAGS) ./core/src/util.c

device.o: ./core/src/device.c
	$(CC) $(CFLAGS) ./core/src/device.c

main.o: ./app/src/main.c
	$(CC) $(CFLAGS) ./app/src/main.c

clean:
	rm -rf *.o main

start:
	./main -l


	