#ifndef __VIEW_H__
#define __VIEW_H__

#include "utils.h"
#include "defs.h"
#include "shm_manager.h"

char* read_line_from_stdin();
int validate_arguments(char *shm_path, char *buff_sem_path, char *mutex_sem_path);
void load_parameters(int argc, char *argv[], char **shm_path, char **buff_sem_path, char **mutex_sem_path);


#endif // __VIEW_H__