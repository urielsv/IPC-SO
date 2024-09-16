// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "include/view.h"

int main(int argc, char *argv[]) {

    char *shm_path = NULL;
    char *buff_sem_path = NULL;
    char *mutex_sem_path = NULL;

    if (argc>1){

        if (argc!=4){
            fprintf(stderr, "Only three parameters are allowed for the view process\n");
            exit(EXIT_FAILURE);
        }

    }
    else {
        setvbuf_pipe(stdin, "view process (stdin)");
        setvbuf_pipe(stdout, "view process (stdout)");
    }

    load_parameters(argc, argv, &shm_path, &buff_sem_path, &mutex_sem_path);

    shared_memory_adt shared_memory = attach_shared_memory(shm_path, buff_sem_path, mutex_sem_path, SHM_BUFFER_SIZE);


    char file_path[BUFF_SIZE];
    char md5[ENC_SIZE + 1];
    char slave_id[PID_MAX_SIZE];
    int i =0;
    while (read_shared_memory(shared_memory, file_path, md5, slave_id) > 0) {
        fprintf(stderr, "(file: %d %s) %s, %s\n", i++, file_path, md5, slave_id);
    }

    printf("\nfinishing\n");
    deattach_shared_memory(shared_memory);
    free(shm_path);

    return 0;
}

// Utility function to read a line from stdin and remove the newline character
char* read_line_from_stdin() {
    char *line = NULL;
    size_t len = 0;
    if (getline(&line, &len, stdin) == -1) {
        free(line);
        fprintf(stderr, "Error: Could not read line from stdin\n");
        exit(EXIT_FAILURE);
    }
    line[strcspn(line, "\n")] = '\0'; // Remove newline character if present
    return line;
}

int validate_arguments(char *shm_path, char *buff_sem_path, char *mutex_sem_path) {
    if (shm_path == NULL || buff_sem_path == NULL || mutex_sem_path == NULL) {
        return -1;
    }

    // Now we check that they start with the correct prefix
    if (strncmp(shm_path, SHM_PREFIX, strlen(SHM_PREFIX)) != 0 ||
        strncmp(buff_sem_path, SEM_BUFF_PREFIX, strlen(SEM_BUFF_PREFIX)) != 0 ||
        strncmp(mutex_sem_path, SEM_MUTEX_PREFIX, strlen(SEM_MUTEX_PREFIX)) != 0) {
        return -1;
    }

    return 0;
}

void load_parameters(int argc, char *argv[], char **shm_path, char **buff_sem_path, char **mutex_sem_path) {
    if (argc == 4) {
        *shm_path = strdup(argv[1]);
        *buff_sem_path = strdup(argv[2]);
        *mutex_sem_path = strdup(argv[3]);
        printf("%s, %s, %s", *shm_path, *buff_sem_path, *mutex_sem_path);
        return;
    }

    // Read from stdin the paths of the shm, and sems
    *shm_path = read_line_from_stdin();
    *buff_sem_path = read_line_from_stdin();
    *mutex_sem_path = read_line_from_stdin();

}
