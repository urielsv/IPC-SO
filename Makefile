CFLAGS=-Wall 
# -Wextra -Iinclude -pedantic
CFLAGS+=-g
LDFLAGS=-fsanitize=address -lpthread
CC=gcc

EXECS=md5 slave view
SRCS=src/*.c
OBJS=$(SRCS:.c=.o)
TXT=./*.txt

all: $(EXECS)

# $@ is the target of the rule
# $^ is the dependencies of the rule
md5: src/md5.o src/slave_manager.o src/utils.o src/shm_utils.o src/shm_manager.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

slave: src/slave.o src/utils.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

view: src/view.o src/utils.o src/shm_manager.o src/shm_utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files into object files
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXECS) $(TXT)


# Phony targets
.PHONY: all clean