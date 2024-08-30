// 1. Recibir input los archivos (./md5 files/*)
// No hay falta expandir el *
// podemos usar pipes por ejemplo ./md5 files/* | ./view

// 2. Iniciar procesos esclavos

// 3. Distribuir archivos entre distintos esclavlos

// 4. Si un slave se libera se le asigna otro archivo a procesar

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include "../include/md5.h"

#define MAX_FILES               100
#define MAX_INITIAL_FILES       10
#define MAX_SLAVES              3
#define REQUIRED_ARGS           2
#define SLAVE_PATH              "./slave"

/*
 * Calculate the number of files per slave
 */
#define files_per_slave(files) ((files + MAX_SLAVES - 1) / MAX_SLAVES)


int main(int argc, char *const argv[]) {
    if (argc < REQUIRED_ARGS) {
        printf("Usage: %s <files> ...", argv[0]);
        return 1;
    }

    /* Validate paths */
    for (int i = 1; i < argc; i++) {
        struct stat path_stat;
        if (stat(argv[i], &path_stat) != 0) {
            fprintf(stderr, "Error: Could not find %s\n", argv[i]);
            return 1;
        }
    }

    /* TEMP: Print the argv paths */
    for (int i = 1; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    // DIR* dir;
    // if (!(dir = opendir(argv[1]))) {
    //     fprintf(stderr, "Error: Could not open directory\n");
    //     return 1;
    // }

    init_slaves(argv, MAX_INITIAL_FILES / MAX_SLAVES);
    int files_count = files_per_slave(argc - 1);

    /* Then process files starting from MAX_INITIAL_FILES + 1 */
    return 0;
}

int assign_slaves(char *argv[], int files_count) {

    return 0;
}

/*
 * Initialize the slave processes
 * @param files_path: The path to the files to process
 * @param files_count: The number of files to process per slave
 */
int init_slaves(char *files_path[], int files_count) {
    int files_assigned = 0;
    for (int i = 0; i < MAX_SLAVES; i++) {
        char* files[files_count];
        for (int j = 0; j < files_count; j++) {
            files[j] = files_path[files_assigned + j];
        }
        create_slave(files);
        files_assigned += files_count;
    }
    return 0;
}

/*
 * Create a slave process
 * @param files_path: The path to the files to process
 */
void create_slave(char *files_path[]){
    pid_t pid = fork();
    if (pid < 0){
        fprintf(stderr, "Error: Could not create slave\n");
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // Child process
    if (pid == 0){
        printf("Child process\n");
        char *const files[] = { (char *)files_path, NULL };
        char *const envp[] = { NULL };

        check_program_path(SLAVE_PATH);
        execve(SLAVE_PATH, files, envp);
        perror("execve");
        exit(EXIT_FAILURE);
    }
}

static void check_program_path(char *path) {
    if (access(path, F_OK) == -1) {
        fprintf(stderr, "Error: Could not find %s\n", path);
        exit(EXIT_FAILURE);
    }
}
