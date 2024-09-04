#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "include/slave.h"
#include "include/utils.h"

#define ENC_SIZE        33
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
    char cmd[BUFF_SIZE + 10];
    snprintf(cmd, sizeof(cmd), "%s %s", MD5SUM_PATH, file_path);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char result[BUFF_SIZE];
    if (fgets(result, sizeof(result), fp) == NULL) {
        pclose(fp);
        fprintf(stderr, "fgets failed\n");
        perror("fgets");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "MD5: %s\n", result);
    fprintf(STDOUT_FILENO, "MD5: %s\n", result);
    write(STDOUT_FILENO, result, BUFF_SIZE);
    if (pclose(fp) == -1) {
        perror("pclose");
        exit(EXIT_FAILURE);
    }
}
