
CFLAGS=-Wall -g
LDFLAGS=-lm
CC=gcc

EXECS=md5 slave view
SRCS=src/md5.c src/slave.c src/view.c
OBJS=$(SRCS:.c=.o)

all: $(EXECS)

# $@ is the target of the rule
# $^ is the dependencies of the rule
md5: src/md5.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

slave: src/slave.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

view: src/view.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# $< is the first dependency of the rule
%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EXECS) src/*.o

PHONY: all clean
