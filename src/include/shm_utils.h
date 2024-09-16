#ifndef _SHM_UTILS_H
#define _SHM_UTILS_H

#include "utils.h"

sem_t* sem_open_util(const char *name, int oflag);
void ftruncate_util(int shm_fd, size_t size, char *const shm_error);
int shm_open_util(char *const shm_path, int flags, char *const shm_error);
void close_semaphore(sem_t *semaphore, char *const sem_path);
void semaphore_up(sem_t *semaphore);
void semaphore_down(sem_t *semaphore);
void unlink_sem(char *const sem_path);
void shm_close(int shm_fd, char *const shm_path);

#endif // _SHM_UTILS_H
