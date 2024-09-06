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

    // Disable buffering because we are writing to a pipe
    if (setvbuf(stdout, NULL, _IONBF, 0) != 0) {
        perror("setvbuf");
        exit(EXIT_FAILURE);
    }

    // Here we disable the buffering for the stdin because we are reading from a pipe
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0) {
        perror("setvbuf");
        exit(EXIT_FAILURE);
    }

    char file_path[BUFF_SIZE];
    char md5[ENC_SIZE+1] = {0};
    fprintf(stderr, "Creating slave\n");
    while (fgets(file_path, BUFF_SIZE, stdin) != NULL) {
        // Remove the newline character, TODO fix warning
        file_path[strcspn(file_path, "\n")] = '\0';
        get_md5(file_path, md5);
        write_pipe(STDOUT_FILENO, "slave write", md5, ENC_SIZE+1);
    }

    return 0;
}

void get_md5(char *const file_path, char *md5) {
    char cmd[BUFF_SIZE];
    snprintf(cmd, sizeof(cmd), "%s %s", MD5SUM_PATH, file_path);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    // md5 hash has a length of 32 + null terminated
    if (fgets(md5, ENC_SIZE+1, fp) == NULL) {
        pclose(fp);
        perror("fgets");
        exit(EXIT_FAILURE);
    }

    md5[ENC_SIZE] = '\0';
    if (pclose(fp) == -1) {
        perror("pclose");
        exit(EXIT_FAILURE);
    }
}
