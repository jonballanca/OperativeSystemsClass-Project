#include "commands.h"

// Constante - Comandos que existen inicializados como constantes, con su nombre, numero de index, argumento y mensaje de error de argumentos.
const Command COMMAND_CONST[] = {
    {"INVALID COMMAND", 0, 0, ""},
    {"LIST REALMS", 0, 1, "Did you mean to list the realms? Please review syntax:\n\tLIST REALMS"},
    {"PLEDGE", 2, 2, "Did you mean to send a pledge? Please review syntax:\n\tPLEDGE <REALM> <sigil.jpg>"},
    {"PLEDGE RESPOND", 2, 3, "Did you mean to send a pledge response? Please review syntax:\n\tPLEDGE RESPOND <REALM> ACCEPT / REJECT"},
    {"LIST PRODUCTS", 1, 4, "Did you mean to send list all products of a realm? Please review syntax: \n\tLIST PRODUCTS <REALM>"},           // Sin argumentos
    {"LIST PRODUCTS", 0, 5, ""},
    {"START TRADE", 1, 6, "Did you mean to start a trade with a realm? Please review syntax:\n\tSTART TRADE <REALM>"},
    {"PLEDGE STATUS", 0, 7, "Did you mean to see a pledge status? Please review syntax:\n\tPLEDGE STATUS"},
    {"ENVOY STATUS", 0, 8, "Did you mean to see an envoy status? Please review syntax:\n\tENVOY STATUS"},
    {"EXIT", 0, 9, "Did you mean to exit the system? Please review syntax.\n\tEXIT"}
};

// Constante - Comandos de "trade" que existen inicializados como constantes, con su nombre, numero de index, argumento y mensaje de error de argumentos.
const Command TRADE_COMMANDS[] = {
    {"INVALID COMMAND", 0, 0, ""},
    {"ADD", 2, 1, "Did you mean to add a product to the trade? Please review syntax:\n\tadd <product> <amount>"},
    {"REMOVE", 2, 2, "Did you mean to remove a product from the trade? Please review syntax:\n\tremove <product> <amount>"},
    {"SEND", 0, 3, "Did you mean to send the trade? Please review syntax:\n\tsend"},
    {"CANCEL", 0, 4, "Did you mean to cancel the trade? Please review syntax:\n\tcancel"}
};

void printUnknownCommand () {
    int printing;                       // La variable que guarda el número de bytes para escribir
    char* buffer;                       // Buffer que guarda lo que se escribe por la terminal

    // Simplemente escibe por pantalla en rojo negrita
    printing = asprintf(&buffer, RED BOLD "Unknown command.%s\n", RESET);
    write(1, buffer, printing);
    free(buffer);
}

void printIncorrectArgs (int index, int argument_count) {
    int printing;                       // La variable que guarda el número de bytes para escribir
    char* buffer;                       // Buffer que guarda lo que se escribe por la terminal

    if ((COMMAND_CONST[index].indexConstant == START_TRADE) && (argument_count == 0)) {
        // Simplemente escibe por pantalla en rojo negrita, y se pone el comando como invalido
        printing = asprintf(&buffer, RED BOLD "Missing arguments, can't start a trade. Please review the syntax.%s\n", RESET);
        write(1, buffer, printing);
        free(buffer);
    } else {
        // Simplemente escibe por pantalla en rojo negrita, y se pone el comando como invalido
        printing = asprintf(&buffer, RED BOLD "Wrong number of arguments. %s%s\n", COMMAND_CONST[index].no_args_error, RESET);
        write(1, buffer, printing);
        free(buffer);
    }
}

void printUnknownTradeCommand() {
    int printing = 0;                       // La variable que guarda el número de bytes para escribir
    char *buffer;                           // Buffer que guarda lo que se escribe por la terminal

    // Simplemente escibe por pantalla en rojo negrita, y se pone el comando como invalido
    printing = asprintf(&buffer, RED BOLD "Unknown trade command.%s\n", RESET);
    write(1, buffer, printing);
    free(buffer);
}

