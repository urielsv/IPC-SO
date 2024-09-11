#include "include/shm_utils.h"


int  shm_open_util(char *const shm_path, int flags, char *const shm_error) {
    // We create a shared memory object to store the data,
    // using the flags O_CREAT and O_RDWR to create it if it does not exist
    int shm_fd = shm_open(shm_path, flags, 0666);
    
    if (shm_fd == -1) {
        fprintf(stderr, "Error: Could not open shared memory during %s\n", shm_error);
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    return shm_fd;
}

// ftruncate will set the size of the shared memory object to the size of the data
void  ftruncate_util(int shm_fd, size_t size, char *const shm_error) {
    // We truncate the shared memory object to the size of the data
    if (ftruncate(shm_fd, size) == -1) {
        fprintf(stderr, "Error: Could not truncate shared memory during %s\n", shm_error);
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
}

// Create the semaphore
void  open_semaphore(char *const sem_path, sem_t **semaphore) {
    *semaphore = sem_open(sem_path, O_RDWR);
    if (*semaphore == SEM_FAILED) {
        fprintf(stderr, "Error: Could not open semaphore during open_semaphore\n");
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

void close_semaphore(sem_t *semaphore, char *const sem_path) {
    if (sem_close(semaphore) == -1) {
        fprintf(stderr, "Error: Could not close semaphore during close_semaphore for sem: %s\n", sem_path);
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(sem_path) == -1) {
        fprintf(stderr, "Error: Could not unlink semaphore during close_semaphore for sem: %s\n", sem_path);
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
}

void  semaphore_up(sem_t *semaphore) {
    if (sem_post(semaphore) == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
}

void  semaphore_down(sem_t *semaphore) {
    if (sem_wait(semaphore) == -1) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
}

void  shm_close(int shm_fd, char *const shm_path) {
    if (shm_unlink(shm_path) == -1) {
        fprintf(stderr, "Error: Could not unlink shared memory during shm_close\n");
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
}

// unlink sem
void  unlink_sem(char *const sem_path) {
    if (sem_unlink(sem_path) == -1) {
        fprintf(stderr, "Error: Could not unlink semaphore during unlink_sem\n");
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
}
