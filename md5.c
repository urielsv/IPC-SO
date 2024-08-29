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

#define MAX_FILES           100
#define MAX_SLAVES          3
#define REQUIRED_ARGS       2
#define SLAVE_PATH          "./slave"

/*
 * Calculate the number of files per slave
 */
#define FILES_PER_SLAVE(files) ((files + MAX_SLAVES - 1) / MAX_SLAVES)

int assign_slaves(char *argv[], int filesPerSlave);
int init_slaves(char *argv[], int filesPerSlave);
void create_slave(char *files[]);
static void check_program_path(char *path);

int main(int argc, char* argv[]) {
    if (argc < REQUIRED_ARGS) {
        printf("Usage: %s <files> ...", argv[0]);
        return 1;
    }

    DIR* dir;
    if (!(dir = opendir(argv[1]))) {
        fprintf(stderr, "Error: Could not open directory\n");
        return 1;
    }


    int files_count = FILES_PER_SLAVE(argc - 1);
    assign_slaves(argv, files_count);

    return 0;
}

int assign_slaves(char *argv[], int files_count) {
    init_slaves(argv, files_count);
    return 0;
}

/*
 * Initialize the slave processes
 */
int init_slaves(char *argv[], int files_count) {
    int files_assigned = 0;
    for (int i = 0; i < MAX_SLAVES; i++) {
        char* files[files_count];
        for (int j = 0; j < files_count; j++) {
            files[j] = argv[files_assigned + j];
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
        char *const files[] = { (char *)files_path, NULL };
        char *const envp[] = { NULL };

        check_program_path(SLAVE_PATH);
        execve(SLAVE_PATH, files, envp);
    }
}

static void check_program_path(char *path) {
    if (access(path, F_OK) == -1) {
        fprintf(stderr, "Error: Could not find %s\n", path);
        exit(EXIT_FAILURE);
    }
}
