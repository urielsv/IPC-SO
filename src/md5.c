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
#define files_per_slave(files) (files <= MAX_SLAVES ? 1 : 2)

int main(int argc, char *const argv[]) {
    if (argc < REQUIRED_ARGS) {
        printf("Usage: %s <files>", argv[0]);
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


    slave_t *slaves[MAX_SLAVES];
    // Initialize the slaves memory
    for (int i = 0; i < MAX_SLAVES; i++) {
        slaves[i] = (slave_t *) malloc(sizeof(slave_t));
        if (slaves[i] == NULL) {
            fprintf(stderr, "Error: Could not allocate memory for slave\n");
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    }

    uint32_t files_assigned;
    printf("%d", files_per_slave(argc-1));
    files_assigned = init_slaves(argv, files_per_slave(argc-1), slaves);

    // // Assign the rest of the files to the slaves
    // while(argv[files_assigned] != NULL) {
    //     char *file_path = argv[files_assigned++];
    //     assign_files(slaves[0], &file_path, 1);
    // }

    // free slaves
    for (int i = 0; i < MAX_SLAVES; i++) {
        waitpid(slaves[i]->pid, NULL, 0);
        free(slaves[i]);
    }

    return 0;
}

/**
 * Assign file to a slave by writing file paths to the slave's pipe.
 * @param slave: The slave process structure containing pipe file descriptors.
 * @param files_path: Array of file paths to be sent.
 * @param files_count: Number of file paths to send.
 * @return: 0 on success, -1 on failure.
 */
int assign_file(slave_t *slave, char *const file_path) {
    if (slave == NULL || file_path == NULL) {
        fprintf(stderr, "Error: Invalid arguments to assign_file.\n");
        return -1;
    }

    // Debug
    // pipefd[0] is the read end of the pipe
    // pipefd[1] is the write end of the pipe
    printf("Assigning file %s to slave %d, using fds: %d (read), and %d (write)\n",
        file_path, slave->pid, slave->pipefd[0], slave->pipefd[1]);
    // Send the file to the slave via the pipe
    size_t len = strlen(file_path) + 1;  // Include null terminator
    ssize_t written = write(slave->pipefd[1], file_path, len);
    if (written != len) {
        perror("write");
        return -1;
    }

    // read from the pipe
    // char buffer[1024];
    // ssize_t bytes_read = read(slave->pipefd[0], buffer, sizeof(buffer));
    // printf("%s", buffer);
    // if (bytes_read == -1) {
    //     perror("read");
    //     return -1;
    // }

    // Close the write end of the pipe after sending all file paths
    // if (close(slave->pipefd[1]) == -1) {
    //     perror("close");
    //     return -1;
    // }

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
int init_slaves(char *const argv[], int files_per_slave, slave_t **slaves) {
    int argc = 0;
    for (int i = 0; i < MAX_SLAVES; i++) {

        // Create the pipe
        if (pipe(slaves[i]->pipefd) == -1) {
            fprintf(stderr, "Error: Could not create pipe\n");
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // Create the slave
        create_slave(slaves[i]);

        //Assign initial files to slave
        for (int j = 0; j < files_per_slave && argv[argc+1] != NULL; j++) {
            // start incrementing to avoid argv[0].
            if (assign_file(slaves[i], argv[++argc]) == -1) {
                fprintf(stderr, "Error: Could not assign file to slave\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return argc-1;
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


        if (close(slave->pipefd[1]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }

        // Redirect stdin to the read end of the pipe
        if (dup2(slave->pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        if (close(slave->pipefd[0]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }

        check_program_path(SLAVE_PATH);
        execve(SLAVE_PATH, NULL, NULL);
        perror("execve");
        exit(EXIT_FAILURE);
    } else {
        // parent process
        // Save the pid and close the write end of the pipe because we are only reading
        slave->pid = pid;
        if (close(slave->pipefd[0]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
    }

    printf("Slave process with pid: %d\n", slave->pid);
    return pid;
}

static void check_program_path(char *path) {
    if (access(path, F_OK) == -1) {
        fprintf(stderr, "Error: Could not find %s\n", path);
        exit(EXIT_FAILURE);
    }
}
