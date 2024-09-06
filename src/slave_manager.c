/*
** Slave Manager
**
** This program is responsible for managing the slave processes.
** It creates the slave processes and sends the files to be analyzed
** by the slaves.
**
** Author: Fernando Li                      <feli@itba.edu.ar>
** Author: Felipe Venturino                 <fventurino@itba.edu.ar>
** Author: Uriel Sosa Vazquez               <usosavazquez@itba.edu.ar>
**
** Last modified: 03-09-2024 (dd-mm-yyyy)
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "include/slave_manager.h"
#include "include/utils.h"

#define SLAVE_PATH              "./slave"

/*
 * Calculate the number of files per slave
 */

static void malloc_slave(slave_t **slave) {
    *slave = (slave_t *) malloc(sizeof(slave_t));
    if (slave == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for slave\n");
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

void free_slave(slave_t *slave) {
    if (slave == NULL) {
        return;
    }
    // If the slave is still running, wait for it to finish
    waitpid(slave->pid, NULL, 0);
    free(slave);
}

/*
 * Initialize the slave processes
 * @param files_path: The path to the files to process
 * @param files_count: The number of files to process per slave
 * @param slaves: The slave processes
 *
 * @return The number of files assigned total
 */
int init_slaves(char *const argv[], uint32_t files_per_slave, slave_t **slaves, uint16_t max_slaves) {
        

    if (argv == NULL || slaves == NULL) {
        fprintf(stderr, "Error: Invalid arguments to init_slaves.\n");
        return -1;
    }

    // Allocate the slaves memory
    for (int i = 0; i < max_slaves; i++) {
        malloc_slave(&slaves[i]);
    }

    int argc = 0;
    for (int i = 0; i < max_slaves; i++) {

        // Create the pipe
        create_pipe(slaves[i]->pipefd, "slave initialization");

        // Create the slave
        create_slave(slaves[i]);

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
    printf("(master) Assigning file %s to slave %d, using fds: %d (read), and %d (write)\n",
        file_path, slave->pid, slave->pipefd[0], slave->pipefd[1]);
    // Send the file to the slave via the pipe
    size_t len = strlen(file_path) + 1;  // Include null terminator
    write_pipe(slave->pipefd[1], "assign_file", file_path, len);

    return 0;
}

/*
 * Create a slave process
 * @param files_path: The path to the files to process
 */
pid_t create_slave(slave_t *slave) {
    pid_t pid = fork();
    char * path[] = {SLAVE_PATH,NULL};
    check_fork(pid, "creation of slave");

    // fork succeeded, child process
    if (pid == 0) {
        
        close_pipe(slave->pipefd[1], "slave process");

        // Redirect stdin to the read end of the pipe
        dup2_pipe(slave->pipefd[0], STDIN_FILENO, "slave process");

        close_pipe(slave->pipefd[0], "slave process");

        check_program_path(SLAVE_PATH);
        execve(SLAVE_PATH, path, NULL);
        perror("execve");
        exit(EXIT_FAILURE);
    } else {
        // parent process
        // Save the pid and close the write end of the pipe because we are only reading
        slave->pid = pid;
        close_pipe(slave->pipefd[0], "slave process");
    }

    return pid;
}
