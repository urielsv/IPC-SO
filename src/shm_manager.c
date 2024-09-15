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




static char *mmap_buffer(int fd, size_t buffer_size, int flags) {
    return (char *) mmap(
        NULL,
        buffer_size,
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
    printf("SHJM!! %s\n", shm_path);
    printf("SHJM!! %s\n", full_buff_sem_path);
    printf("SHJM!! %s\n", mutex_sem_path);
    int fd = shm_open_util(shm_path, O_RDWR, "attaching shared memory");
    if (fd < 0) {
        perror("shm_open_util");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "FD: %d", fd);


    shared_memory_adt shared_memory = calloc(1, sizeof(struct shared_memory_cdt));
    shared_memory->buffer = calloc(1, sizeof(buffer_t));
    shared_memory->buffer->base_addr = mmap_buffer(fd, buffer_size, PROT_READ | PROT_WRITE);
    // shared_memory->buffer->base_addr = shared_memory->buffer;
    fprintf(stderr,"Base addr was read from: %p", shared_memory->buffer);
    if (shared_memory->buffer == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    shared_memory->shm->fd;
    shared_memory->full_buff_sem = calloc(1, sizeof(semaphore_t));
    shared_memory->mutex_sem = calloc(1, sizeof(semaphore_t));
    shared_memory->full_buff_sem->sem_path = full_buff_sem_path;
    shared_memory->mutex_sem->sem_path = mutex_sem_path;

    open_semaphores(shared_memory);
    // shared_memory->shm = calloc(1, sizeof(shm_t));
    // if (shared_memory->shm == NULL) {
    //     perror("calloc");
    //     munmap(shared_memory, sizeof(struct shared_memory_cdt) + buffer_size);
    //     close(fd);
    //     exit(EXIT_FAILURE);
    // }
    // shared_memory->shm->fd = fd;
    // shared_memory->shm->shm_path = strdup(shm_path);
    // if (shared_memory->shm->shm_path == NULL) {
    //     perror("strdup");
    //     free(shared_memory->shm);
    //     munmap(shared_memory, sizeof(struct shared_memory_cdt) + buffer_size);
    //     close(fd);
    //     exit(EXIT_FAILURE);
    // }

    // shared_memory->buffer = malloc (sizeof(buffer_t));
    // shared_memory->buffer->base_addr = malloc(buffer_size);
    // shared_memory->buffer = (buffer_t *)((char *)shared_memory + sizeof(struct shared_memory_cdt));
    // shared_memory->buffer->base_addr = (char *)shared_memory->buffer + sizeof(buffer_t);
    // shared_memory->buffer->size = buffer_size;
     shared_memory->buffer->read = 0;
     shared_memory->buffer->written = 0;


    // shared_memory->files_processed = 0;

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
    fprintf(stderr, "FD: %d", fd);

    if (ftruncate(fd, sizeof(struct shared_memory_cdt) + buf_size) < 0) {
        perror("ftruncate");
        close(fd);
        exit(EXIT_FAILURE);
    }

    shared_memory_adt shared_memory = calloc(1, sizeof(struct shared_memory_cdt));
    shared_memory->buffer = calloc(1, sizeof(buffer_t));
    shared_memory->buffer->base_addr = mmap_buffer(fd, buf_size, PROT_READ | PROT_WRITE);
    //shared_memory->buffer = malloc(sizeof(buffer_t))
    // shared_memory->buffer = mmap_buffer(fd, buf_size, PROT_READ | PROT_WRITE);
    fprintf(stderr, "\nBase addr was created in : %p", shared_memory->buffer);
    if (shared_memory->buffer == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    shared_memory->shm = calloc(1, sizeof(shm_t));
    if (shared_memory->shm == NULL) {
        perror("calloc");
        // munmap(shared_memory, sizeof(struct shared_memory_cdt) + buf_size);
        close(fd);
        exit(EXIT_FAILURE);
    }
    shared_memory->shm->fd = fd;
    shared_memory->shm->shm_path = shm_path;

    shared_memory->buffer->size = buf_size;
    shared_memory->buffer->read = 0;
    shared_memory->buffer->written = 0;

    shared_memory->full_buff_sem = create_semaphore(SEM_BUFF_PREFIX, uid, 0);
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



    size_t slave_id_size = snprintf(NULL, 0, "%d", slave_id);
    char slave_id_str[slave_id_size + 1];
    snprintf(slave_id_str, slave_id_size + 1, "%d", slave_id);
    
    char *write_ptr = shared_memory->buffer->base_addr + written;

    size_t total_size = 0;    
    size_t curr_size = 0;

    curr_size = strlen(file_path) + 1;
    memcpy(write_ptr, file_path, curr_size);
    fprintf(stderr, "\n(write, file_path) %s", write_ptr);
    total_size += curr_size;
    write_ptr += curr_size;

    curr_size = strlen(md5) + 1;
    memcpy(write_ptr, md5, curr_size);
    fprintf(stderr, "\n(write, md5) %s", write_ptr);
    write_ptr += curr_size;
    total_size += curr_size;
    
    curr_size = strlen(slave_id_str) + 1;
    memcpy(write_ptr, slave_id_str, curr_size);
    fprintf(stderr, "\n(write, slave_id) %s", write_ptr);
    write_ptr += curr_size;
    total_size += curr_size;

    shared_memory->buffer->written += total_size;

    

    semaphore_up(shared_memory->mutex_sem->semaphore);
    semaphore_up(shared_memory->full_buff_sem->semaphore);
}


void strcopy(char *dest, const char *src) {
    while (*src != '\0') { // Mientras no se llegue al final de la cadena
        *dest = *src; // Copiar carácter de src a dest
        fprintf(stderr, "Copying character: '%c'\n", *src); // Imprimir carácter copiado
        dest++; // Mover puntero a la siguiente posición en dest
        src++; // Mover puntero a la siguiente posición en src
    }
    *dest = '\0'; // Agregar el carácter nulo al final de la cadena copiada
    fprintf(stderr, "Copying character: '%c'\n", *src); // Imprimir el carácter nulo
}


void read_shared_memory(shared_memory_adt shared_memory, char *file_path, char *md5, char *slave_id) {
    
    semaphore_down(shared_memory->full_buff_sem->semaphore);
    semaphore_down(shared_memory->mutex_sem->semaphore);
    fprintf(stderr, "\n\nReading from shared memory\n");

    char *read_ptr = shared_memory->buffer->base_addr + shared_memory->buffer->read;
    size_t total_size = 0;
    fprintf(stderr, "\n(view, read, expeted incremention by 45 each time) %d", shared_memory->buffer->read);


      for (size_t i = 0; i < 45;) {
        // Imprimir caracteres hasta el siguiente carácter nulo
        fprintf(stderr, "\nData: ");
        while(i < 1000) {
            fprintf(stderr, "%c", shared_memory->buffer->base_addr[i]);
            if(shared_memory->buffer->base_addr[i] == '\0') {
                fprintf(stderr, ",");
            }
            i++;
        }
       }

    fprintf(stderr, "\n");fprintf(stderr, "\n");

    size_t curr_size = 0;

    strcpy(file_path, read_ptr);
    fprintf(stderr,"\n(view, fp) %s", file_path);
    curr_size = strlen(file_path) + 1;
    total_size += curr_size;
    read_ptr += curr_size;


    //fprintf(stderr,"\n(view, read_ptr expected md5) %s", read_ptr);
    strcpy(md5, read_ptr);
    fprintf(stderr,"\n(view, md5) %s", md5);
    curr_size = strlen(md5) + 1;
    total_size += curr_size;
    read_ptr += curr_size;


    strcpy(slave_id, read_ptr);
    fprintf(stderr,"\n(view, slaveid) %s", slave_id);
    total_size += strlen(read_ptr) + 1;



    shared_memory->buffer->read += total_size;
    fprintf(stderr,"\n(view, total size expected 40 aprox) %d", total_size);

    // printf("\n(view) %s", read_ptr);
    shared_memory->files_processed++;

    semaphore_up(shared_memory->mutex_sem->semaphore);
    //semaphore_up(shared_memory->full_buff_sem->semaphore);
    // shared_memory->buffer->read += 6;
    // size_t read = shared_memory->buffer->read;
    // char *buffer = shared_memory->buffer->base_addr + read;
    // printf("read_from_shm: %s\n", buffer);
    // char *read_ptr = buffer;

    // strcpy(file_path, read_ptr);
    // read_ptr += strlen(file_path) + 1;

    // strcpy(md5, read_ptr);
    // read_ptr += strlen(md5) + 1;

    // *slave_id = atoi(read_ptr);
    // read_ptr += strlen(read_ptr) + 1;

    // shared_memory->buffer->read += (read_ptr - buffer) + 1;

    // semaphore_up(shared_memory->full_buff_sem->semaphore);
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

    if (shared_memory->shm != NULL) {
        close(shared_memory->shm->fd);
        shm_unlink(shared_memory->shm->shm_path);
        free(shared_memory->shm->shm_path);
        free(shared_memory->shm);
    }

    if (shared_memory->full_buff_sem != NULL && shared_memory->mutex_sem != NULL) {
        destroy_semaphores(shared_memory);
    }
    if (shared_memory->buffer != NULL) {
         if (shared_memory->buffer->base_addr != NULL) {
             munmap(shared_memory->buffer->base_addr, shared_memory->buffer->size);
         }
        // free(shared_memory->buffer->base_addr);
         free(shared_memory->buffer);
    }
    // munmap(shared_memory, 4096);

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
