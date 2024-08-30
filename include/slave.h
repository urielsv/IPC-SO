#ifndef __SLAVE_H__
#define __SLAVE_H__

#include <stdint.h>
#include <stdio.h>

#define ENC_SIZE 33

typedef struct slave_t {
    int pid;
    int ppid;
    char *file_path;
    char md5[ENC_SIZE];
} slave_t;

void create_slave(slave_t* slave, char *const file_path);
char *get_md5(char *const file_path);
#endif