void printProductUnavailable (char **arguments) {
    int printing = 0;                       // La variable que guarda el número de bytes para escribir
    char *buffer;                           // Buffer que guarda lo que se escribe por la terminal

    // Simplemente escibe por pantalla en rojo negrita, y se pone el comando como invalido
    printing = asprintf(&buffer, "The product \"%s\" is not available. Please review the available products.%s\n", arguments[FIRST_ARGUMENT], RESET);
    write(1, buffer, printing);
    free(buffer);
}

void printSynthaxError (char* current_command) {
    int printing = 0;                       // La variable que guarda el número de bytes para escribir
    char *buffer;                           // Buffer que guarda lo que se escribe por la terminal

    // Simplemente escibe por pantalla en rojo negrita, y se pone el comando como invalido
    if (!strcmp(current_command, "ADD")) {
        printing = asprintf(&buffer, RED BOLD "%s%s\n", TRADE_COMMANDS[ADD].no_args_error, RESET);
        write(1, buffer, printing);
        free(buffer);
    } else if (!strcmp(current_command, "REMOVE")) {
        printing = asprintf(&buffer, RED BOLD "%s%s\n", TRADE_COMMANDS[REMOVE].no_args_error, RESET);
        write(1, buffer, printing);
        free(buffer);
    } else {
        printing = asprintf(&buffer, RED BOLD "Synthax error.%s\n", RESET);
        write(1, buffer, printing);
        free(buffer);
    }
}

int findTradeIndex (char* current_command) {
    int index = INVALID_COMMAND;            // El index que representa el comando que se ha comprobado
    int i = 0;                          // Contador

    // Recorre todos los comandos constantes y comprueba si alguno es igual al argumento
    for (i = 0; (i < MAX_TRADE_COMMANDS) && (index == INVALID_COMMAND); i++) {
        if (!strcmp(current_command, TRADE_COMMANDS[i].name)) {
            index = TRADE_COMMANDS[i].indexConstant;
        }
    }
    return index;
}

int getProductPosition (Inventory avail_inventory, char** arguments) {
    char *aux_name;                         // Buffer que guarda temporalmente el nombre de un producto
    int product_pos = 0;                    // Guarda la posición del producto cuando se recorre para comprobaciónes
    int i = 0;                              // Contador

    // Recorre cada producto del inventario disponible en mayúsculas y mira si
    // el argumento pasado, que tiene que ser un producto, realmente es un 
    // producto que existe
    for (i = 0; i < avail_inventory.product_count; i++) {
        aux_name = (char*)malloc(sizeof(char) * (strlen(avail_inventory.products[i].name) + 1));
        strcpy(aux_name, avail_inventory.products[i].name);
        convertToCapitalLetters(aux_name);

        if (!strcmp(aux_name, arguments[FIRST_ARGUMENT])) {
            product_pos = i;
        }

        free(aux_name);
    }

    return product_pos;
}

int checkAddRemove (Inventory avail_inventory, Inventory trade, char** arguments, int command_index, int product_pos, int index) {
    int printing = 0;                       // La variable que guarda el número de bytes para escribir
    char *buffer;                           // Buffer que guarda lo que se escribe por la terminal

    // Mira y escribe por pantalla si hay un error en el argumento pasado por argumento
    // para añadir o quitar un producto en el "trade"
    if (!checkAmountLimit(avail_inventory, trade, arguments[SECOND_ARGUMENT], command_index, product_pos)) {
        // Simplemente escibe por pantalla en rojo negrita, y se pone el comando como invalido
        index = INVALID_COMMAND;
        printing = asprintf(&buffer, RED BOLD "Cannot add %d products when only %d products are available for the trade. Choose a lower amount.%s\n", atoi(arguments[SECOND_ARGUMENT]), avail_inventory.products[product_pos].amount, RESET);
        write(1, buffer, printing);
        free(buffer);
    }

    return index;
}

