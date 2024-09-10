CFLAGS=-Wall -g
LDFLAGS=
CC=gcc

EXECS=md5 slave view
SRCS=src/md5.c src/slave.c src/view.c src/slave_manager.c src/utils.c src/shm_utils.c
OBJS=$(SRCS:.c=.o)

all: $(EXECS)

# $@ is the target of the rule
# $^ is the dependencies of the rule
md5: src/md5.o src/slave_manager.o src/utils.o src/shm_utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

slave: src/slave.o src/utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

view: src/view.o src/utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files into object files
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXECS)


# Phony targets
.PHONY: all clean