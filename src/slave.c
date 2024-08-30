#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../include/slave.h"

#define REQUIRED_ARGS 2

int main(int argc, char *const argv[]) {
    printf("Hello im slave pid: %d, ppid: %d\n", getpid(), getppid());

    if (argc != REQUIRED_ARGS) {
        fprintf(stderr, "Usage: %s <file1>\n", argv[0]);
        return 1;
    }

    slave_t *slave = (slave_t *) malloc(sizeof(slave_t));
    create_slave(slave, argv[1]);
    return 0;
}

void create_slave(slave_t *slave, char *const file_path) {
    slave->file_path = file_path;
    slave->pid = getpid();
    slave->ppid = getppid();
}

char *get_md5(char *const file_path) {

    return NULL;
}