int findCommandIndex (char* current_command, int argument_count) {
    int index = INVALID_COMMAND;        // Index que representa el comando que se ha comprobado
    int i = 0;                          // Contador

    // Recorre todos los comandos constantes y comprueba si alguno es igual al argumento
    for (i = 0; (i < MAX_COMMANDS) && (index == INVALID_COMMAND); i++) {
        if (!strcmp(current_command, COMMAND_CONST[i].name)) {
            if ((i == LIST_PRODUCTS) && (argument_count == 0)) {
                i++;
            }
            index = COMMAND_CONST[i].indexConstant;
        }
    }
    return index;
}

int checkPledge (Maester maester, char** arguments, int index) {
    int correct_argument = 0;           // Flag que guarda si el argumento no tiene errores
    int printing;                       // La variable que guarda el número de bytes para escribir
    char* buffer;                       // Buffer que guarda lo que se escribe por la terminal

    // Si el realm pasado por argumento existe.
    correct_argument = realmExists(maester, arguments[FIRST_ARGUMENT], index);
    if (correct_argument) {
        correct_argument = 0;
        correct_argument = sigilExists(arguments[SECOND_ARGUMENT]);
        if (!correct_argument) {
            // Simplemente escibe por pantalla en rojo negrita, y se pone el comando como invalido
            // No se pone como invalido para la Fase 1.
            //index = INVALID_COMMAND;
            printing = asprintf(&buffer, RED BOLD "The sigil that was entered does not exist. Please enter an existing sigil.%s\n", RESET);
            write(1, buffer, printing);
            free(buffer);
            index = INVALID_COMMAND;
        }
    } else {
        index = INVALID_COMMAND;
    }

    return index;
}

int checkPledgeRespond (Maester maester, char **arguments, int index) {
    int correct_argument = 0;           // Flag que guarda si el argumento no tiene errores
    int printing;                       // La variable que guarda el número de bytes para escribir
    char* buffer;                       // Buffer que guarda lo que se escribe por la terminal

    // Si el realm pasado por argumento existe.
    correct_argument = realmExists(maester, arguments[FIRST_ARGUMENT], index);
    if (correct_argument) {

        // Si los siguientes argumentos no son "ACCEPT" o "REJECT"
        correct_argument = 0;
        correct_argument = tradePending();
        if (!correct_argument) {
            if (!((!strcmp("ACCEPT", arguments[SECOND_ARGUMENT])) || (!strcmp("REJECT", arguments[SECOND_ARGUMENT])))) {
                // Simplemente escibe por pantalla en rojo negrita, y se pone el comando como invalido
                // No se pone como invalido para la Fase 1.
                //index = INVALID_COMMAND;
                printing = asprintf(&buffer, RED BOLD "The second entered argument is of incorrect format.\nPlease enter ACCEPT or REJECT.%s\n", RESET);
                write(1, buffer, printing);
                free(buffer);
                index = INVALID_COMMAND;
            }
        } else {
            // Simplemente escibe por pantalla en rojo negrita, y se pone el comando como invalido
            // No se pone como invalido para la Fase 1.
            //index = INVALID_COMMAND;
            printing = asprintf(&buffer, RED BOLD "You have not recieved any pledges from %s.%s\n", arguments[FIRST_ARGUMENT], RESET);
            write(1, buffer, printing);
            free(buffer);
            index = INVALID_COMMAND;
        }
    } else {
        // Simplemente escibe por pantalla en rojo negrita, y se pone el comando como invalido
        // No se pone como invalido para la Fase 1.
        //index = INVALID_COMMAND;
        printing = asprintf(&buffer, RED BOLD "Realm \"%s\" does not exist. Please enter an existing realm.%s\n", arguments[FIRST_ARGUMENT], RESET);
        write(1, buffer, printing);
        free(buffer);
        index = INVALID_COMMAND;
    }

    return index;
}

void getCommandInput (char **string_read) {
    int printing = 0;                   // Variable que guarda los bytes para escribir por la terminal
    char *buffer = NULL;
    
    showText(BOLD "$ ");

    // Lee el del usuario desde la terminal
    *string_read = readUntil(0, '\n');
    printing = asprintf(&buffer, "%s", RESET);
    write(1, buffer, printing);
    free(buffer);
    buffer = NULL;
}

