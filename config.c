#include "config.h"

void changeTimedOut (char *realm_name, int action, Maester *maester) {
    int i = 0;
    char *aux_name;
    char *aux_realm_name;

    aux_realm_name = (char*)malloc(sizeof(char) * (strlen(realm_name) + 1));
    strcpy(aux_realm_name, realm_name);
    convertToCapitalLetters(aux_realm_name);

    for (i = 0; i < maester->num_routes; i++) {
        aux_name = (char*)malloc(sizeof(char) * (strlen(maester->routes[i].realm_name) + 1));
        strcpy(aux_name, maester->routes[i].realm_name);
        convertToCapitalLetters(aux_name);

        if (!strcmp(aux_realm_name, aux_name)) {
            if (action == TIMED_OUT) {
                maester->routes[i].timed_out = 1;
            } else if (action == RESET_TIMED_OUT) {
                maester->routes[i].timed_out = 0;
            }
        }

        free(aux_name);
    }

    free(aux_realm_name);
}

int checkTimedOut (Maester maester, char *realm_name) {
    int i = 0;
    int timed_out = 0;
    char *aux_name;
    char *aux_realm_name;

    aux_realm_name = (char*)malloc(sizeof(char) * (strlen(realm_name) + 1));
    strcpy(aux_realm_name, realm_name);
    convertToCapitalLetters(aux_realm_name);

    for (i = 0; i < maester.num_routes; i++) {
        aux_name = (char*)malloc(sizeof(char) * (strlen(maester.routes[i].realm_name) + 1));
        strcpy(aux_name, maester.routes[i].realm_name);
        convertToCapitalLetters(aux_name);

        if (!strcmp(aux_realm_name, aux_name)) {
            timed_out = maester.routes[i].timed_out;
        }

        free(aux_name);
    }

    free(aux_realm_name);

    return timed_out;
}

void removeAmpersand (char **buffer) {
    int i = 0, j = 0;           // Contadores
    char *temp;

    temp = malloc(sizeof(char) * (strlen(*buffer) + 1));

    while ((*buffer)[i] != '\0') {
        if ((*buffer)[i] != '&') {
            temp[j++]  = (*buffer)[i];
        }
        i++;
    }

    temp[j] = '\0';
    *buffer = realloc(*buffer, sizeof(char) * (strlen(temp) + 1));
    strcpy(*buffer, temp);
    free(temp);
}

Maester decomposeRoute (Maester maester, char* buffer, int route_pos) {
    int i = 0, j = 0;           // Contadores
    int word_count = 0;         // Variable que cuenta el número de palabras
    char* temp;                 // Buffer que guarda cada palabra por separado

    temp = malloc((strlen(buffer) + 1));

    // Recorre todo el buffer hasta llegar a una siguiente linea o el final
    while ((buffer[i] != '\n') && (buffer[i] != '\0')) {
        temp[j] = buffer[i];
        i++;
        j++;

        // Guarda hasta que haya un espacio, otra linea, o final de string
        if ((buffer[i] == ' ') || (buffer[i] == '\n') || (buffer[i] == '\0')) {
            word_count++;

            temp[j] = '\0';

            // Separa los diferentes casos para guardar
            switch (word_count) {
                case REALM_NAME:

                    // Guarda el nombre del realm de la ruta
                    maester.routes[route_pos].realm_name = malloc((strlen(temp) + 1));
                    removeAmpersand(&temp);
                    strcpy(maester.routes[route_pos].realm_name, temp);
                    i++;
                    break;
                case IP_ADDRESS:

                    // Guarda el IP del realm de la ruta
                    if (temp[0] == '*') {
                        maester.routes[route_pos].ip.s_addr = htonl(INADDR_ANY);
                    } else {
                        inet_pton(AF_INET, temp, &maester.routes[route_pos].ip);
                    }
                    i++;
                    break;
                case PORT_NUMBER:

                    // Guarda el puerto del realm de la ruta
                    maester.routes[route_pos].port = atoi(temp);
                    break;
                default:
                    break;
            }
            j = 0;
        }
    }

    maester.routes[route_pos].alliance_exists = 0;
    maester.routes[route_pos].active = 1;
    maester.routes[route_pos].timed_out = 0;

    free(temp);

    return maester;
}

int readConfigFile (Maester* maester, char* filename) {
    int fd;                     // Entero que guarda un file descriptor
    int end_of_file = 0;        // Flag que representa que se ha llegado al final del fichero
    int route_count = 0;        // Contador que cuenta a cuantas rutas se conecta el maester
    int error = 0;              // Entero que se devuelve y representa si ha habido un error en lectura
    char* buffer;               // Buffer que se utiliza para leer cada linea

    fd = open(filename, O_RDONLY);

    // Si se ha leido el fichero
    if (fd < 0) {
        showText("\nThere was an error while reading the configuration of a Maester.\n");
        error = ERROR_CONFIG_FILE;
    } else {
        // Guarda Realm Name
        buffer = readUntil(fd, '\n');
        maester->realm_name = malloc(sizeof(char) * (strlen(buffer) + 1));
        removeAmpersand(&buffer);
        strcpy(maester->realm_name, buffer);
        free(buffer);

        // Guarda Path
        buffer = readUntil(fd, '\n');
        maester->path = malloc(sizeof(char) * (strlen(buffer) + 1));
        strcpy(maester->path, buffer);
        free(buffer);

        // Guarda Number of Envoys
        buffer = readUntil(fd, '\n');
        maester->num_envoys = atoi(buffer);
        free(buffer);

        // Guarda IP
        buffer = readUntil(fd, '\n');
        inet_pton(AF_INET, buffer, &maester->ip);
        free(buffer);

        // Guarda Port
        buffer = readUntil(fd, '\n');
        maester->port = atoi(buffer);
        free(buffer);

        // Guarda Routes Label
        buffer = readUntil(fd, '\n');
        free(buffer);

        
        // Guarda Routes
        maester->routes = malloc(sizeof(Route));

        // Recorre el fichero hasta llegar al final para guardar las rutas
        while (!end_of_file) {
            buffer = readUntil(fd, '\n');
            if (buffer != NULL) {
                if (route_count != 0) {
                    maester->routes = realloc(maester->routes, sizeof(Route) * (route_count + 1));
                }
                *maester = decomposeRoute(*maester, buffer, route_count);
                free(buffer);
                route_count++;
            } else {
                end_of_file = 1;
            }
        }

        maester->num_routes = route_count;

        free(buffer);
        
        close(fd);
    }

    return error;
}