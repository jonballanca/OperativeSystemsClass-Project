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
#include "protocols.h"

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


void freeBuffers () {
    if (string_read != NULL) {
        free(string_read);
        string_read = NULL;
    }

    if (buffer_global != NULL) {
        free(buffer_global);
        buffer_global = NULL;
    }

    if (buffer_threads != NULL) {
        free(buffer_threads);
        buffer_threads = NULL;
    }

    if (command != NULL) {
        free(command);
        command = NULL;
    }

    if (command_separated != NULL) {
        for (int i = 0; i < word_count; i++) {
            free(command_separated[i]);
        }
        free(command_separated);
        command_separated = NULL;
    }

    if (command_args != NULL) {
        for (int i = 0; i < argument_count; i++) {
            free(command_args[i]);
        }
        free(command_args);
        command_args = NULL;
    }

    word_count = 0;
    argument_count = 0;
}

/***********************************************
*
* @Finalitat: Harcodear información para las pruebas de la fase 1. No se utiliza esta función al final.
* @Parametres: ---
* @Retorn: Inventario con la información hardcodeada para las pruebas de la fase1
*
************************************************/
/*Inventory requestAvailableProducts () {
    Inventory inventory;            // Inventario que se hardcodea para la Fase 1, la que es el trade
                                    // No se utiliza esta función al final.
    */
    /* Hardcode de dos productos para la Fase 1 */
    /*
    inventory.product_count = 2;

    inventory.products = (Product*)malloc(sizeof(Product) * inventory.product_count);

    strcpy(inventory.products[0].name, "Myrish Lace");
    strcpy(inventory.products[1].name, "Sweetwine");

    inventory.products[0].amount = 150;
    inventory.products[1].amount = 80;

    inventory.products[0].weight = 4.2;
    inventory.products[1].weight = 9.5;

    return inventory;
}*/

/***********************************************
*
* @Finalitat: Añade la información de los productos en el invetario trade, y los resta en el avail.
* @Parametres:  in/out: avail_inventory = inventario con el que se desea tradear
                in/out: trade = inventario que indica que se desea tradear
                in: arguments = nombre del producto que se desea tradear y la cantidad
* @Retorn: ---
*
************************************************/
void addToTrade (Inventory* avail_inventory, Inventory* trade, char** arguments) {
    int i = 0;              // Contador
    char * aux_name;        // Buffer que guarda un string temporalmente para poder convertirlo a mayúsculas

    for (i = 0; i < (*avail_inventory).product_count; i++) {
        aux_name = (char*)malloc(sizeof(char) * (strlen((*avail_inventory).products[i].name) + 1));
        strcpy(aux_name, (*avail_inventory).products[i].name);
        convertToCapitalLetters(aux_name);

        if (!strcmp(arguments[FIRST_ARGUMENT], aux_name)) {
            (*trade).products[i].amount += atoi(arguments[SECOND_ARGUMENT]);
            (*avail_inventory).products[i].amount -= atoi(arguments[SECOND_ARGUMENT]);
        }

        free(aux_name);
    }
}

/***********************************************
*
* @Finalitat: Resta la información de los productos en el invetario trade, y los añade en el avail.
* @Parametres:  in/out: avail_inventory = inventario con el que se desea tradear
                in/out: trade = inventario que indica que se desea tradear
                in: arguments = nombre del producto que se desea tradear y la cantidad
* @Retorn: ---
*
************************************************/
void removeFromTrade (Inventory* avail_inventory, Inventory* trade, char** arguments) {
    int i = 0;              // Contador
    char * aux_name;        // Buffer que guarda un string temporalmente para poder convertirlo a mayúsculas

    for (i = 0; i < (*avail_inventory).product_count; i++) {
        aux_name = (char*)malloc(sizeof(char) * (strlen((*avail_inventory).products[i].name) + 1));
        strcpy(aux_name, (*avail_inventory).products[i].name);
        convertToCapitalLetters(aux_name);
        
        if (!strcmp(arguments[FIRST_ARGUMENT], aux_name)) {
            (*trade).products[i].amount -= atoi(arguments[SECOND_ARGUMENT]);
            (*avail_inventory).products[i].amount += atoi(arguments[SECOND_ARGUMENT]);
        }

        free(aux_name);
    }
}

