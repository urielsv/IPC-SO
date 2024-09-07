#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include "include/shm_utils.h"

int inline shm_open_util(char *const shm_error) {
    // We create a shared memory object to store the data,
    // using the flags O_CREAT and O_RDWR to create it if it does not exist
    int shm_fd = shm_open("/my_shm", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        fprintf(stderr, "Error: Could not open shared memory during %s\n", shm_error);
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    return shm_fd;
}

shared_memory_t *create_shared_memory(char *const semaphore_name, size_t semaphore_value) {
    shared_memory_t * shared_memory = malloc(sizeof(shared_memory_t));
    if (shared_memory == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for shared memory\n");
        exit(EXIT_FAILURE);
    }
    int fd = shm_open_util("create_shared_memory");
    ftruncate_util(fd, MD5_HASH_SIZE * MAX_FILES, "create_shared_memory");

    // Map shared memory
    shared_memory = (shared_memory_t *) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    shared_memory->fd = fd;
    shared_memory->buffer = calloc(1, sizeof(struct buffer_t));
    shared_memory->files_processed = 0;
    // Create the semaphore


    shared_memory->semaphore = create_semaphore(semaphore_name, semaphore_value, "create_shared_memory");
    return shared_memory;
    
}
// ftruncate will set the size of the shared memory object to the size of the data
void inline ftruncate_util(int shm_fd, size_t size, char *const shm_error) {
    // We truncate the shared memory object to the size of the data
    if (ftruncate(shm_fd, size) == -1) {
        fprintf(stderr, "Error: Could not truncate shared memory during %s\n", shm_error);
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
}

// Create the semaphore
semaphore_t *create_semaphore(char *const sem_name, int initial_value, char *const sem_error) {
    // Check the semaphore name is correct
    if (sem_name == NULL) {
        fprintf(stderr, "Error: Semaphore name is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (sem_name[0] != '/') {
        fprintf(stderr, "Error: Semaphore name must start with a '/'\n");
        exit(EXIT_FAILURE);
    }

    sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        fprintf(stderr, "Error: Could not create semaphore during %s\n", sem_error);
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    semaphore_t * semaphore = malloc(sizeof(semaphore_t));
    if (semaphore == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for semaphore\n");
        exit(EXIT_FAILURE);
    }

    semaphore->semaphore = sem;
    semaphore->name = sem_name;
    semaphore->value = initial_value;

    return semaphore;
}

// Open the semaphore
sem_t *open_semaphore(char *const sem_name, char *const sem_error) {
    sem_t *sem = sem_open(sem_name, O_RDWR);
    if (sem == SEM_FAILED) {
        fprintf(stderr, "Error: Could not open semaphore during %s\n", sem_error);
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return sem;
}

// Close the semaphore
void close_semaphore(sem_t *sem, char *const sem_error) {
    if (sem_close(sem) == -1) {
        fprintf(stderr, "Error: Could not close semaphore during %s\n", sem_error);
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
}

// Write to the shared memory
void write_shared_memory(shared_memory_t *shared_memory, struct file_info_t file_info) {
    if (sem_wait(shared_memory->semaphore->semaphore) == -1) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
    // We write the data appending it to the buffer
    size_t written = ++shared_memory->buffer->written;
    struct file_info_t *fi = &shared_memory->buffer->file_info[written];
    // We copy the data to the shared memory
    memcpy(fi->file_path, file_info.file_path, strlen(file_info.file_path) + 1);
    memcpy(fi->slave_id, file_info.slave_id, strlen(file_info.slave_id) + 1);
    memcpy(fi->md5, file_info.md5, strlen(file_info.md5) + 1);

    if (sem_post(shared_memory->semaphore->semaphore) == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
}

// Read from the shared memory
void read_shared_memory(shared_memory_t *shared_memory, char *buf, size_t size) {
    if (sem_wait(shared_memory->semaphore->semaphore) == -1) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }

    // We read the data from the buffer
    // memcpy(buf, shared_memory->buffer->data + shared_memory->buffer->read, size);
    shared_memory->buffer->read += size;
    if (sem_post(shared_memory->semaphore->semaphore ) == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }

}



void destroy_shared_memory(shared_memory_t *shared_memory) {
    // if (munmap(shared_memory, BUF_SIZE) == -1) {
    //     perror("munmap");
    //     exit(EXIT_FAILURE);
    // }
    // if (close(shared_memory->fd) == -1) {
    //     perror("close");
    //     exit(EXIT_FAILURE);
    // }
    free(shared_memory);
    // shm_unlink_util("destroy_shared_memory");
}