int manageCommand (int *word_count, int *argument_count, char **string_read, char ***command_separated, char ***command_args, char **command, Maester maester) {
    int command_index = 0;

    *word_count = 0;
    *argument_count = 0;

    getCommandInput(string_read);

    // Se reinicia el array
    *command_separated = NULL;

    // Si se ha leido algo
    if (*string_read != NULL) {

        // Si lo que ha leido no ha sido todo espacios
        *word_count = separateCommand(command_separated, *string_read);
        if (*command_separated != NULL) {
            
            // Se separa el string leido en comando y argumentos
            *argument_count = separateArguments(command, command_args, *command_separated, *word_count);

            // Se guarda el index del comando
            command_index = getCommandIndex(*command, *command_args, *argument_count, maester);
        } else {
            emptyStringError();
        }
    } else {
        emptyStringError();
    }

    return command_index;
}

void convertToCapitalLetters (char* word) {
    int i = 0;      // Contador

    // Bucle que recore todas las letras de la palabra
    for (i = 0; i < (int)strlen(word); i++) {
        word[i] = (char)toupper((unsigned char)word[i]);
    }
}

void convertToLowercaseLetters (char* word) {
    int i = 0;      // Contador

    // Bucle que recore todas las letras de la palabra
    for (i = 0; i < (int)strlen(word); i++) {
        word[i] = (char)tolower((unsigned char)word[i]);
    }
}

void messageNoExists (char* realm_name) {
    char* buffer;           // Buffer para escribir
    int num_bytes = 0;      // Numero de bytes para escribir

    // Simplemente escibe por pantalla en rojo negrita
    num_bytes = asprintf(&buffer, RED BOLD "\nERROR: The realm %s doesn't exists%s\n", realm_name, RESET);
    write(1, buffer, num_bytes);
    free(buffer);
}    

void messageNoAlliance (char* realm_name) {
    char* buffer;           // Buffer para escribir
    int num_bytes = 0;      // Numero de bytes para escribir

    // Simplemente escibe por pantalla en rojo negrita
    num_bytes = asprintf(&buffer, RED BOLD"\nERROR: You must have an alliance with %s to trade or list their products.%s\n", realm_name, RESET);
    write(1, buffer, num_bytes);
    free(buffer);
}

int realmExists (Maester maester, char* realm_name, int index) {
    int i = 0;                          // Contador
    int exists = 0;                     // Flag para indicar si existe
    int correct_arguments = 1;          // Flag para indicar si el argumento esta bien
    char* aux_name;                     // Buffer para guardar el nombre en mayúsculas
        
    // Recorre todas las rutas del realm hasta que encuentra uno igual que realm_name
    // y mira si hay una alianza con ese realm.
    for (i = 0; (i < maester.num_routes) && !exists; i++) {
        aux_name = (char*)malloc(sizeof(char) * (strlen(maester.routes[i].realm_name) + 1));
        strcpy(aux_name, maester.routes[i].realm_name);
        convertToCapitalLetters(aux_name);

        if (!strcmp(realm_name, "DEFAULT")) {
            free(aux_name);
            correct_arguments = 0;
            return correct_arguments;
        }

        // Mira si existe el realm
        if (!strcmp(aux_name, realm_name)) {
            exists = 1;
            
            // Mira si hay alianza
            if (!maester.routes[i].alliance_exists && !((index == PLEDGE_RESPOND) || (index == PLEDGE) || (index == PLEDGE_STATUS))) {
                messageNoAlliance(maester.routes[i].realm_name);
                correct_arguments = 0;
            }
        }
        free(aux_name);
    }

    // Muestra mensaje de error
    if (!exists) {
        messageNoExists(realm_name);
        correct_arguments = 0;
    }

    return correct_arguments;
}

