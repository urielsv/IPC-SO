#ifndef __MD_5_H__
#define __MD_5_H__

typedef struct slave_t {
    pid_t pid;
    char *file_path;
    int pipefd[2];
} slave_t;

int assign_file(slave_t *slave, char *const file_path);
int init_slaves(char *files_path[], int files_count, slave_t **slaves);
pid_t create_slave(slave_t *slave);
static void check_program_path(char *path);

#endif
