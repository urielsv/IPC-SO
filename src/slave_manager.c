// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*

** Author: Fernando Li                      <feli@itba.edu.ar>
** Author: Felipe Venturino                 <fventurino@itba.edu.ar>
** Author: Uriel Sosa Vazquez               <usosavazquez@itba.edu.ar>

*/

#include "include/slave_manager.h"


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


    int initial_idx = 1;
    for (int i = 0; i < slave_count; i++) {
        // Allocate memory for the slave 
        malloc_slave(&slaves[i]);

        // Create the pipes
        create_pipe(slaves[i]->master2_slave_fd, "master2 slave pipe initialization");
        create_pipe(slaves[i]->slave2_master_fd, "slave2 master pipe initialization");

        char *files[files_per_slave];
        for (int j = 0; j < files_per_slave && argv[initial_idx] != NULL; j++) {
            fprintf(stderr, "File %d: %s, slave: %d\n", j, argv[initial_idx], i);
            files[j] = argv[initial_idx++];
        }
        create_slave(slaves[i], files, files_per_slave);
    }
    
    return files_per_slave * slave_count;
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

    slave->is_available = 0;
    slave->file_path = file_path;
 
    // Create a new string with a newline at the end of the file path
    size_t len = strlen(file_path);
    char *new_file_path = (char *) malloc(len + 1);
    if (new_file_path == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for new file path\n");
        perror("malloc");
        return -1;
    }
    strncpy(new_file_path, file_path, len);
    new_file_path[len] = '\n';
    write_pipe(slave->master2_slave_fd[1], "assign_file", new_file_path, len+1);
    free(new_file_path);
    return 0;
}






void debug_slave(slave_t *slave) {
    printf("--- Slave Debug ---\n");
    printf("Slave %d\n", slave->pid);
    printf("File path: %s\n", slave->file_path);
    printf("Slave2Master: %d %d\n", slave->slave2_master_fd[0], slave->slave2_master_fd[1]);
    printf("Master2Slave: %d %d\n", slave->master2_slave_fd[0], slave->master2_slave_fd[1]);
    printf("Is available: %d\n", slave->is_available);
    printf("-------------------\n");
}

void finish_slaves(slave_t **slaves, uint16_t slave_count) {
    for (int i = 0; i < slave_count; i++) {
        // close pipes, so we can finish the program
        close_pipe(slaves[i]->master2_slave_fd[1], "master to slave pipe");
        close_pipe(slaves[i]->slave2_master_fd[0], "slave to master pipe");
        
    }

     // free slaves memory
    for(int i =0; i < slave_count; i++)
        free_slave(slaves[i]);
}

/*
 * Create a slave process
 * @param files_path: The path to the files to process
 */
pid_t create_slave(slave_t *slave, char *const files_path[], uint32_t files_count) {
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
        
        // Assign initial files 
        for (int i = 0; i < files_count; i++) {
            fprintf(stderr, "Assigning file %s to slave\n", files_path[i]);
            if (assign_file(slave, files_path[i]) == -1) {
                fprintf(stderr, "Error: Could not assign file to slave\n");
                exit(EXIT_FAILURE);
             }
         }
    

        // Now we close the file descriptors that are not going to be used by the parent
        // fd 1 is the write end of the pipe
        close_pipe(slave->slave2_master_fd[1], "slave process");
        close_pipe(slave->master2_slave_fd[0], "slave process");
    }

    return pid;
}
