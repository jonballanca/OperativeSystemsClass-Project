/***********************************************
*
* @Proposit: Controlar la logica del maester, que consiste en recibir y hacer la logica para cada uno de los comandos
* @Autor/s: Jon Ballanca y Alejandro Cortes
* @Data creacio: 20 / 10 / 2025
* @Data ultima modificacio: 26 / 10 / 2025
*
************************************************/

#define _GNU_SOURCE
#include <stdio.h> // snprintf, asprintf, etc
#include <stdlib.h> // Per atoi
#include <string.h> // Per strlen
#include <unistd.h> // Per read i write
#include <sys/stat.h> // Per constants de modes (permisos del archiu) com S_IRUSR
#include <fcntl.h> // Per open i constants del tipo de open com O_RDONLY
#include <signal.h> //Signals
#include <pthread.h> // Threads

#include "config.h"
#include "commands.h"
#include "inventory.h"
#include "communication.h"

#define FLECHA "\u2192"

char *string_read;                  // El string que guarda lo que se lee desde la terminal
char *buffer_global;                // El buffer que se utiliza para guardar lo que se escribe por pantalla
char *buffer_threads;				// El buffer que utiliza el buffer
char *command;                      // String que guarda el comando
char **command_separated = NULL;    // Array de strings que guarda cada palabra del comando inicial por separado
char **command_args = NULL;         // Array de strings que guarda cada argumento por separado
int word_count = 0;                 // Guarda el número de palabras que se han escrito como comando
int argument_count = 0;             // Guarda el número de argumentos que contiene el comando
int fd_socket = 0;                  // fd del socket
int fd_client = 0;                  // fd del cliente
int fd_client_send = 0;
pthread_mutex_t terminal_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int timer_flag;
Maester maester;

typedef struct {
    Inventory inventory;

    char* list_products_rl;         // El nombre del realm cuya lista de productos queremos
    char* inventory_file_name;

    int pledge_flag;
    int pledge_pending;
    char *pledge_response;
    char *pledge_destination;
    struct in_addr pledge_ip;
    int pledge_port;
} ReceptionInfo;




void showInitializedMaester () {
    int printing = 0;                   // Variable que guarda los bytes para escribir por la terminal

    printing = asprintf(&buffer_global, "%s initialized. The board is set.\n", maester.realm_name);
    write(1, buffer_global, printing);
    free(buffer_global);
    buffer_global = NULL;
}

void initializeReception (ReceptionInfo *reception, Inventory inventory, char *argument) {
    reception->inventory = inventory;
    reception->inventory_file_name = malloc(sizeof(char) * strlen(argument));
    reception->list_products_rl = NULL;
    reception->pledge_flag = 0;
    reception->pledge_pending = 0;
    strcpy(reception->inventory_file_name, argument);
}

void getCommandInput () {
    int printing = 0;                   // Variable que guarda los bytes para escribir por la terminal
    
    showText(BOLD "$ ");

    // Lee el del usuario desde la terminal
    string_read = readUntil(0, '\n');
    printing = asprintf(&buffer_global, "%s", RESET);
    write(1, buffer_global, printing);
    free(buffer_global);
    buffer_global = NULL;
}

int manageCommand () {
    int command_index = 0;

    word_count = 0;

    getCommandInput(&string_read);

    // Se reinicia el array
    command_separated = NULL;

    // Si se ha leido algo
    if (string_read != NULL) {

        // Si lo que ha leido no ha sido todo espacios
        word_count = separateCommand(&command_separated, string_read);
        if (command_separated != NULL) {
            
            // Se separa el string leido en comando y argumentos
            argument_count = separateArguments(&command, &command_args, command_separated, word_count);

            // Se guarda el index del comando
            command_index = getCommandIndex(command, command_args, argument_count, maester);
        } else {
            emptyStringError();
        }
    } else {
        emptyStringError();
    }

    return command_index;
}


int switchOfCommands(ReceptionInfo *reception, Inventory inventory, int command_index) {
    int exit_flag = 0;

    switch (command_index) {
        case INVALID_COMMAND:
            // Hacer nada; No hacer ningun commando. Los errores ya se han controlado
            break;
        case LIST_REALMS:
            listRealms();
            break;
        case PLEDGE:
            reception->pledge_destination = malloc(sizeof(char) * strlen(command_args[FIRST_ARGUMENT]));
            strcpy(reception->pledge_destination, command_args[FIRST_ARGUMENT]);
            pledge(command_args[FIRST_ARGUMENT], command_args[SECOND_ARGUMENT], &reception);
            break;
        case PLEDGE_RESPOND:
            if (reception->pledge_pending) {
                pledgeRespond(command_args[FIRST_ARGUMENT], command_args[SECOND_ARGUMENT]);
                reception->pledge_pending = 0;
            } else {
                // No me han enviado ningun pledge
            }
            break;
        case LIST_PRODUCTS:
            // Implementado
            reception->list_products_rl = realloc(reception->list_products_rl, sizeof(char) * strlen(command_args[FIRST_ARGUMENT]));
            strcpy(reception->list_products_rl, command_args[FIRST_ARGUMENT]);
            listProductsMaester(command_args[FIRST_ARGUMENT]);
            break;
        case LIST_PRODUCTS_NO_ARGS:
            // Implementado
            listProductsNoArgs(inventory);
            break;
        case START_TRADE:
            startTrade(command_args[FIRST_ARGUMENT], inventory);
            break;
        case PLEDGE_STATUS:
            pledgeStatus();
            break;
        case ENVOY_STATUS:
            envoyStatus();
            break;
        case EXIT:
            exitMaester(&inventory);
            exit_flag = 1;

            break;
        default:
            break;
    }

    return exit_flag;
}




void main (int argc, char **argv) {
    Inventory inventory;                // El inventario del maester
    ReceptionInfo reception;

    int error_config_file = 0;          // Flag que guarda si hay un error en el fichero de configuración
    int error_inventory_file = 0;       // Flag que guarda si hay un error en el fichero del inventario
    int file_errors = 0;                // Flag que guarda si hay un error en cualquier o los dos ficheros
    int printing = 0;                   // Variable que guarda los bytes para escribir por la terminal
    int exit_flag = 0;                  // Flag que guarda si se tiene que salir de estar esperando a los comandos
    int command_index = 0;              // Guarda el comando como el index que representa baseado en el commands.h

    pthread_t reception_thread;
    
    signal(SIGINT, unexpectedExit);
    signal(SIGALRM, timerFlag);

    // Si se han entrado menos que 3 argumentos
    if (argc < 3) {
        showText("\nToo few arguments to initialize the maester.\n");
        exit(1);
    }

    error_config_file = readConfigFile(&maester, argv[1]);
    error_inventory_file = readInventoryFile(&inventory, argv[2]);

    file_errors = checkFileErrors(error_config_file, error_inventory_file);

    // Si no hay ningún error en ningún fichero
    if (file_errors) {
        exit(1);
    }

    addActive();
    showInitializedMaester(maester);
    initializeReception(&reception, inventory, argv[2]);

    pthread_create(&reception_thread, NULL, receiveClients, &reception);

    exit_flag = 0;
    timer_flag = 0;
    

    // Recorre el bucle de comandos mientras que el usuario no escribe "EXIT"
    do {
        command_index = manageCommand();

        // A partir de aqui, solo trabajamos con command y command_args
        // Se separa cada caso de cada comando en el switch
        exit_flag = switchOfCommands(&reception, inventory, command_index);

        // Liberar memoria de las variables que reservan su memoria
        freeBuffers();
        
    } while(!exit_flag); //Tiene que hacerlo hasta que sea exit

    return 0;
}