int sigilExists (char* filename) {
    int fd;                 // File descriptor para abrir el fichero
    int exists = 0;         // Flag que indica si existe el fichero
    char* path;             // Buffer que guarda el path del fichero

    // Convierte el fichero todo en minúsculas, como está en el directorio
    convertToLowercaseLetters(filename);

    // Reservar memoria para el buffer del path
    path = (char*)malloc(sizeof(char)*(strlen("Sigils/") + strlen(filename) + 1));

    // Concatenar las cadenas para el path
    strcpy(path, "Sigils/");
    strcat(path, filename);

    // Mira si se puede abrir el fichero para ver si el fichero existe
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        exists = 1;
        close(fd);
    }
    free(path);

    return exists;
}

int productExists (Inventory inventory, char* argument) {
    int exists = 0;         // Flag que indica si el producto existe
    int i = 0;              // Contador
    char * aux_name;        // Buffer para guardar el numbre en mayúsculas

    // Recorre cada nombre de producto para comparar con el argumento,
    // todo en mayíscilas.
    for (i = 0; i < inventory.product_count; i++) {
        aux_name = (char*)malloc(sizeof(char) * (strlen(inventory.products[i].name) + 1));
        strcpy(aux_name, inventory.products[i].name);
        convertToCapitalLetters(aux_name);
        
        // Si el producto seleccionado es igual al argumento, es que existe
        if (!strcmp(aux_name, argument)) {
            exists = 1;
        }
        free(aux_name);
    }

    return exists;
}

int checkAmountLimit(Inventory avail_inventory, Inventory trade, char* amount_arg, int command_index, int product_pos) {
    int correct = 1;        // Flag que indica si la cantidad del producto es dentro de los límites
    
    // Función para los dos casos de añadir o eliminar un producto
    switch (command_index) {
        case ADD:
            // Si el número del producto que se quiere añadir es mayor que la cantidad del producto disponible
            if (atoi(amount_arg) > avail_inventory.products[product_pos].amount) {
                correct = 0;
            }
            break;
        case REMOVE:
            // Si el número del producto que se quiere eliminar es mayor que la cantidad del producto seleccionado
            if (atoi(amount_arg) > trade.products[product_pos].amount) {
                correct = 0;
            }
            break;
        default:
            break;
    }

    return correct;
}

int tradePending () {
    // No se implementa para la Fase 1
    return 0;
}

int isNumber (char* word) {
    int is_number = 1;      // Flag que indica si la palabra es un número
    int i = 0;              // Contador

    // Recorre toda la palabra, carácter por carácter y mira si alguno no es un numero
    for(i = 0; (i < (int)strlen(word)) && is_number; i++){
        if (!isdigit(word[i])) {
            is_number = 0;
        }
    }

    
    return is_number;
}

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

int checkFileErrors (int error_config_file, int error_inventory_file) {
    int file_error = 1;     // Flag que indica si ha habido un error de fichero

    // Si ha habido un error en los dos ficheros
    if ((error_config_file == ERROR_CONFIG_FILE) && (error_inventory_file == ERROR_INVENTORY_FILE)) {
        showText("\nThere was an error when openning both the Configuration AND Inventory file.\n\n");
    } else {
        // Si ha habido un error en el fichero de configuración
        if (error_config_file == ERROR_CONFIG_FILE) {
            showText("\nThere was an error when openning the Configuration file.\n\n");
        } else {
            // Si ha habido un error en el fichero de inventario
            if (error_inventory_file == ERROR_INVENTORY_FILE) {
                showText("\nThere was an error when openning the Configuration file.\n\n");
            } else {
                file_error = 0;
            }
        }
    }

    return file_error;
}

