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
#define REQUIRED_ARGS       1


int assignSlaves(char *argv[], int filesPerSlave);
int initSlaves(char *argv[], int filesPerSlave);
void createSlave(char *files[]);
int filesPerSlave(int files_count);

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


    int files_count = filesPerSlave(argc - 1);
    assignSlaves(argv, files_count);
    
    
    
    return 0;
}

int assignSlaves(char *argv[], int files_count) {
    struct dirent *entry; // Contains the name of the file
    struct stat file_stat; // Contains the file information


    initSlaves(argv, filesPerSlave);
}

int initSlaves(char *argv[], int filesPerSlave) {
    int files_assigned = 0;
    for (int i = 0; i < MAX_SLAVES; i++) {
        char* files[filesPerSlave];
        for (int j = 0; j < filesPerSlave; j++) {
            files[j] = argv[files_assigned + j];
        }
        createSlave(files);
        files_assigned += filesPerSlave;
    }
}

void createSlave(char *files[]){
    pid_t pid = fork();
    if (pid < 0){
        fprintf(stderr, "Error: Could not create slave\n");
        return 1;
    }
    if (pid == 0){
        execve("./slave", files, NULL); //ejecuto el esclavo
    }
}

int filesPerSlave(int files_count) {
    return (files_count + MAX_SLAVES - 1) / MAX_SLAVES;
}