// 1. Recibir input los archivos (./md5 files/*)
// No hay falta expandir el *
// podemos usar pipes por ejemplo ./md5 files/* | ./view

// 2. Iniciar procesos esclavos

// 3. Distribuir archivos entre distintos esclavlos

// 4. Si un slave se libera se le asigna otro archivo a procesar

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

#include "include/slave_manager.h"

#define REQUIRED_ARGS           2
#define MIN_PERCENT_FILES       20
#define DEFAULT_SLAVE_COUNT     3
uint32_t initial_files_per_slave(uint32_t files, uint32_t slave_count) {
    if(files < MIN_PERCENT_FILES) {
        return 1;
    }

    uint32_t initial_files = (uint32_t) files * 10 / 100;
    return (uint32_t) initial_files / slave_count;
}

uint32_t slave_count(uint32_t files) {
    if (files < MIN_PERCENT_FILES) {
        return files > DEFAULT_SLAVE_COUNT ? DEFAULT_SLAVE_COUNT : files;
    } 
    return (uint32_t) files * 5 / 100;
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

    // Initialize slaves
    int files = argc - 1;
    int assigned_slaves = slave_count(files);
    slave_t *slaves[assigned_slaves];
    int init_files_per_slave = initial_files_per_slave(files, assigned_slaves);

    int files_assigned = 0;
    files_assigned = init_slaves(argv, init_files_per_slave, slaves, assigned_slaves);
    if (files_assigned == -1) {
        fprintf(stderr, "Error: Could not initialize slaves\n");
        return 1;
    }


    while (1) {
        output_from_slaves(slaves, assigned_slaves);
    }

    /*
    Select and logic to keep processing files until finished
    while (1) { assign files to available slave }
    */


    // free slaves
    for (int i = 0; i < assigned_slaves; i++) {
        free_slave(slaves[i]);
    }

    return 0;
}
