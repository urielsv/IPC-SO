#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "slave.h"

int main(int argc, char* argv[]) {
    printf("Hello im slave pid: %d, ppid: %d\n", getpid(), getppid());
    FILE** files = (FILE**) malloc(sizeof(FILE*) * argc);
    for (int i = 1; i < argc; i++) {
        files[i - 1] = fopen(argv[i], "r");
    }
    slave_t* slave = (slave_t*) malloc(sizeof(slave_t));
    create_slave(slave, files, argc - 1);
    return 0;
}

void create_slave(slave_t* slave, FILE** files, uint64_t files_count) {
    slave->files = files;
    slave->files_count = files_count;
    slave->pid = getpid();
    slave->ppid = getppid();
}

int generate_MD5(slave_t* slave) {
    // Generate MD5

    return 0;
}
