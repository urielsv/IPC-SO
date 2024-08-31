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
#include <string.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

#include "../include/md5.h"

#define MAX_FILES               100
#define MAX_INITIAL_FILES       10
#define MAX_SLAVES              3
#define REQUIRED_ARGS           2
#define SLAVE_PATH              "./slave"

/*
 * Calculate the number of files per slave
 */
#define files_per_slave(files) (files < MAX_INITIAL_FILES ? files : MAX_INITIAL_FILES / MAX_SLAVES)

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

    slave_t *slaves[MAX_SLAVES];
    // Initialize the slaves memory
    for (int i = 0; i < MAX_SLAVES; i++) {
        slaves[i] = (slave_t *) malloc(sizeof(slave_t));
    }    

    uint32_t files_assigned = 0;
    files_assigned = init_slaves(argv, files_per_slave(argc-1), slaves);

    // Assign the rest of the files to the slaves
    while(argv[files_assigned] != NULL) {
        char *file_path = argv[files_assigned++];
        assign_files(&slaves[0], file_path, 1); 
    }

    return 0;
}

int assign_files(slave_t *slave, char *const files_path[], int files_count) {
    // Send the files to the slave via the pipe using close, dup2

    for (int i = 0; i < files_count; i++) {
        write(slave->pipefd[1], files_path[i], strlen(files_path[i]));
    }
    
    
    return 0;
}

/*
 * Initialize the slave processes
 * @param files_path: The path to the files to process
 * @param files_count: The number of files to process per slave
 * @param slaves: The slave processes
 *
 * @return The number of files assigned total 
 */
int init_slaves(char *files_path[], int files_count, slave_t **slaves) {
    int files_assigned = 0;
    for (int i = 0; i < MAX_SLAVES; i++) {

        // Assign initial files per slaves
        char* files[files_count];
        for (int j = 0; j < files_count && files_path != NULL; j++) {
            files[j] = files_path[files_assigned + j];
        }

        create_slave(slaves[i]);
        

        // Initialize the pipe
        if (pipe(slaves[i]->pipefd) == -1) {
            fprintf(stderr, "Error: Could not create pipe for pid: %d\n", slaves[i]->pid);
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        //Assign initial files to slave
        assign_files(slaves[i], files, files_count);
        files_assigned += files_count;
    }
    return files_assigned;
}

/*
 * Create a slave process
 * @param files_path: The path to the files to process
 */
pid_t create_slave(slave_t *slave) {
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error: Could not create slave\n");
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // fork succeeded, child process
    if (pid == 0) {
        slave->pid = getpid();
        char *const argv[] = { NULL };
        char *const envp[] = { NULL };

        close(slave->pipefd[1]);
        dup2(slave->pipefd[0], STDIN_FILENO);
        dup2(slave->pipefd[1], STDOUT_FILENO);

        check_program_path(SLAVE_PATH);
        execve(SLAVE_PATH, argv, envp);
        perror("execve");
        exit(EXIT_FAILURE); 
    }else{
        close(slave->pipefd[0]);
    }

    printf("Child process with pid: %d\n", getpid());
    return pid;
}

static void check_program_path(char *path) {
    if (access(path, F_OK) == -1) {
        fprintf(stderr, "Error: Could not find %s\n", path);
        exit(EXIT_FAILURE);
    }
}
