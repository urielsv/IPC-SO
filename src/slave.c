#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "include/slave.h"
#include "include/utils.h"

#define ENC_SIZE        32
#define BUFF_SIZE       256
#define MD5SUM_PATH     "/usr/bin/md5sum"

int main(int argc, char *const argv[]) {

    // char md5[ENC_SIZE];
    // Read the file path from the pipe

    char file_path[BUFF_SIZE];
    read(STDIN_FILENO, file_path, BUFF_SIZE);
    fprintf(stderr, "Slave read: %s\n", file_path);
    get_md5(file_path);
    return 0;
}

void get_md5(char *const file_path) {
    char cmd[BUFF_SIZE];
    snprintf(cmd, sizeof(cmd), "%s %s", MD5SUM_PATH, file_path);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    // md5 hash has a length of 32 + null terminated
    char result[ENC_SIZE+1];
    if (fgets(result, sizeof(result), fp) == NULL) {
        pclose(fp);
        perror("fgets");
        exit(EXIT_FAILURE);
    }

    //el maldito null terminated chaval
    result[ENC_SIZE] = '\0';
    write(STDOUT_FILENO, result, ENC_SIZE);
    if (pclose(fp) == -1) {
        perror("pclose");
        exit(EXIT_FAILURE);
    }
}
