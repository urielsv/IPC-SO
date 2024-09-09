#ifndef __SHM_UTILS_H__
#define __SHM_UTILS_H__

#include <stddef.h>
#include <semaphore.h>
#include <string.h>

#define MAX_FILES       256
#define FILE_PATH_SIZE  64
#define MD5_HASH_SIZE   33
#define SHM_NAME        "/md5_shm"
#define SEM_NAME        "/md5_sem"

struct file_info_t {
    char file_path[FILE_PATH_SIZE];
    char md5[MD5_HASH_SIZE];
    int slave_id; // temp -> change to uint16_tS
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
    sem_t *semaphore;
} shared_memory_t;

int shm_open_util(char *const shm_error);
void ftruncate_util(int shm_fd, size_t size, char *const shm_error);
shared_memory_t *create_shared_memory();
int create_semaphore(shared_memory_t *shared_memory, int initial_value);
void write_shared_memory(shared_memory_t *shared_memory, char *const file_path, char *const md5, int slave_id);
void read_shared_memory(shared_memory_t *shared_memory, char *file_path, char *md5, int *slave_id);
void destroy_semaphore(shared_memory_t *shared_memory);
void destroy_shared_memory(shared_memory_t *shared_memory);
void shm_unlink_util(char *const shm_error);
void semaphore_down(sem_t *semaphore);
void semaphore_up(sem_t *semaphore);

#endif // __SHM_UTILS_H__