/***********************************************
*
* @Finalitat: Muestra el mensaje de que se ha enviado un trade
* @Parametres:  in: trade = productos y cantidad que se desean tradear
                in: realm = nombre del reino con el que se desea tradear
* @Retorn: ---
*
************************************************/
void sendTrade (Inventory trade, char* realm, char* file_path, ReceptionInfo **reception) {
    Protocol protocol;

    (*reception)->sending_file = SENDING_TRADE;
    (*reception)->trade_file_path = realloc((*reception)->trade_file_path, sizeof(char) * (strlen(file_path) + 1));
    strcpy((*reception)->trade_file_path, file_path);

    protocol = commOrderRequest(maester, realm, file_path);
    sendToRoute(realm, protocol, maester);
    printReset("\n[DEBUG TRADE] Trade successfully sent!\n\n");

    /* Hardcode de Send Trade para la Fase 1 */

    /*if (maester.num_envoys == 3) {
        // Solo para que no salga warning
    }

    trade.product_count = trade.product_count + 2;
    realm = NULL;
    
    int printing;           // Entero que guarda el número de los bytes para escribir
    char* buffer;           // Buffer que guarda lo que se quiere escribir por la terminal

    printing = asprintf(&buffer, "Trade list sent to %s.\n", realm);
    write(1, buffer, printing);
    free(buffer);*/
}

/***********************************************
*
* @Finalitat: Liberar la información de los productos auxiliars
* @Parametres:  in/out: inventory = información que se desea liberar
                in/out: trade = información que se desea liberar
* @Retorn: ---
*
************************************************/
void freeProducts(Inventory *inventory, Inventory *trade) {
    if (inventory != NULL) {
        free(inventory->products);
        inventory->products = NULL; 
        inventory->product_count = 0;
    }

    if (trade != NULL) {
        free(trade->products);
        trade->products = NULL;
        trade->product_count = 0;
    }
}

/***********************************************
*
* @Finalitat: Ensenyar un mensaje para aqullos commandos no implementados que esten escritos correctamente
* @Parametres:  ---
* @Retorn:  ----
*
************************************************/
void notImplementedYet(){
    showText("Command OK\n");
}

/***********************************************
*
* @Finalitat: Listar todos los reinos que conoce este maester
* @Parametres:  in: maester = contiene toda la informacion del maester, entre otros los nombres de los reinos conocidos 
* @Retorn:  ---
*
************************************************/
void listRealms() {
    int i = 0;              // Contador
    int num_bytes = 0;      // Entero que guarda el número de los bytes para escribir   
    char* buffer;           // Buffer que guarda lo que se quiere escribir por la terminal

    for (i = 0; i < maester.num_routes; i++) {
        if (strcmp(maester.routes[i].realm_name, "DEFAULT")) {
            num_bytes = asprintf(&buffer, "- %s\n", maester.routes[i].realm_name);
            write(1, buffer, num_bytes);
            free(buffer);
        }
    }

    showText("\n");
}

/***********************************************
*
* @Finalitat: Hacer un pledge (No implementado)
* @Parametres:  ---
* @Retorn:  ---
*
************************************************/
void pledge (char *realm_name, char *sigil_path, ReceptionInfo *reception) {
    Protocol protocol;
    int error = 0;

    changeTimedOut(realm_name, RESET_TIMED_OUT, &maester);

    protocol = comAllianceRequest(maester, realm_name, sigil_path);

    error = sendToRoute(realm_name, protocol, maester);

    if (error != -1) {
        reception->sending_file = SENDING_SIGIL;
        asprintf(&buffer_global, RESET "[DEBUG PLEDGE] Pledge sent to %s\n%s", realm_name, BOLD);
        write(1, buffer_global, strlen(buffer_global));
        free(buffer_global);
        buffer_global = NULL;

        reception->pledge_flag = 1;

        asprintf(&buffer_global, RESET "[DEBUG PLEDGE] Waiting for response...\n%s", BOLD);
        write(1, buffer_global, strlen(buffer_global));
        free(buffer_global);
        buffer_global = NULL;

        timer_flag = 1;
        alarm(120);
        //alarm(2);

        while ((reception->pledge_flag == 1) && (timer_flag == 1));
        
        if (timer_flag == 0) {
            reception->sending_file = 0;
            changeTimedOut(realm_name, TIMED_OUT, &maester);

            printReset("\n[DEBUG PLEDGE] Waited for 2 minutes. Received no response.\n");
        }

        timer_flag = 1;
    } else {
        printReset("\n[DEBUG PLEDGE] Error connecting to socket.\n");
    }
}

