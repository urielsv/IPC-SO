#ifndef __SLAVE_MANAGER_H__
#define __SLAVE_MANAGER_H__

#include <stdlib.h>
#include <stdint.h>

typedef struct slave_t {
    pid_t pid;
    char *file_path;
    int pipefd[2];
} slave_t;

int assign_file(slave_t *slave, char *const file_path);
int init_slaves(char *const argv[], uint32_t files_count, slave_t **slaves, uint16_t max_slaves);
pid_t create_slave(slave_t *slave);
void free_slave(slave_t *slave);

#endif // __SLAVE_MANAGER_H__
