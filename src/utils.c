
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "include/utils.h"


void inline setvbuf_pipe(FILE *stream, char *const pipe_error) {
    if (setvbuf(stream, NULL, _IONBF, 0) != 0) {
        fprintf(stderr, "Error: Could not setvbuf during %s\n", pipe_error);
        perror("setvbuf");
        exit(EXIT_FAILURE);
    }
}

void inline create_pipe(int fd[2], char *const pipe_error) {
    if (pipe(fd) == -1) {
        fprintf(stderr, "Error: Could not create pipe during %s\n", pipe_error);
        perror("pipe");
        exit(EXIT_FAILURE);
    }
}

ssize_t inline write_pipe(int fd, char *const pipe_error, char *const buffer, size_t len) {
    ssize_t written = write(fd, buffer, len);
    if (written != len) {
        fprintf(stderr, "Error: Could not write to pipe during %s\n", pipe_error);
        perror("write");
        exit(EXIT_FAILURE);
    }
    return written;
}

ssize_t inline read_pipe(int fd, char *const pipe_error, char *const buffer, size_t len) {
    ssize_t nbytes = read(fd, buffer, len);
    if (nbytes == -1) {
        fprintf(stderr, "Error: Could not read from pipe during %s\n", pipe_error);
        perror("read");
        exit(EXIT_FAILURE);
    }
    return nbytes;
}

void inline close_pipe(int fd, char *const pipe_error) {
    if (close(fd) == -1) {
        fprintf(stderr, "Error: Could not close pipe during %s\n", pipe_error);
        perror("close");
        exit(EXIT_FAILURE);
    }
}

void inline dup2_pipe(int fd, int new_fd, char *const pipe_error) {
    if (dup2(fd, new_fd) == -1) {
        fprintf(stderr, "Error: Could not duplicate pipe during %s\n", pipe_error);
        perror("dup2");
        exit(EXIT_FAILURE);
    }
}

void inline check_fork(pid_t pid, char *const fork_error) {
    if (pid < 0) {
        fprintf(stderr, "Error: Could not create process during %s\n", fork_error);
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

void inline check_program_path(char *path) {
    if (access(path, F_OK) == -1) {
        fprintf(stderr, "Error: Could not find %s\n", path);
        exit(EXIT_FAILURE);
    }
}

void inline close_end_and_dup2(int to_redirect, int new_fd, int to_close, char *const pipe_error) {
    close_pipe(to_close, pipe_error);
    dup2_pipe(to_redirect, new_fd, pipe_error);
    close_pipe(to_redirect, pipe_error);
}
