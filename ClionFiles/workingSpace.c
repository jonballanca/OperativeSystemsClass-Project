#define _GNU_SOURCE
#include <stdio.h> // snprintf, asprintf, etc
#include <stdlib.h> // Per atoi
#include <string.h> // Per strlen
#include <unistd.h> // Per read i write
#include <sys/stat.h> // Per constants de modes (permisos del archiu) com S_IRUSR
#include <fcntl.h> // Per open i constants del tipo de open com O_RDONLY
#include <time.h> //Per poder agafar la informacio del temps actual
#include <signal.h> //Signals
#include <pthread.h> // Threads

#include "../config.h"
#include "../commands.h"
#include "../inventory.h"
#include "../communication.h"

char *string_read;                  // El string que guarda lo que se lee desde la terminal
char *buffer_global;                // El buffer que se utiliza para guardar lo que se escribe por pantalla
char *command;                      // String que guarda el comando
char **command_separated = NULL;    // Array de strings que guarda cada palabra del comando inicial por separado
char **command_args = NULL;         // Array de strings que guarda cada argumento por separado
int word_count = 0;                 // Guarda el número de palabras que se han escrito como comando
int argument_count = 0;             // Guarda el número de argumentos que contiene el comando
int fd_socket = 0;                  // fd del socket
int fd_client = 0;                  // fd del cliente
int fd_client_send = 0;

typedef struct {
    Maester maester;
    Inventory inventory;
} ReceptionInfo;

showHopMessage(Maester maester, destination_name, protocol)