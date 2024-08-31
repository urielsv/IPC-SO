
CFLAGS=-Wall -g
LDFLAGS=-lm
CC=gcc

all: md5 slave view

md5: md5
	$(CC) $(CFLAGS) src/md5.c -o md5 $(LDFLAGS)

slave: slave
	$(CC) $(CFLAGS) src/slave.c -o  slave $(LDFLAGS)

view: view
	$(CC) $(CFLAGS) src/view.c -o view $(LDFLAGS)

clean:
	rm -f md5 slave view *.o
