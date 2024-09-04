/*
** Slave Manager
**
** This program is responsible for managing the slave processes.
** It creates the slave processes and sends the files to be analyzed
** by the slaves.
**
** Author: Fernando Li                      <fli@itba.edu.ar>
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
int init_slaves(char *const argv[], uint32_t files_per_slave, slave_t **slaves, uint16_t slave_count) {
        

    if (argv == NULL || slaves == NULL) {
        fprintf(stderr, "Error: Invalid arguments to init_slaves.\n");
        return -1;
    }

    // Allocate the slaves memory
    for (int i = 0; i < slave_count; i++) {
        malloc_slave(&slaves[i]);
    }

    int argc = 0;
    for (int i = 0; i < slave_count; i++) {

        // Create the pipes
        create_pipe(slaves[i]->master2_slave_fd, "master2 slave pipe initialization");
        create_pipe(slaves[i]->slave2_master_fd, "slave2 master pipe initialization");

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

    slave->file_path = file_path;

    // TEMP: Debug
    printf("(master) Assigning file %s to slave %d, using fds: %d \
         (write to slave, master2_slave), and %d (write to master, slave2_master)\n",
        file_path, slave->pid, slave->master2_slave_fd[1], slave->slave2_master_fd[0]);

    size_t len = strlen(file_path) + 1; 
    write_pipe(slave->master2_slave_fd[1], "assign_file", file_path, len);

    return 0;
}


//select
int output_from_slaves(slave_t **slaves, uint16_t slave_count) {
    fd_set read_fds;

    FD_ZERO(&read_fds);
    int max_fd = -1;
    for (int i = 0; i < slave_count; i++) {
        if (slaves[i]->slave2_master_fd[0] < 0) {
            fprintf(stderr, "Invalid file descriptor for slave %d\n", i);
            return -1;
        }
        FD_SET(slaves[i]->slave2_master_fd[0], &read_fds);
        if (slaves[i]->slave2_master_fd[0] > max_fd) {
            // max_fd is the highest file descriptor in the set this means that 
            // select will check all the file descriptors from 0 to max_fd
            max_fd = slaves[i]->slave2_master_fd[0];
        }
    }

    // todo -> utils.h function
    int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
    if (ready == -1) {
        perror("select");
        return -1;
    } else if (ready == 0) {
        fprintf(stderr, "select timed out\n");
        return -1;
    }

    for (int i = 0; i < slave_count; i++) {
        if (FD_ISSET(slaves[i]->slave2_master_fd[0], &read_fds)) {
            char buffer[1024];
            // Read from the file descriptor and process the data
            ssize_t bytes_read = read_pipe(slaves[i]->slave2_master_fd[0], "output_from_slaves", buffer, sizeof(buffer));

            // print the output from the slave temp
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("Printing from master: %s (%s)\n", buffer, slaves[i]->file_path);
            }
        }
    }

    return 0;
}


/*
 * Create a slave process
 * @param files_path: The path to the files to process
 */
pid_t create_slave(slave_t *slave) {
    pid_t pid = fork();
    check_fork(pid, "creation of slave");
    
    // fork succeeded, child process
    if (pid == 0) {
        // Child process

        // MASTER (write) -> SLAVE (read)
        close_end_and_dup2(slave->master2_slave_fd[0], STDIN_FILENO, slave->master2_slave_fd[1], "master to slave pipe");
    
        // SLAVE (write) -> MASTER (read)
        close_end_and_dup2(slave->slave2_master_fd[1], STDOUT_FILENO, slave->slave2_master_fd[0], "slave to master pipe"); 
       
        check_program_path(SLAVE_PATH);
        char* const argv[] = {SLAVE_PATH, NULL};
        char* const envp[] = {NULL};
        execve(SLAVE_PATH, argv, envp);
        perror("execve");
        exit(EXIT_FAILURE);
    } else {
        // parent process
        // Save the pid of the slave
        slave->pid = pid;

        // Now we close the file descriptors that are not going to be used by the parent
        // fd 1 is the write end of the pipe
        close_pipe(slave->slave2_master_fd[1], "slave process");
        close_pipe(slave->master2_slave_fd[0], "slave process");
    }

    return pid;
}