int separateCommand (char ***command_separated, char *string_read) {
    int word_count = 0;         // Número de palabras detectados
    int i = 0, j = 0;           // Contadores
    char* word = NULL;          // Buffer que va guardando la palabra para separarla

    // El caso donde hay espacios antes de que empieze la palabra,
    // se guardan para indicar comando incorrecto luego
    while (string_read[i] == ' ') {
        word = realloc(word, (j + 2));
        word[j++] = string_read[i++];
    }

    // Si solo ha leido espacios
    if (string_read[i] == '\0') {
        word[j] = '\0';
        j = 0;
    }

    // Mientras que no llega al final del string
    while (string_read[i] != '\0') {
        // No incluye espacios ni el '\0'. En caso de espacio se quiere saltar
        while ((string_read[i] != ' ') && (string_read[i] != '\0')) {
            word = realloc(word, (j + 2));
            word[j++] = string_read[i++];
        }
        word[j] = '\0';

        // Salta espacios entre palabras
        while (string_read[i] == ' ') {
            i++;
        }
        
        // Si ha leido algo que no es un espacio o '\0', se quiere guardar como palabra separada
        if (j > 0) {
            j = 0;
            word_count++;
            // Se aumenta la memoria para añadir una palabra más
            *command_separated = realloc(*command_separated, sizeof(char*) * word_count);
            (*command_separated)[word_count - 1] = malloc(strlen(word) + 1);
            convertToCapitalLetters(word);

            // Se controla el caso particular del sigil, para que el path del fichero esté en minúsculas
            if ((!strcmp(*command_separated[0], COMMAND_CONST[1].name)) && (word_count == 3)) {
                convertToLowercaseLetters(word);
            }
            strcpy((*command_separated)[word_count - 1], word);
        }
    }

    free(word);

    return word_count;
}

int separateArguments (char** command, char*** command_args, char** command_separated, int word_count) {
    int argument_pos = 1;               // La posición de los argumentos en el command_separated
    int argument_count = 0;             // El número de argumentos del comando
    int total_argument_count = 0;       // El número total de argumentes que se devuelve

    // Si la primera palabra no es el comando EXIT y no hay mas palabras
    if (strcmp(command_separated[0], "EXIT") && (word_count != 1)) {

        // Si la primera palabra es el comando PLEDGE y la segunda no es ni RESPOND ni STATUS
        if (!strcmp(command_separated[0], "PLEDGE") && (strcmp(command_separated[1], "RESPOND") && (strcmp(command_separated[1], "STATUS")))) {
            *command = malloc(strlen(command_separated[0]) + 1);
            strcpy(*command, command_separated[0]);
        } else {
            *command = malloc(strlen(command_separated[0]) + strlen(command_separated[1]) + 2);
            strcpy(*command, command_separated[0]);
            strcat(*command, " ");
            strcat(*command, command_separated[1]);
            argument_pos++;
        }
    } else {
        *command = malloc(strlen(command_separated[0]) + 1);
        strcpy(*command, command_separated[0]);
    }

    // Se reserva el espacio para los argumentos despues de haber guardado el comando
    *command_args = malloc(sizeof(char*) * (word_count - argument_pos + 1));

    total_argument_count = word_count - argument_pos;
    
    // Recorre cada palabra que queda y la va guardando
    while (argument_pos < word_count) {
        (*command_args)[argument_count] = malloc(strlen(command_separated[argument_pos]) + 1);
        strcpy((*command_args)[argument_count], command_separated[argument_pos]);
        argument_count++;
        argument_pos++;
    }

    return total_argument_count;
}

int separateTradeArgs (char** command, char*** command_args, char** command_separated, int word_count) {
    int argument_pos = 1;           // La posición de los argumentos en el command_separated
    int total_args = 0;             // El número total de argumentos dependiendo del caso

    // Se quiere guardar la primera palabra en el command, la ultima palabra en el command_separated[1],
    // y todo lo que hay entre los dos en el command_separated[0]

    // Reserva memoria para el comando
    *command = malloc(strlen(command_separated[0]) + 1);
    strcpy(*command, command_separated[0]);

    // Si hay argumentos para guardar y no solo comando
    if (word_count > 1) {
        // Reserva memoria para el array de argumentos, para la posicion 0 y 1, como se ha indicado arriba
        *command_args = malloc(sizeof(char*) * 2);

        // Reserva memoria para las lettras del argumento 0
        (*command_args)[0] = malloc(strlen(command_separated[1]) + 1);
        strcpy((*command_args)[0], command_separated[1]);

        argument_pos = 2;
        while (argument_pos < (word_count - 1)) {
            // Se aumenta memoria para las lettras del argumento 0 y se añade un espacio entre palabras
            (*command_args)[0] = realloc((*command_args)[0], strlen((*command_args)[0]) + 1 + strlen(command_separated[argument_pos]) + 1);
            strcat((*command_args)[0], " ");
            strcat((*command_args)[0], command_separated[argument_pos]);

            argument_pos++;
        }

        // Se reserva el espcaio para el argumento de posición 1 y se guarda la última palabra del
        // comando separado en palabras
        (*command_args)[1] = malloc(strlen(command_separated[word_count - 1]) + 1);
        strcpy((*command_args)[1], command_separated[word_count - 1]);

        // Solo hay 2 argumentos en esta estructura si hay algo más que solo el comando (word_count > 1)
        total_args = 2;
    }

    return total_args;
}

