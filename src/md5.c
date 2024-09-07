#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "include/slave_manager.h"
#include "include/utils.h"
#include "include/shm_utils.h"
#include "include/md5.h"

#define REQUIRED_ARGS           2
#define MIN_FILES               20
#define DEFAULT_SLAVE_COUNT     3
#define MD5_HASH                32
#define INITIAL_SEM_MUTEX       1
uint32_t initial_files_per_slave(uint32_t files, uint32_t slave_count) {
    if(files < MIN_FILES) {
        return 1;
    }

    uint32_t initial_files = (uint32_t) files * 0.1f;
    return (uint32_t) initial_files / slave_count;
}

uint32_t slave_count(uint32_t files) {
    if (files < MIN_FILES) {
        return files > DEFAULT_SLAVE_COUNT ? DEFAULT_SLAVE_COUNT : files;
    } 
    return (uint32_t) files * 0.05f;
}



int main(int argc, char *const argv[]) {
    // Validate arguments
    if (argc < REQUIRED_ARGS) {
        printf("Usage: %s <file1> <file2> <...> <fileN>", argv[0]);
        return 1;
    }

    // Validate paths
    for (int i = 1; i < argc; i++) {
        struct stat path_stat;
        if (stat(argv[i], &path_stat) != 0) {
            fprintf(stderr, "Error: Could not find %s\n", argv[i]);
            return 1;
        }
    }

    // Give time for the user to init the view before starting the slaves
    sleep(2);
    int files = argc - 1;
    // Shared memory buffer (we will store the md5 hashes here)
    shared_memory_t *shared_memory = create_shared_memory("/md5_sem", INITIAL_SEM_MUTEX);

    

    // Initialize slaves
    int assigned_slaves = slave_count(files);
    slave_t *slaves[assigned_slaves];
    int init_files_per_slave = initial_files_per_slave(files, assigned_slaves);

    int files_assigned = 0;
    files_assigned = init_slaves(argv, init_files_per_slave, slaves, assigned_slaves);
    if (files_assigned == -1) {
        fprintf(stderr, "Error: Could not initialize slaves\n");
        return 1;
    }

    // /*TODO Test */
    // int fd = open("resultado.txt",O_RDWR);
    // if (fd==-1){
    //     perror("ERROR: error when opening file resultado.txt");
    //     exit(EXIT_FAILURE);
    // }
    // dup2(fd,0);


    // Now we start processing the remaining files 
    while (shared_memory->files_processed < files) {

        // Check if any slave is available
        for (int i = 0; i < assigned_slaves; i++) {
            if (slaves[i]->is_available && files_assigned < files) {
                assign_file(slaves[i], argv[++files_assigned]);
            }
        }
        output_from_slaves(slaves, assigned_slaves, shared_memory);
    }

    // Print the shared memory buffer
    for (int i = 0; i < shared_memory->files_processed; i++) {
        printf("MD5 hash: %s\n", shared_memory->buffer[i].file_info->md5);
        printf("File path: %s\n", shared_memory->buffer[i].file_info->file_path);
        printf("Slave id: %d\n", shared_memory->buffer[i].file_info->slave_id);
    }
    
    finish_slaves(slaves, assigned_slaves);
    return 0;
}

int output_from_slaves(slave_t **slaves, uint16_t slave_count, shared_memory_t *shared_memory) {
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
            // Clean the buffer
            char buffer[1024] = {0};

            // Read from the file descriptor and process the data
            ssize_t bytes_read = read_pipe(slaves[i]->slave2_master_fd[0], "output_from_slaves", buffer, sizeof(buffer));

            // print the output from the slave temp
            if (bytes_read > 0) {
                slaves[i]->is_available = 1;
                shared_memory->files_processed++;

                // Now we store the md5 hash in the shared memory

                // temp, change to parameters instead 
                struct file_info_t fi;
                // fi.slave_id = slaves[i]->pid;
                // strncpy(fi.slave_id, "333", 30);
                // strncpy(fi.md5, buffer, MD5_HASH);
                // strncpy(fi.file_path, slaves[i]->file_path, strlen(slaves[i]->file_path));


                write_shared_memory(shared_memory, fi);

                buffer[bytes_read] = '\0';
                printf("Printing from master: %s (%s)\n", buffer, slaves[i]->file_path);
            }
        }
    }

    return 0;
}
