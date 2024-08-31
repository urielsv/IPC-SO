#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../include/slave.h"


#define ENC_SIZE 33
#define BUFF_SIZE 256
int main(int argc, char *const argv[]) {
    // printf("Hello im slave pid: %d, ppid: %d\n", getpid(), getppid());

    char md5[ENC_SIZE];
    // Read the file path from the pipe

    char file_path[BUFF_SIZE];
    read(STDIN_FILENO, file_path, BUFF_SIZE);
    get_md5(file_path);
    return 0;
}

char *get_md5(char *const file_path) {
    printf("Slave read: %s\n", file_path);

    return NULL;
}
