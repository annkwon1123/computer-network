CC=gcc
CFLAGS=-Wall -g

myserver: myserver.o
	$(CC) $(CFLAGS) -o myserver myserver.o

myserver.o: myserver.c
	$(CC) $(CFLAGS) -c myserver.c

clean:
	rm -f myserver myserver.o