int getTradeIndex (char* current_command, char** arguments, Inventory avail_inventory, Inventory trade, int argument_count) {
    int index = INVALID_COMMAND;            // El index que representa el comando que se ha comprobado
    int product_pos = 0;                    // Guarda la posición del producto cuando se recorre para comprobaciónes

    index = findTradeIndex(current_command);

    // Si el comando no se ha encontrado
    if (index == INVALID_COMMAND) {
        printUnknownTradeCommand();
        return index;
    }

    // Si el comando no tiene argumentos
    if ((argument_count == 0) && !(index == SEND) && (strcmp(current_command, "CANCEL"))) {
        printSynthaxError(current_command);
        index = INVALID_COMMAND;
        return index;
    } else if ((argument_count == 0) && ((index == SEND) || !strcmp(current_command, "CANCEL"))) {
        return index;
    }

    // Si el último argumento es un numero
    if (!isNumber(arguments[1])) {
        printSynthaxError(current_command);
        index = INVALID_COMMAND;
        return index;
    }

    // Si el comando como no tiene argumentos
    if (TRADE_COMMANDS[index].numArgs == 0) {
        return index;
    }

    // Si el producto no existe
    if (!productExists(avail_inventory, arguments[FIRST_ARGUMENT])) {
        printProductUnavailable(arguments);
        index = INVALID_COMMAND;
        return index;
    }

    product_pos = getProductPosition(avail_inventory, arguments);

    // Separa los dos casos que tienen argumentos
    if (index == ADD) {
        index = checkAddRemove(avail_inventory, trade, arguments, ADD, product_pos, index);
    } else {
        index = checkAddRemove(avail_inventory, trade, arguments, REMOVE, product_pos, index);
    }
    
    return index;
}

int getCommandIndex (char* current_command, char** arguments, int argument_count, Maester maester) {
    int index = INVALID_COMMAND;        // Index que representa el comando que se ha comprobado

    index = findCommandIndex(current_command, argument_count);

    // Si el comando no se ha encontrado
    if (index == INVALID_COMMAND) {
        printUnknownCommand();
        return index;
    }

    // Si el comando no tiene el número de argumentos que debería tener
    if (argument_count != COMMAND_CONST[index].numArgs) {
        printIncorrectArgs(index, argument_count);
        index = INVALID_COMMAND;
        return index;
    }

    // Si el comando no tiene argumentos
    if (COMMAND_CONST[index].numArgs == 0) {
        return index;
    }

    // Separa cada caso del comando que tiene argumentos
    switch (index) {
        case PLEDGE:
            index = checkPledge(maester, arguments, index);
            break;
        case PLEDGE_RESPOND:
            index = checkPledgeRespond(maester, arguments, index);
            break;
        case LIST_PRODUCTS:
            // Si el realm pasado por argumento existe.
            if (!realmExists(maester, arguments[FIRST_ARGUMENT], index)) {
                index = INVALID_COMMAND;
            }
            break;
        case START_TRADE:
            // Si el realm pasado por argumento existe.
            if (!realmExists(maester, arguments[FIRST_ARGUMENT], index)) {
                index = INVALID_COMMAND;
            }
            break;
        default:
            break;
    }

    return index;
}