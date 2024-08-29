#ifndef __SLAVE_H__
#define __SLAVE_H__

#include <stdint.h>
#include <stdio.h>

#define ENC_SIZE 33

typedef struct slave_t {
    int pid;
    int ppid;
    FILE** files;
    uint64_t files_count;
    char md5[ENC_SIZE]; //char[] md5 = char[enc_size]
} slave_t;

void create_slave(slave_t* slave, FILE** files, uint64_t files_count);

#endif
