#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include "include/shm_utils.h"
#include "include/utils.h"
int inline shm_open_util(char *const shm_error) {
    // We create a shared memory object to store the data,
    // using the flags O_CREAT and O_RDWR to create it if it does not exist
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        fprintf(stderr, "Error: Could not open shared memory during %s\n", shm_error);
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    return shm_fd;
}

shared_memory_t *create_shared_memory() {
    shared_memory_t * shared_memory = calloc(1, sizeof(shared_memory_t));
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
    shared_memory->semaphore = NULL;
    // Create the semaphore


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
int create_semaphore(shared_memory_t *shared_memory, int initial_value) {

    if (shared_memory == NULL) {
        fprintf(stderr, "Error: Shared memory is NULL during create_semaphore\n");
        exit(EXIT_FAILURE);
    }

    if (shared_memory->semaphore != NULL) {
        fprintf(stderr, "Error: Semaphore already exists during create_semaphore\n");
        exit(EXIT_FAILURE);
    }


    shared_memory->semaphore = sem_open(SEM_NAME, O_CREAT, 0666, initial_value);
    if (shared_memory->semaphore == SEM_FAILED) {
        fprintf(stderr, "Error: Could not create semaphore.");
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    return 0;
}

// Open the semaphore
void open_semaphore(shared_memory_t *shared_memory) {
    if (shared_memory == NULL) {
        fprintf(stderr, "Error: Shared memory is NULL during open_semaphore\n");
        exit(EXIT_FAILURE);
    }

    shared_memory->semaphore = sem_open(SEM_NAME, O_RDWR);
    if (shared_memory->semaphore == SEM_FAILED) {
        fprintf(stderr, "Error: Could not open semaphore during open_semaphore\n");
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

// Close the semaphore
void close_semaphore(shared_memory_t *shared_memory) {
    if (shared_memory == NULL) {
        fprintf(stderr, "Error: Shared memory is NULL during close_semaphore\n");
        exit(EXIT_FAILURE);
    }

    if (sem_close(shared_memory->semaphore) == -1) {
        fprintf(stderr, "Error: Could not close semaphore during close_semaphore\n");
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
}

// Write to the shared memory
void write_shared_memory(shared_memory_t *shared_memory, char *const file_path, char *const md5, int slave_id) {
    semaphore_down(shared_memory->semaphore);

    size_t written = shared_memory->buffer->written++;
    struct file_info_t *fi = &shared_memory->buffer->file_info[written % MAX_FILES];

    fi->slave_id = slave_id;
    strcpy(fi->file_path, file_path);
    strcpy(fi->md5, md5);

    // printf("%s\n", shared_memory->buffer->file_info[written % MAX_FILES].file_path);
    // printf("%s\n", shared_memory->buffer->file_info[written % MAX_FILES].md5);
    // printf("%d\n", shared_memory->buffer->file_info[written % MAX_FILES].slave_id);

    semaphore_up(shared_memory->semaphore);
}

void inline semaphore_up(sem_t *semaphore) {
    if (sem_post(semaphore) == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
}

void inline semaphore_down(sem_t *semaphore) {
    if (sem_wait(semaphore) == -1) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
}
// Read from the shared memory
void read_shared_memory(shared_memory_t *shared_memory, char *file_path, char *md5, int *slave_id) {
    semaphore_down(shared_memory->semaphore);

    size_t read = shared_memory->buffer->read++;
    memcpy(file_path, shared_memory->buffer->file_info[read].file_path, FILE_PATH_SIZE);
    memcpy(md5, shared_memory->buffer->file_info[read].md5, MD5_HASH_SIZE);
    *slave_id = shared_memory->buffer->file_info[read].slave_id;

    semaphore_up(shared_memory->semaphore);

}



void destroy_shared_memory(shared_memory_t *shared_memory) {
    if (shared_memory == NULL) {
            fprintf(stderr, "Error: Shared memory is NULL during destroy_shared_memory\n");
            exit(EXIT_FAILURE);
        }

        if (shared_memory->buffer != NULL) {
            free(shared_memory->buffer);
        }

        if (munmap(shared_memory, sizeof(shared_memory_t)) == -1) {
            perror("munmap");
            exit(EXIT_FAILURE);
        }

        close_pipe(shared_memory->fd, "shared_memory fd");
        free(shared_memory);
}

// destroy the semaphore
void destroy_semaphore(shared_memory_t *shared_memory) {
    if (sem_unlink(SEM_NAME) == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    close_semaphore(shared_memory);

}

// unlink the shared memory
void shm_unlink_util(char *const shm_error) {
    if (shm_unlink(SHM_NAME) == -1) {
        fprintf(stderr, "Error: Could not unlink shared memory during %s\n", shm_error);
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
}
