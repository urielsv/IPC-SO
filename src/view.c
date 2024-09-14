#include <stdio.h>
#include "include/shm_utils.h"
#include "include/utils.h"
#include "include/view.h"
#include "include/defs.h"

int validate_arguments(char *shm_path, char *buff_sem_path, char *mutex_sem_path) {
    if (shm_path == NULL || buff_sem_path == NULL || mutex_sem_path == NULL) {
        return -1;
    }

    // Now we check that they start with the correct prefix
    // if (strncmp(shm_path, SHM_PREFIX, strlen(SHM_PREFIX)) != 0 ||
    //     strncmp(buff_sem_path, SEM_BUFF_PREFIX, strlen(SEM_BUFF_PREFIX)) != 0 ||
    //     strncmp(mutex_sem_path, SEM_MUTEX_PREFIX, strlen(SEM_MUTEX_PREFIX)) != 0) {
    //     return -1;
    // }

    return 0;
}

void load_parameters(int const argc, char *const argv, char *shm_path, char *buff_sem_path, char *mutex_sem_path) {
    if (argc == 4)  {
        strcpy(shm_path, &argv[1]);
        strcpy(buff_sem_path, &argv[2]);
        strcpy(mutex_sem_path, &argv[3]);        
        return;
    }

    // The arguments shoould have been passed through fd.
    // Read from stdin the paths of the shm, and sems
    if (scanf("%s", shm_path) == EOF) {
        fprintf(stderr, "Error: Could not read shm path from stdin\n");
        exit(EXIT_FAILURE);
    }

    size_t len = 0;
    getline(&shm_path, &len, stdin);
    // shm_path[len - 1] = '\0';

    getline(&buff_sem_path, &len, stdin);
    // buff_sem_path[len - 1] = '\0';

    getline(&mutex_sem_path, &len, stdin);
    // mutex_sem_path[len - 1] = '\0';


    if (validate_arguments(shm_path, buff_sem_path, mutex_sem_path) == -1) {
        fprintf(stderr, "Error: Invalid arguments\n");
        exit(EXIT_FAILURE);
    } else {
        // print them
        printf("shm_path: %s\n", shm_path);
        printf("buff_sem_path: %s\n", buff_sem_path);
        printf("mutex_sem_path: %s\n", mutex_sem_path);
    }
}





int main(int argc, char *const argv[]) {

    setvbuf_pipe(stdin, "Set stdin in view");
    
    setvbuf_pipe(stdout, "Set stdout in view");

//     char
// shm_path = malloc    // Read from stdin the paths of the shm, and sems

    // char *shm_path = calloc(1, sizeof(char));
    // char *buff_sem_path = calloc(1, sizeof(char));
    // char *mutex_sem_path = calloc(1, sizeof(char));

    char *shm_path = malloc(100 * sizeof(char));
    char *buff_sem_path = malloc(100 * sizeof(char));
    char *mutex_sem_path = malloc(100 * sizeof(char));
    load_parameters(argc, argv, shm_path, buff_sem_path, mutex_sem_path);

    free(shm_path);
    free(buff_sem_path);
    free(mutex_sem_path);
}
