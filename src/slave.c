

#include "include/slave.h"
#include "include/utils.h"
#include "include/defs.h"


int main(int argc, char *const argv[]) {

    // Disable buffering because we are writing to a pipe
    setvbuf_pipe(stdin, "stdin");
    // Disable the buffering for the stdin because we are reading from a pipe
    setvbuf_pipe(stdout, "stdout");


    // We are reading from a pipe the file paths
    char file_path[BUFF_SIZE];
    char md5[ENC_SIZE+1] = {0};
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

    // Add null terminator
    md5[ENC_SIZE] = '\0';

    if (pclose(fp) == -1) {
        perror("pclose");
        exit(EXIT_FAILURE);
    }
}
