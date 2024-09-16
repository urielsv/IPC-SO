#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stddef.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

void create_pipe(int fd[2], char *const pipe_error);
ssize_t write_pipe(int fd, char *const pipe_error, char *const buffer, size_t len);
ssize_t read_pipe(int fd, char *const pipe_error, char *const buffer, size_t len);
void close_pipe(int fd, char *const pipe_error);
void dup2_pipe(int fd1, int fd2, char *const pipe_error);
void check_fork(pid_t pid, char *const fork_error);
void check_program_path(char *path);
void close_end_and_dup2(int to_redirect, int new_fd, int to_close, char *const pipe_error);
void setvbuf_pipe(FILE *stream, char *const pipe_error);

#endif // __UTILS_H__
