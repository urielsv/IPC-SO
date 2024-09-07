#ifndef __SHM_UTILS_H__
#define __SHM_UTILS_H__

#include <stddef.h>
#include <semaphore.h>
#include <string.h>

#define MAX_FILES       256
#define FILE_PATH_SIZE  64
#define MD5_HASH_SIZE   33

typedef struct semaphore_t {
    char *name;
    sem_t *semaphore;
    int value;
} semaphore_t;

struct file_info_t {
    char file_path[FILE_PATH_SIZE];
    char md5[MD5_HASH_SIZE];
    char slave_id[30]; // temp
};

struct buffer_t {
    struct file_info_t file_info[MAX_FILES];
    size_t written;
    size_t read;
};

typedef struct shared_memory_t {
    int fd;
    struct buffer_t *buffer;
    size_t files_processed;
    semaphore_t *semaphore;
} shared_memory_t;

int shm_open_util(char *const shm_error);
void ftruncate_util(int shm_fd, size_t size, char *const shm_error);
shared_memory_t *create_shared_memory(char *const semaphore_name, size_t semaphore_value);
semaphore_t *create_semaphore(char *const sem_name, int initial_value, char *const sem_error);
void write_shared_memory(shared_memory_t *shared_memory, struct file_info_t file_info);

#endif // __SHM_UTILS_H__