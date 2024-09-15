#ifndef __SHM_MANAGER_H__
#define __SHM_MANAGER_H__

#include <stddef.h>
#include <semaphore.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>

typedef struct shared_memory_cdt *shared_memory_adt;

// Function to create shared memory and associated semaphores
shared_memory_adt create_shared_memory(char *const uid, size_t buf_size, size_t sem_value);

// Function to attach to existing shared memory and semaphores
shared_memory_adt attach_shared_memory(char *shm_path, char *full_buff_sem_path, char *mutex_sem_path, size_t buffer_size);

// Function to read data from shared memory
// void read_shared_memory(shared_memory_adt shared_memory, char *file_path, char *md5, int *slave_id);
void read_shared_memory(shared_memory_adt shared_memory, char *file_path, char *md5, int *slave_id);

// Function to write data to shared memory
void write_shared_memory(shared_memory_adt shared_memory, char *const file_path, char *const md5, int slave_id);

// Function to open semaphores
void open_semaphores(shared_memory_adt shared_memory);

// Function to close semaphores
void close_semaphores(shared_memory_adt shared_memory);

// Function to destroy shared memory and resources
void destroy_resources(shared_memory_adt shared_memory);

// Function to unlink all semaphores
void unlink_all_semaphores(shared_memory_adt shared_memory);

// Function to get the number of processed files
size_t get_processed_files(shared_memory_adt shared_memory);

// Function to get the shared memory path
char *get_shm_path(shared_memory_adt shared_memory);

// Function to get the buffer semaphore path
char *get_buff_sem_path(shared_memory_adt shared_memory);

// Function to get the mutex semaphore path
char *get_mutex_sem_path(shared_memory_adt shared_memory);

#endif // __SHM_MANAGER_H__
