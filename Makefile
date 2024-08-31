
CFLAGS=-Wall -g
LDFLAGS=-lm
CC=gcc

all: md5 slave view

md5: md5.o
	$(CC) $(CFLAGS) -o md5 md5.o $(LDFLAGS)

slave: slave.o
	$(CC) $(CFLAGS) -o slave slave.o $(LDFLAGS)

view: view.o
	$(CC) $(CFLAGS) -o view view.o $(LDFLAGS)

clean:
	rm -f md5 slave view *.o
