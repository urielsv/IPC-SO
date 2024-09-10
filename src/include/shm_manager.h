#ifndef __SHM_MANAGER_H__
#define __SHM_MANAGER_H__

#include <stddef.h>
#include <semaphore.h>
#include <string.h>


// change, make it modular
#define SHM_PATH                "/md5_shm_"
#define SEM_BUFF_PATH           "/md5_buff_sem_"
#define SEM_MUTEX_PATH          "/md5_mutex_"

typedef struct shared_memory_cdt * shared_memory_adt;

shared_memory_adt create_shared_memory(char *const uid, size_t buf_size, size_t sem_value);
void read_shared_memory(shared_memory_adt shared_memory, char *file_path, char *md5, int *slave_id);
void write_shared_memory(shared_memory_adt shared_memory, char *const file_path, char *const md5, int slave_id);
void open_semaphores(shared_memory_adt shared_memory);
void close_semaphores(shared_memory_adt shared_memory);
void destroy_resources(shared_memory_adt shared_memory);
void unlink_all_semaphores(shared_memory_adt shared_memory);
size_t get_processed_files(shared_memory_adt shared_memory) {

#endif // __SHM_MANAGER_H__
