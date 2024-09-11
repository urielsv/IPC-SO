
#include "include/shm_utils.h"
#include "include/utils.h"
#include "include/defs.h"
#include "include/shm_manager.h"

typedef struct buffer_t {
    char *base_addr;
    size_t size;
    size_t written;
    size_t read;
} buffer_t;

typedef struct semaphore_t {
    sem_t *semaphore;
    char *sem_path;
} semaphore_t;

typedef struct shm {
    int fd;
    char *shm_path;
} shm_t;

struct shared_memory_cdt {
    buffer_t *buffer;
    size_t files_processed;
    semaphore_t *full_buff_sem;
    semaphore_t *mutex_sem;
    shm_t *shm;
};

static void create_path_string(char **path, char *uid, char *prefix) {
    *path = calloc(strlen(prefix) + strlen(uid) + 1, sizeof(char));

    if (*path == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for path\n");
        exit(EXIT_FAILURE);
    }

    strcpy(*path, prefix);
    strcat(*path, uid); 
 
}

static semaphore_t * create_semaphore(char *const prefix, char *const uid, int initial_value) {

    semaphore_t *semaphore = calloc(1, sizeof(semaphore_t));
    if (semaphore == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for semaphore\n");
        exit(EXIT_FAILURE);
    }
    create_path_string(&semaphore->sem_path, uid, prefix);

    sem_t *sem = sem_open(semaphore->sem_path, O_CREAT, 0666, initial_value);
    if (sem == SEM_FAILED) {
        fprintf(stderr, "Error: Could not create semaphore.");
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    semaphore->semaphore = sem;
    return semaphore;
}


shared_memory_adt create_shared_memory(char *const uid, size_t buf_size, size_t sem_value) {
    shared_memory_adt shared_memory = calloc(1, sizeof(struct shared_memory_cdt));
    if (shared_memory == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for shared memory\n");
        exit(EXIT_FAILURE);
    }

    
    shared_memory->shm = calloc(1, sizeof(shm_t));
    if(shared_memory->shm == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for shared memory\n");
        free(shared_memory);
        exit(EXIT_FAILURE);
    }

    create_path_string(&shared_memory->shm->shm_path, uid, SHM_PATH);
    printf("Shared memory path: %s\n", shared_memory->shm->shm_path);
    // create_path_string(shared_memory->shm->shm_path, uid, SHM_PATH);
    // printf("Shared memory path: %s\n", shared_memory->shm->shm_path);
    // fprintf(stderr, "Shared memory path: %s\n", shared_memory->shm->shm_path);
    int fd = shm_open_util(shared_memory->shm->shm_path, O_CREAT | O_RDWR, "create_shared_memory");
    ftruncate_util(fd, buf_size, "create_shared_memory");
    shared_memory->shm->fd = fd;
    // Buffer
    shared_memory->buffer = calloc(1, sizeof(buffer_t));
    shared_memory->buffer->size = buf_size;

    // Map shared memory to the address space of the calling process (no se si esta bien)
    shared_memory->buffer->base_addr = mmap(
        NULL,
        shared_memory->buffer->size + sizeof(long),
        PROT_READ | PROT_WRITE, MAP_SHARED,
        shared_memory->shm->fd,
        0
    );

    if (shared_memory->buffer->base_addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Semaphores
    shared_memory->full_buff_sem = create_semaphore(SEM_BUFF_PATH, uid, sem_value);
    shared_memory->mutex_sem = create_semaphore(SEM_MUTEX_PATH, uid, sem_value);
    
    // Other
    shared_memory->files_processed = 0;


    return shared_memory;

}

void open_semaphores(shared_memory_adt shared_memory) {
    if (shared_memory == NULL) {
        fprintf(stderr, "Error: Shared memory is NULL during open_semaphore\n");
        exit(EXIT_FAILURE);
    }

    open_semaphore(shared_memory->full_buff_sem->sem_path, &shared_memory->full_buff_sem->semaphore);
    open_semaphore(shared_memory->mutex_sem->sem_path, &shared_memory->mutex_sem->semaphore);
}

void close_semaphores(shared_memory_adt shared_memory) {
    if (shared_memory == NULL) {
        fprintf(stderr, "Error: Shared memory is NULL during close_semaphore\n");
        exit(EXIT_FAILURE);
    }

    close_semaphore(shared_memory->full_buff_sem->semaphore, shared_memory->full_buff_sem->sem_path);
    close_semaphore(shared_memory->mutex_sem->semaphore, shared_memory->mutex_sem->sem_path);
}


void write_shared_memory(shared_memory_adt shared_memory, char *const file_path, char *const md5, int slave_id) {
    semaphore_down(shared_memory->mutex_sem->semaphore);

    
    size_t written = shared_memory->buffer->written++;

    
    char *buffer = shared_memory->buffer->base_addr;
    size_t file_path_size = strlen(file_path);
    size_t md5_size = strlen(md5);
    size_t slave_id_size = snprintf(NULL, 0, "%d", slave_id);
    char slave_id_str[slave_id_size + 1];
    snprintf(slave_id_str, slave_id_size + 1, "%d", slave_id);

    size_t total_size = md5_size + 1 + file_path_size + 1 + slave_id_size + 1 + 1; // md5\0file_path\0slave_id\0\n

    if (written + total_size > shared_memory->buffer->size) {
        fprintf(stderr, "Error: Not enough space in buffer to write data\n");
        exit(EXIT_FAILURE);
    }

    char *write_ptr = buffer + written;
    memcpy(write_ptr, md5, md5_size);
    write_ptr += md5_size;
    *write_ptr++ = '\0';

    memcpy(write_ptr, file_path, file_path_size);
    write_ptr += file_path_size;
    *write_ptr++ = '\0';

    memcpy(write_ptr, slave_id_str, slave_id_size);
    write_ptr += slave_id_size;
    *write_ptr++ = '\0';

    *write_ptr++ = '\n';

    shared_memory->files_processed++;
    shared_memory->buffer->written += total_size;


    semaphore_up(shared_memory->mutex_sem->semaphore);
    semaphore_up(shared_memory->full_buff_sem->semaphore);
}


void read_shared_memory(shared_memory_adt shared_memory, char *file_path, char *md5, int *slave_id) {
    semaphore_down(shared_memory->full_buff_sem->semaphore);
    semaphore_down(shared_memory->mutex_sem->semaphore);

    
    size_t read = shared_memory->buffer->read;
    char *buffer = shared_memory->buffer->base_addr + read;
    char *read_ptr = buffer;

    
    strcpy(file_path, read_ptr);
    read_ptr += strlen(file_path) + 1;

    
    strcpy(md5, read_ptr);
    read_ptr += strlen(md5) + 1;

    
    *slave_id = atoi(read_ptr);
    read_ptr += strlen(read_ptr) + 1;

    
    shared_memory->buffer->read += (read_ptr - buffer) + 1;

    semaphore_up(shared_memory->mutex_sem->semaphore);
    semaphore_up(shared_memory->full_buff_sem->semaphore);
}



static void destroy_semaphores(shared_memory_adt shared_memory) {

    close_semaphores(shared_memory);
    
    free(shared_memory->full_buff_sem->sem_path);
    free(shared_memory->mutex_sem->sem_path);
    free(shared_memory->full_buff_sem);
    free(shared_memory->mutex_sem);
}

void destroy_resources(shared_memory_adt shared_memory) {
    if (shared_memory == NULL) {
        fprintf(stderr, "Error: Shared memory is NULL during destroy_shared_memory\n");
        return;
    }

    if (shared_memory->buffer != NULL) {
        if (shared_memory->buffer->base_addr != NULL) {
            munmap(shared_memory->buffer->base_addr, shared_memory->buffer->size);
        }
        free(shared_memory->buffer);
    }

    if (shared_memory->shm != NULL) {
        shm_unlink(shared_memory->shm->shm_path);
        close_pipe(shared_memory->shm->fd, "shared_memory fd");
        free(shared_memory->shm->shm_path);
        free(shared_memory->shm);
    }

    if (shared_memory->full_buff_sem != NULL &&
        shared_memory->mutex_sem != NULL
        ) {
        destroy_semaphores(shared_memory);
    }

    free(shared_memory);
}

void unlink_all_semaphores(shared_memory_adt shared_memory) {
    unlink_sem(shared_memory->full_buff_sem->sem_path);
    unlink_sem(shared_memory->mutex_sem->sem_path);
}

size_t get_processed_files(shared_memory_adt shared_memory) {
    return shared_memory->files_processed;
}