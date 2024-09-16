#ifndef __SLAVE_MANAGER_H__
#define __SLAVE_MANAGER_H__

#include "utils.h"
#include "defs.h"

typedef struct slave_t {
    pid_t pid;
    char *file_path;
    int slave2_master_fd[2];
    int master2_slave_fd[2];
    int is_available;
} slave_t;

int assign_file(slave_t *slave, char *const file_path);
int init_slaves(char *const argv[], uint32_t files_count, slave_t **slaves, uint16_t max_slaves);
pid_t create_slave(slave_t *slave, char *const files_path[], uint32_t files_count);
void free_slave(slave_t *slave);
void debug_slave(slave_t *slave);
void finish_slaves(slave_t **slaves, uint16_t slave_count);



#endif // __SLAVE_MANAGER_H__
