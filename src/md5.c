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

#define MAX_SLAVES              10
#define REQUIRED_ARGS           2

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


    int assigned_slaves = MAX_SLAVES;
    slave_t *slaves[assigned_slaves];

    int files_assigned;
    files_assigned = init_slaves(argv, 2, slaves, assigned_slaves);

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