/***********************************************
*
* @Finalitat: Responder a un pledge (No implementado)
* @Parametres: ---
* @Retorn:  ---
*
************************************************/
void pledgeRespond(char* realm_name, char* decision, struct in_addr ip, int port, ReceptionInfo *reception){
    Protocol protocol;
    int error = 0;

    protocol = commAllianceResponse (maester, realm_name, decision);

    error = sendToRoute(realm_name, protocol, maester);

    if (error != -1) {
        reception->response_confirmation = 1;
        reception->pledge_decision = realloc(reception->pledge_decision, sizeof(char) * (strlen(decision) + 1));
        strcpy(reception->pledge_decision, decision);
        /*if (!strcmp(decision, "ACCEPT")) {
            asprintf(&buffer_global, RESET "Alliance with %s established.\n%s", realm_name, BOLD);
            write(1, buffer_global, strlen(buffer_global));
            free(buffer_global);
            buffer_global = NULL;
            addAlliance(realm_name, &maester, ip, port);
            addPledgeStatus(reception, ACCEPTED, realm_name);
        } else if (!strcmp(decision, "REJECT")) {
            asprintf(&buffer_global, RESET "Alliance with %s was rejected.\n%s", realm_name, BOLD);
            write(1, buffer_global, strlen(buffer_global));
            free(buffer_global);
            buffer_global = NULL;
            addPledgeStatus(reception, REJECTED, realm_name);
        }*/
    } else {
        asprintf(&buffer_global, RESET "Error connecting to socket.\n%s", BOLD);
        write(1, buffer_global, strlen(buffer_global));
        free(buffer_global);
        buffer_global = NULL;
    }
}

/***********************************************
*
* @Finalitat: Listar los productos de un reino que no sea el propio (No implementado)
* @Parametres:  ---
* @Retorn:  ---
*
************************************************/
void listProductsMaester (char *realm_name) {
    Protocol protocol;
    int error = 0;

    protocol = commProductsRequest(maester, realm_name);

    error = sendToRoute(realm_name, protocol, maester);

    if (error != -1) {
        showText("[DEBUG LIST PRODUCTS] Header sent!\n");
    } else {
        asprintf(&buffer_global, RESET "Error connecting to socket.\n%s", BOLD);
        write(1, buffer_global, strlen(buffer_global));
        free(buffer_global);
        buffer_global = NULL;
    }
}

/***********************************************
*
* @Finalitat: Listar los productos del propio reino
* @Parametres:  in: inventory = inventorio que se desea listar
* @Retorn: ---
*
************************************************/
void listProductsNoArgs (Inventory inventory) {
    listProducts(inventory);
}

