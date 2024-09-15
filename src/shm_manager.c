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




static shared_memory_adt mmap_memory(int fd, size_t buffer_size, int flags) {
    return (shared_memory_adt) mmap(
        NULL,
        sizeof(struct shared_memory_cdt) + buffer_size,
        flags, 
        MAP_SHARED,
        fd,
        0 
    );
}

static void create_path_string(char **path, const char *uid, const char *prefix) {
    *path = calloc(strlen(prefix) + strlen(uid) + 1, sizeof(char));
    if (*path == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for path\n");
        exit(EXIT_FAILURE);
    }
    strcpy(*path, prefix);
    strcat(*path, uid); 
}

static semaphore_t *create_semaphore(const char *prefix, const char *uid, int initial_value) {
    semaphore_t *semaphore = calloc(1, sizeof(semaphore_t));
    if (semaphore == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for semaphore\n");
        exit(EXIT_FAILURE);
    }
    create_path_string(&semaphore->sem_path, uid, prefix);

    sem_t *sem = sem_open(semaphore->sem_path, O_CREAT | O_EXCL, 0666, initial_value);
    if (sem == SEM_FAILED) {
        fprintf(stderr, "Error: Could not create semaphore.");
        perror("sem_open");
        free(semaphore->sem_path);
        free(semaphore);
        exit(EXIT_FAILURE);
    }

    semaphore->semaphore = sem;
    return semaphore;
}
shared_memory_adt attach_shared_memory(char *shm_path, char *full_buff_sem_path, char *mutex_sem_path, size_t buffer_size) {
    int fd = shm_open_util(shm_path, O_RDWR, "attaching shared memory");
    if (fd < 0) {
        perror("shm_open_util");
        exit(EXIT_FAILURE);
    }

    shared_memory_adt shared_memory = mmap_memory(fd, buffer_size, PROT_READ | PROT_WRITE);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    shared_memory->shm = calloc(1, sizeof(shm_t));
    if (shared_memory->shm == NULL) {
        perror("calloc");
        munmap(shared_memory, sizeof(struct shared_memory_cdt) + buffer_size);
        close(fd);
        exit(EXIT_FAILURE);
    }
    shared_memory->shm->fd = fd;
    shared_memory->shm->shm_path = strdup(shm_path);
    if (shared_memory->shm->shm_path == NULL) {
        perror("strdup");
        free(shared_memory->shm);
        munmap(shared_memory, sizeof(struct shared_memory_cdt) + buffer_size);
        close(fd);
        exit(EXIT_FAILURE);
    }

    shared_memory->buffer = (buffer_t *)((char *)shared_memory + sizeof(struct shared_memory_cdt));
    shared_memory->buffer->base_addr = (char *)shared_memory->buffer + sizeof(buffer_t);
    shared_memory->buffer->size = buffer_size;
    shared_memory->buffer->read = 0;
    shared_memory->buffer->written = 0;

    shared_memory->full_buff_sem = create_semaphore(SEM_BUFF_PREFIX, full_buff_sem_path, 0);
    shared_memory->mutex_sem = create_semaphore(SEM_MUTEX_PREFIX, mutex_sem_path, 1);

    shared_memory->files_processed = 0;

    return shared_memory;
}


shared_memory_adt create_shared_memory(char *uid, size_t buf_size, size_t sem_value) {
    char *shm_path = NULL;
    create_path_string(&shm_path, uid, SHM_PREFIX);

    int fd = shm_open_util(shm_path, O_CREAT | O_RDWR, "create_shared_memory");
    if (fd < 0) {
        perror("shm_open_util");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, sizeof(struct shared_memory_cdt) + buf_size) < 0) {
        perror("ftruncate");
        close(fd);
        exit(EXIT_FAILURE);
    }

    shared_memory_adt shared_memory = mmap_memory(fd, buf_size, PROT_READ | PROT_WRITE);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    shared_memory->shm = calloc(1, sizeof(shm_t));
    if (shared_memory->shm == NULL) {
        perror("calloc");
        munmap(shared_memory, sizeof(struct shared_memory_cdt) + buf_size);
        close(fd);
        exit(EXIT_FAILURE);
    }
    shared_memory->shm->fd = fd;
    shared_memory->shm->shm_path = shm_path;

    shared_memory->buffer = (buffer_t *)((char *)shared_memory + sizeof(struct shared_memory_cdt));
    shared_memory->buffer->base_addr = (char *)shared_memory->buffer + sizeof(buffer_t);
    shared_memory->buffer->size = buf_size;
    shared_memory->buffer->read = 0;
    shared_memory->buffer->written = 0;

    shared_memory->full_buff_sem = create_semaphore(SEM_BUFF_PREFIX, uid, sem_value);
    shared_memory->mutex_sem = create_semaphore(SEM_MUTEX_PREFIX, uid, sem_value);

    shared_memory->files_processed = 0;

    return shared_memory;
}

void open_semaphores(shared_memory_adt shared_memory) {
    if (shared_memory == NULL) {
        fprintf(stderr, "Error: Shared memory is NULL during open_semaphore\n");
        exit(EXIT_FAILURE);
    }

    shared_memory->full_buff_sem->semaphore = sem_open_util(shared_memory->full_buff_sem->sem_path, O_RDWR);
    shared_memory->mutex_sem->semaphore = sem_open_util(shared_memory->mutex_sem->sem_path, O_RDWR);
}

void close_semaphores(shared_memory_adt shared_memory) {
    if (shared_memory == NULL) {
        fprintf(stderr, "Error: Shared memory is NULL during close_semaphore\n");
        exit(EXIT_FAILURE);
    }

    close_semaphore(shared_memory->full_buff_sem->semaphore, shared_memory->full_buff_sem->sem_path);
    close_semaphore(shared_memory->mutex_sem->semaphore, shared_memory->mutex_sem->sem_path);
}

void write_shared_memory(shared_memory_adt shared_memory, char * const file_path, char * const md5, int slave_id) {
    semaphore_down(shared_memory->mutex_sem->semaphore);

    size_t written = shared_memory->buffer->written;
    size_t file_path_size = strlen(file_path);
    size_t md5_size = strlen(md5);
    size_t slave_id_size = snprintf(NULL, 0, "%d", slave_id);
    char slave_id_str[slave_id_size + 1];
    snprintf(slave_id_str, slave_id_size + 1, "%d", slave_id);

    size_t total_size = md5_size + 1 + file_path_size + 1 + slave_id_size + 1 + 1; // md5\0file_path\0slave_id\0\n

    if (written + total_size > shared_memory->buffer->size) {
        fprintf(stderr, "Error: Not enough space in buffer to write data\n");
        semaphore_up(shared_memory->mutex_sem->semaphore);
        exit(EXIT_FAILURE);
    }

    char *write_ptr = shared_memory->buffer->base_addr + written;
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
        close(shared_memory->shm->fd);
        free(shared_memory->shm->shm_path);
        free(shared_memory->shm);
    }

    if (shared_memory->full_buff_sem != NULL && shared_memory->mutex_sem != NULL) {
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

char *get_shm_path(shared_memory_adt shared_memory) {
    return shared_memory->shm->shm_path;
}

char *get_buff_sem_path(shared_memory_adt shared_memory) {
    return shared_memory->full_buff_sem->sem_path;
}

char *get_mutex_sem_path(shared_memory_adt shared_memory) {
    return shared_memory->mutex_sem->sem_path;
}
