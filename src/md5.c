// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "include/md5.h"
#include "include/defs.h"
 int main(int argc, char *const argv[]) {
    // Validate arguments
    if (argc < REQUIRED_ARGS) {
        printf("Usage: %s <file1> <file2> <...> <fileN>\n", argv[0]);
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
    char pid[PID_MAX_SIZE] = {0};
    snprintf(pid, sizeof(pid), "%d", getpid());


    shared_memory_adt shared_memory = create_shared_memory(pid, SHM_BUFFER_SIZE, INITIAL_SEM_MUTEX);

    // We pass the shm, and sem to the view process
    printf("%s\n", get_shm_path(shared_memory));
    printf("%s\n", get_buff_sem_path(shared_memory));
    printf("%s\n", get_mutex_sem_path(shared_memory));
    fflush(stdout);

     // Give time for the user to init the view before starting the slaves
     sleep(VIEW_SLEEP_TIME);

    int files = argc - 1;

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

    int output_file_fd;
    if ( (output_file_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644) ) == -1) {
         perror("ERROR: error when opening file.txt");
        return 1;
    }

    // Now we start processing the remaining files
     while (get_processed_files(shared_memory) < files) {

         // Check if any slave is available
        for (int i = 0; i < assigned_slaves; i++) {
            if (slaves[i]->is_available && files_assigned < files) {
                assign_file(slaves[i], argv[++files_assigned]);
            }
        }
        output_from_slaves(slaves, assigned_slaves, shared_memory);
     }

    // Now we write to the shm an empty string to signal the view process that we are done
    write_shared_memory(shared_memory, "", "", 0);
    // Clean up resources
    destroy_resources(shared_memory);
    finish_slaves(slaves, assigned_slaves);

    if (close(output_file_fd) != 0) {
        perror("Error closing file");
        return 1;
    }

     return 0;
 }

int output_from_slaves(slave_t **slaves, uint16_t slave_count, shared_memory_adt shared_memory) {
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
            char buffer[BUFF_SIZE] = {0};

            ssize_t bytes_read = read_pipe(slaves[i]->slave2_master_fd[0], "output_from_slaves", buffer, sizeof(buffer));

            if (bytes_read > 0) {
                slaves[i]->is_available = 1;

                // Add a null terminator to the buffer so we can avoid printing garbage
                buffer[bytes_read] = '\0';

                write_shared_memory(shared_memory, slaves[i]->file_path, buffer, slaves[i]->pid);

            } 
        }
    }

    return 0;
}

uint32_t initial_files_per_slave(uint32_t files, uint32_t slave_count) {
    if (files < MIN_FILES) {
        return 1;
    }
    if (slave_count == 0) {
        return 0; // Prevent division by zero
    }
    uint32_t initial_files = (uint32_t)(files * 0.1f);
    uint32_t files_per_slave = initial_files / slave_count;
    return files_per_slave > 0 ? files_per_slave : 1; // Ensure at least 1 file per slave
}


uint32_t slave_count(uint32_t files) {
    if (files < MIN_FILES) {
        return files > DEFAULT_SLAVE_COUNT ? DEFAULT_SLAVE_COUNT : files;
    }
    uint32_t count = (uint32_t)(files * 0.05f);
    return count > 0 ? count : DEFAULT_SLAVE_COUNT; 
}