/***********************************************
*
* @Finalitat: Hacer toda la lógica del submenu startTrade, que consiste en ir recibiendo subcomandos e ir 
                añadiendo o removiendo productos o enviar o cancelar la operación de comercio 
* @Parametres:  in: realm = nombre del reino con el que se desea intercambiar
                in: maester = información del reino
                in: avail_inventory = productos con los que se puede comerciar
* @Retorn:  ----
*
************************************************/
void startTrade (char *realm, Inventory avail_inventory, ReceptionInfo *reception) {
    //char *buffer;                       // El buffer que se utiliza para guardar lo que se escribe por pantalla
    //char *string_read;                  // El string que guarda lo que se lee desde la terminal
    //char **command_separated = NULL;    // Array de strings que guarda cada palabra del comando inicial por separado
    //char *command;                      // String que guarda el comando
    //char **command_args = NULL;         // Array de strings que guarda cada argumento de "trade" por separado
    int printing = 0;                   // Variable que guarda los bytes para escribir por la terminal
    //int word_count = 0;                 // Guarda el número de palabras que se han escrito como comando de "trade"
    int exit_flag = 0;                  // Flag que guarda si se tiene que salir de estar esperando a los comandos de "trade"
    //int argument_count = 0;             // Guarda el número de argumentos que contiene el comando de "trade"
    int command_index;                  // El entero que guarda el index que representa el comando de "trade"
    Inventory trade;                    // El inventario auxiliar para hacer el trade
    Inventory avail_copy;

    avail_copy = initializeAvailableInventoryCopy(avail_inventory);

    printing = asprintf(&buffer_global, "Entering trade mode with %s.\n", realm);
    write(1, buffer_global, printing);
    free(buffer_global);
    buffer_global = NULL;

    // avail_inventory = requestAvailableProducts();
    showAvailableProducts(avail_copy);

    trade = createTradeInventory(avail_copy);

    //Bucle para leer los comandos y hacer la lógica de ellos
    do {
        word_count = 0;

        //Leer y comprobar que no hay errores
        printBold("(trade)> ");
        string_read = readUntil(0, '\n');
        command_separated = NULL;
        if (string_read != NULL) {
            word_count = separateCommand(&command_separated, string_read);
            if (command_separated != NULL) {
                argument_count = separateTradeArgs(&command, &command_args, command_separated, word_count);
                command_index = getTradeIndex(command, command_args, avail_copy, trade, argument_count);
            } else {
                emptyStringError();
            }
        } else {
            emptyStringError();
        }

        //Hacer la lógica de cada comando
        switch (command_index) {
            case INVALID_COMMAND:
                // Hacer nada; No hacer ningun commando. Los errores ya se han controlado
                break;
            case ADD:
                addToTrade(&avail_copy, &trade, command_args);
                break;
            case REMOVE:
                removeFromTrade(&avail_copy, &trade, command_args);
                break;
            case SEND:
                buffer_global = malloc(sizeof(char) * (strlen(realm) + strlen(".txt") + 1));
                strcpy(buffer_global, realm);
                strcat(buffer_global, ".txt");

                tradeFile(trade, buffer_global);
                
                sendTrade(trade, realm, buffer_global, &reception);

                freeProducts(&avail_copy, &trade);
                exit_flag = 1;

                free(buffer_global);
                buffer_global = NULL;
                break;
            case CANCEL:
                freeProducts(&avail_copy, &trade);
                exit_flag = 1;
                break;
            default:
                break;
        }

        //Liberar memoria de las variables auxiliares
        freeBuffers();
        /*if (string_read != NULL) {
            free(string_read);
            if (command_separated != NULL) {
                free(command);
                if (argument_count != 0) {
                    for (int i = 0; i < argument_count; i++) {
                        free(command_args[i]);
                    }
                    free(command_args);
                    argument_count = 0;
                }
                for (int i = 0; i < word_count; i++) {
                    free(command_separated[i]);
                }
                free(command_separated);
            }
        }*/

    } while (!exit_flag);
}

/***********************************************
*
* @Finalitat: Mirar el estado de los pledge (No implementado)
* @Parametres: ---
* @Retorn:  ---
*
************************************************/
void pledgeStatus (ReceptionInfo *reception) {
    int i = 0;
    int printing = 0;
    char *buffer = NULL;

    pthread_mutex_lock(&reception->pledge_status_mutex);
    
    if (reception->pledge_status.status_count == 0) {
        printReset("You have no pledges awaiting or accepted.\n");
    } else {
        for (i = 0; i < reception->pledge_status.status_count; i++) {
            printing = asprintf(&buffer, "- %s: ", reception->pledge_status.pledges[i].realm_name);
            write(1, buffer, printing);
            free(buffer);
            buffer = NULL;
            
            if (reception->pledge_status.pledges[i].status == PENDING) {
                printReset("PENDING\n");
            } else if (reception->pledge_status.pledges[i].status == ACCEPTED) {
                printReset("ACCEPTED\n");
            } else if (reception->pledge_status.pledges[i].status == REJECTED) {
                printReset("REJECTED\n");
            }
        }
    }

    pthread_mutex_unlock(&reception->pledge_status_mutex);
}

/***********************************************
*
* @Finalitat: Mirar el  estado de los envoy (No implementado)
* @Parametres: ---
* @Retorn: ---
*
************************************************/
void envoyStatus(){
    notImplementedYet();
}

/***********************************************
*
* @Finalitat: Liberar la memoria de la información utilizada por el maesster
* @Parametres:  in/out: maester = información que se desea liberar sobre el maester
                in/out: maester = información que se desea liberar sobre los productos
* @Retorn:  ---
*
************************************************/
void exitMaester (Inventory* inventory) {
    int i = 0;      // Contador

    // Libera información del inventario
    if (inventory != NULL) {
        free(inventory->products);
        inventory->products = NULL; 
        inventory->product_count = 0;
    }

    // Libera información del maester
    if (maester.routes != NULL) {
        for (i = 0; i < maester.num_routes; i++) {
            free(maester.routes[i].realm_name);
        }
    }
    free(maester.routes);
    maester.routes = NULL;
    free(maester.realm_name);
    maester.realm_name = NULL;
    free(maester.path);
    maester.path = NULL;

    maester.num_routes = 0;

}

