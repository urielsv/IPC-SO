/*
** Utils
**
** Utility functions for the IPC and process management.
**
** Author: Fernando Li                      <feli@itba.edu.ar>
** Author: Felipe Venturino                 <fventurino@itba.edu.ar>
** Author: Uriel Sosa Vazquez               <usosavazquez@itba.edu.ar>
**
** Last modified: 03-09-2024 (dd-mm-yyyy)
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


void create_pipe(int fd[2], char *const pipe_error);
ssize_t write_pipe(int fd, char *const pipe_error, char *const buffer, size_t len);
ssize_t read_pipe(int fd, char *const pipe_error, char *const buffer, size_t len);
void close_pipe(int fd, char *const pipe_error);
void dup2_pipe(int fd1, int fd2, char *const pipe_error);
void check_fork(pid_t pid, char *const fork_error);
void check_program_path(char *path);
void close_end_and_dup2(int to_redirect, int new_fd, int to_close, char *const pipe_error);
void setvbuf_pipe(FILE *stream, char *const pipe_error);

#endif // UTILS_H
