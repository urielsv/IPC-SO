#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "include/slave.h"


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
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        // Close the original write end of the pipe
        close(pipe_fd[1]);
        char *const args[] = { "md5sum", file_path, NULL };
        execve("/sbin/md5sum", args, NULL);
        perror("execve");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        // Close the write end of the pipe
        close(pipe_fd[1]);

        char buffer[BUFF_SIZE];
        ssize_t nbytes;
        // TEMP: Testing the read end of the pipe
        while ((nbytes = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[nbytes] = '\0';
            printf("%s", buffer);
        }
        close(pipe_fd[0]);
        // Wait for the child process to finish
        wait(NULL);
    }
    // TEMP print the slave with pid and the file path analyzed
    printf("slave (%d): reading %s\n", pid, file_path);
    return NULL;
}