// Thread
void* receiveClients (void* arg) {
    ReceptionInfo* reception = (ReceptionInfo*)arg;
    Protocol protocol;
    int flag = 0;
    int error = 0;

    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    /*asprintf(&buffer_threads,"\n\nIP : %s\n\n", inet_ntoa(reception->maester.ip));
    write(1, buffer_threads, strlen(buffer_threads));
    free(buffer_threads);
    buffer_threads = NULL;*/

    fd_socket = createSocket(maester.ip, maester.port);

    if (fd_socket == -1) {
        // imprimir algo
        showText("Failed creating receiving socket.\n");
        return NULL;
    }

    do {
        fd_client = accept(fd_socket, &client_addr, &addrlen);

        char *ip_string = NULL;
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_string, sizeof(ip_string));

        read(fd_client, &protocol, sizeof(Protocol));

        error = checkDestinationRequest(protocol, client_addr, &maester, &reception);

        if (error != -1) {
            manageReception(&reception, &protocol, &maester, flag);
        }

        close(fd_client);
    } while (1);

    showText("Gets out??\n");
    
    return NULL;
}

// Thread
void* sendFiles (void* arg) {
    ReceptionInfo* reception = (ReceptionInfo*)arg;
    int flag = 0;
    int file_opened = 0;
    int fd = 0;
    char data[DATA_MAX_SIZE];
    char *temp_data = NULL;
    Protocol protocol;
    struct in_addr ip;
    int port = 0;
    int bytes_read = 0;
    int remaining_bytes = 0;
    int file_size = 0;
    
    do {
        if ((reception->send_listproducts == 1) || (reception->send_sigil == 1) || (reception->send_trade == 1)) {
            if (!file_opened) {
                if (reception->send_listproducts == 1) {
                    prepareSendingFile(&file_size, &fd, reception, maester, SENDING_INVENTORY);
                } else if (reception->send_sigil == 1) {
                    prepareSendingFile(&file_size, &fd, reception, maester, SENDING_SIGIL);
                } else if (reception->send_trade == 1) {
                    prepareSendingFile(&file_size, &fd, reception, maester, SENDING_TRADE);
                }

                getIpAndPortFromRealm(maester, reception->realm_name_sending, &ip, &port);
                file_opened = 1;
            }
            
            if ((reception->bytes_sent < file_size) && file_opened) {
                remaining_bytes = file_size - reception->bytes_sent;
                
                if (DATA_MAX_SIZE <= remaining_bytes) {
                    bytes_read = read(fd, data, DATA_MAX_SIZE);
                    if (reception->send_listproducts == 1) {
                        protocol = commProductTransfer(maester, reception->realm_name_sending, reception->inventory_file_name, data, bytes_read);
                    } else if (reception->send_sigil == 1) {
                        protocol = commSigilTransfer(maester, reception->realm_name_sending, data, bytes_read);
                    } else if (reception->send_trade == 1) {
                        protocol = commOrderTransfer(maester, reception->realm_name_sending, data, bytes_read);
                    }
                    sendToRoute(reception->realm_name_sending, protocol, maester);
                    reception->bytes_sent += bytes_read;
                } else {
                    temp_data = malloc(sizeof(char) * remaining_bytes);
                    bytes_read = read(fd, temp_data, remaining_bytes);
                    if (reception->send_listproducts == 1) {
                        printReset("[DEBUG LIST PRODUCTS] The list has been sealed and sent forth.\n");
                        printBold("$ ");
                        protocol = commProductTransfer(maester, reception->realm_name_sending, reception->inventory_file_name, temp_data, bytes_read);
                    } else if (reception->send_sigil == 1) {
                        protocol = commSigilTransfer(maester, reception->realm_name_sending, temp_data, bytes_read);
                    } else if (reception->send_trade == 1) {
                        protocol = commOrderTransfer(maester, reception->realm_name_sending, temp_data, bytes_read);
                    }

                    sendToRoute(reception->realm_name_sending, protocol, maester);
                    reception->bytes_sent += bytes_read;
                    free(temp_data);
                    
                    close(fd);
                    file_opened = 0;
                    
                    reception->send_listproducts = 0;
                    reception->send_sigil = 0;
                    //reception->send_trade = 0;
                    reception->bytes_sent = 0;

                    if (reception->sigil_file_path != NULL) {
                        free(reception->sigil_file_path);
                        reception->sigil_file_path = NULL;
                    }

                    /*if (reception->trade_file_path != NULL) {
                        free(reception->trade_file_path);
                        reception->trade_file_path = NULL;
                    }*/
                }
            }
        }
    } while (!flag);
    
    return NULL;
}

