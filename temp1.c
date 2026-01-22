#define _GNU_SOURCE
#include <stdio.h> // snprintf, asprintf, etc
#include <stdlib.h> // Per atoi
#include <string.h> // Per strlen
#include <unistd.h> // Per read i write
#include <sys/stat.h> // Per constants de modes (permisos del archiu) com S_IRUSR
#include <fcntl.h> // Per open i constants del tipo de open com O_RDONLY
#include <time.h> //Per poder agafar la informacio del temps actual
#include <signal.h> //Signals
#include <sys/wait.h>
#include <sys/types.h>
#include <math.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h> 


char* readUntil (int fd, char delimiter) {
    char car;                   // El carácter que se le
    char* buffer = NULL;        // Buffer que se le y se devuelve
    int i = 0;                  // Contador

    // Numero de bytes para leer
    int num_bytes = read(fd, &car, 1);

    if (num_bytes > 0) {
        // Minetras que no llega al delimitador
        while (car != delimiter && num_bytes != 0) {
            // Read more
            if (i == 0) {
                buffer = malloc(sizeof(char)*2);        // el character + \0     
            } else {
                buffer = realloc(buffer, sizeof(char)*(i + 2));
            }

            buffer[i] = car;
            i++;
            
            num_bytes = read(fd, &car, 1);
        }

        // Convertir en palabra
        if (i != 0) {
            buffer[i] = '\0';
        }
    }

    return buffer;
}



int getFileSizeOne (char *path) {
    int file_size = 0;
    int fd = 0;
    int end_of_file = 0;
    char *buffer;

    fd = open(path, O_RDONLY);

    if (fd < 0) {
        write(1, "Error getting file size.\n", sizeof("Error getting file size.\n"));
        return file_size;
    }

    while (!end_of_file) {
        buffer = readUntil(fd, ' ');
        if (buffer != NULL) {
            file_size += strlen(buffer) + 1;
        } else {
            end_of_file = 1;
        }
    }


    return file_size;
}

int getFileSizeTwo (char *path) {
    int file_size = 0;
    int fd = 0;
    int bytes;
    char buffer[256];

    fd = open(path, O_RDONLY);

    if (fd < 0) {
        write(1, "Error getting file size.\n", sizeof("Error getting file size.\n"));
        return file_size;
    }

    do {
        bytes = read(fd, buffer, sizeof(buffer));
        if (bytes > 0) {
            file_size += bytes;
        }
    } while (bytes > 0);

    return file_size;
}

void pathToName (char *path, char **name) {
    int pos = 0;
    int i = 0;
    char *temp;
    
    while ((path[pos] != '.') && (path[pos] != '\0')) {
        pos++;
    }
    
    *name = malloc(sizeof(char) * (pos + 1));
    
    for (i = 0; i < pos; i++) {
        (*name)[i] = path[i];
    }
    (*name)[i] = '\0';
}

int main () {
    int size_one = 0;
    int size_two = 0;

    char *name;

    size_one = getFileSizeOne("Sigils/arryn.png");
    size_two = getFileSizeTwo("Sigils/arryn.png");

    pathToName("sigil.png", &name);

    printf("Path:\t%s\n", "sigil.png");
    printf("Name:\t%s\n", name);

    return 0;
}