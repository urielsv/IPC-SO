#ifndef __MD_5_H__
#define __MD_5_H__


int assign_slaves(char *argv[], int filesPerSlave);
int init_slaves(char *argv[], int filesPerSlave);
void create_slave(char *files[]);
static void check_program_path(char *path);

#endif