// Alarm
void timerFlag () {
    timer_flag = 0;
}

// Ctrl+C
void unexpectedExit () {
    sendDisconection(maester);
    freeBuffers();

    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
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
            if (reception->sigil_file_path != NULL) {
                free(reception->sigil_file_path);
                reception->sigil_file_path = NULL;
            } 
            reception->sigil_file_path = realloc(reception->sigil_file_path, sizeof(char) * (strlen( "Sigils/") + strlen(command_args[SECOND_ARGUMENT]) + 1));
            reception->sigil_file_path[0] = '\0';
            strcat(reception->sigil_file_path, "Sigils/");
            strcat(reception->sigil_file_path, command_args[SECOND_ARGUMENT]);

            pledge(command_args[FIRST_ARGUMENT], reception->sigil_file_path, reception);
            break;
        case PLEDGE_RESPOND:
            if (reception->pledge_pending) {
                pledgeRespond(command_args[FIRST_ARGUMENT], command_args[SECOND_ARGUMENT], reception->pledge_ip, reception->pledge_port, reception);
                reception->pledge_pending = 0;
            } else {
                // No me han enviado ningun pledge
                printReset("No pledge has been received.\n");
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
            if (checkTradeInventory(reception, command_args[FIRST_ARGUMENT])) {
                startTrade(command_args[FIRST_ARGUMENT], getRealmInventory(reception, command_args[FIRST_ARGUMENT]), reception);
            } else {
                printReset("No products available. Use LIST PRODUCTS first.\n");
            }
            break;
        case PLEDGE_STATUS:
            pledgeStatus(reception);
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

// Procedimiento principal
int main (int argc, char **argv) {
    Inventory inventory;                // El inventario del maester
    ReceptionInfo reception;

    int error_config_file = 0;          // Flag que guarda si hay un error en el fichero de configuración
    int error_inventory_file = 0;       // Flag que guarda si hay un error en el fichero del inventario
    int file_errors = 0;                // Flag que guarda si hay un error en cualquier o los dos ficheros
    int exit_flag = 0;                  // Flag que guarda si se tiene que salir de estar esperando a los comandos
    int command_index = 0;              // Guarda el comando como el index que representa baseado en el commands.h

    pthread_t reception_thread;
    pthread_t sending_thread;
    
    signal(SIGINT, unexpectedExit);
    signal(SIGALRM, timerFlag);

    // Si se han entrado menos que 3 argumentos
    if (argc < 3) {
        showText("\nToo few arguments to initialize the maester. Correct format:\n./SO_Project.exe <configuration file> <inventory file>\n\n");
        exit(1);
    } else if (argc > 3) {
        showText("\nToo many arguments to initialize the maester. Correct format:\n./SO_Project.exe <configuration file> <inventory file>\n\n");
        exit(1);
    }

    error_config_file = readConfigFile(&maester, argv[1]);
    error_inventory_file = readInventoryFile(&inventory, argv[2]);

    file_errors = checkFileErrors(error_config_file, error_inventory_file);

    // Si no hay ningún error en ningún fichero
    if (file_errors) {
        exit(1);
    }

    showInitializedMaester(maester);
    initializeReception(&reception, inventory, argv[2]);

    pthread_create(&reception_thread, NULL, receiveClients, &reception);
    pthread_create(&sending_thread, NULL, sendFiles, &reception);
    

    exit_flag = 0;
    timer_flag = 0;
    

    // Recorre el bucle de comandos mientras que el usuario no escribe "EXIT"
    do {
        command_index = manageCommand(&word_count, &argument_count, &string_read, &command_separated, &command_args, &command, maester);

        // A partir de aqui, solo trabajamos con command y command_args
        // Se separa cada caso de cada comando en el switch
        exit_flag = switchOfCommands(&reception, reception.inventory, command_index);

        // Liberar memoria de las variables que reservan su memoria
        freeBuffers();
        
    } while(!exit_flag); //Tiene que hacerlo hasta que sea exit

    return 0;
}