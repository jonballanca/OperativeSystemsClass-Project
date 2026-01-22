#include "printing.h"

void emptyStringError () {
    int printing;       // Variable de tipo entero que guarda el número de bytes para escribir por la termina 
    char* buffer;       // Buffer que guarda un string temporalmente para poder convertirlo a mayúsculas

    printing = asprintf(&buffer, RED BOLD "You enterd an empty string as a command. Please try again.%s\n", RESET);
    write(1, buffer, printing);
    free(buffer);
}

void showAvailableProducts (Inventory inventory) {
    int i = 0;              // Contador
    int printing = 0;      // Entero que guarda el número de los bytes para escribir   
    char* buffer;           // Buffer que guarda lo que se quiere escribir por la terminal

    showText("Available products: ");

    for (i = 0; i < inventory.product_count; i++) {
        printing = asprintf(&buffer, "%s", inventory.products[i].name);
        write(1, buffer, printing);
        free(buffer);

        if (i == (inventory.product_count - 1)) {
            showText(".\n");
        } else {
            showText(", ");
        }
    }
}

void showProtocolAsString(uint8_t protocol) {
    switch (protocol) {
        case ALLIANCE_REQUEST_TYPE:
            printBold("PLEDGE");
        break;
        case SIGIL_TRANSFER_TYPE:
            printBold("SIGIL");
        break;
        case LIST_PRODUCT_REQUEST_TYPE:
            printBold("LIST_PRODUCT");
        break;
        case ORDER_REQUEST_TYPE:
            printBold("ORDER_REQUEST");
        break;
        case ORDER_TRANSFER_TYPE:
            printBold("ORDER_TRANSFER");
        break;
        case DISCONNECTION_TYPE:
            printBold("DISCONNECTION");
        break;
        case NACK_TYPE:
            printBold("INVALID_FORMAT");
        break;
        default:
        break;
    }
}

void showHopMessage (char* destination_name, Protocol protocol, struct sockaddr_in origin, int error, Maester maester, int bytes_sent, int total_bytes, int previous_bytes) {
    int i = 0;
    int percentage = 0;
    char *buffer = NULL;

    if (error != -1 ){
        if ((protocol.type == SIGIL_TRANSFER_TYPE && previous_bytes == 0) || protocol.type != SIGIL_TRANSFER_TYPE) {  
            showText("\n>>> Received hop: ");

            for (i = 0; (i < maester.num_routes); i++) {
                if (memcmp(&maester.routes[i].ip, &origin, sizeof(struct sockaddr_in)) == 0 ){
                    asprintf(&buffer, "%s %s %s\n", maester.routes[i].realm_name, FLECHA, destination_name);
                    showText(buffer);
                    free(buffer);
                    buffer = NULL;
                }

            }

            //Parte que se envia a default
            asprintf(&buffer, RESET "Found route: %s%s %s %s:%d", BOLD, destination_name, FLECHA, inet_ntoa(maester.routes[error].ip), maester.routes[error].port);
            showText(buffer);
            free(buffer);
            buffer = NULL;
        }

        if (protocol.type != SIGIL_TRANSFER_TYPE) {
            printReset("\nForwarding ");
            showProtocolAsString(protocol.type);
            printReset(" request...\n");
            printBold("$ ");
        }

        if (protocol.type == SIGIL_TRANSFER_TYPE) {
            if (total_bytes > 0) {
                if (previous_bytes > 0) {
                    write(1, "\033[A", 3);
                }

                write(1, "\033[2K\r", 5);

                percentage = (bytes_sent * 100) / total_bytes;
                printReset("\rForwarding ");
                showProtocolAsString(protocol.type);
                asprintf(&buffer, RESET " request...\t\t %d / %d bytes (%d%%)\n", bytes_sent, total_bytes, percentage);
                write(1, buffer, strlen(buffer));
                free(buffer);
                buffer = NULL;
                printBold("$ ");
            }
        }
        //printBold("\n$ ");

    } else {
        //No se ha encontrado ni ruta ni default
        //showText("Route not found\n");

    }
}

void showInitializedMaester (Maester maester) {
    int printing = 0;                   // Variable que guarda los bytes para escribir por la terminal
    char *buffer = NULL;

    printing = asprintf(&buffer, "%s initialized. The board is set.\n", maester.realm_name);
    write(1, buffer, printing);
    free(buffer);
    buffer = NULL;
}

void printAllianceRequest (char *realm_name) {
    char *buffer = NULL;

    asprintf(&buffer, RESET "\nAlliance request received from %s.\n%s$ ", realm_name, BOLD);
    write(1, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;
}

void printOrderRequest (char *realm_name) {
    char *buffer = NULL;

    asprintf(&buffer, RESET "\nOrder request received from %s.\n%s$ ", realm_name, BOLD);
    write(1, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;
}

void printSuccessfulAlliance (char *realm_name) {
    char *buffer = NULL;
    
    asprintf(&buffer, RESET "\n>>>\tAlliance with %s forged successfully!\n%s", realm_name, BOLD);
    write(1, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;
}

void printFailedAlliance (char *realm_name) {
    char *buffer = NULL;

    asprintf(&buffer, RESET "\n>>>\tAlliance with %s was rejected!\n%s", realm_name, BOLD);
    write(1, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;
}

void printReset (char *text) {
    char *buffer = NULL;

    asprintf(&buffer, RESET "%s%s", text, BOLD);
    write(1, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;
}

void printBold (char *text) {
    char *buffer = NULL;

    asprintf(&buffer, RESET "%s%s", BOLD, text);
    write(1, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;
}

void printDisconnection (char *realm_name) {
    char *buffer = NULL;

    asprintf(&buffer, RESET "\n>>>\tReceived disconnection from your ally realm %s.\n%s", realm_name, BOLD);
    write(1, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;
}

void printIpAddress (struct in_addr ip) {
    char ip_string[INET_ADDRSTRLEN];
    char *buffer = NULL;

    inet_ntop(AF_INET, &ip, ip_string, sizeof(ip_string));

    asprintf(&buffer, RESET "Ip: %s\n%s", ip_string, BOLD);
    write(1, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;
}

void printRejectReason (char *reason) {
    char *buffer = NULL;

    asprintf(&buffer, RESET "%s\n%s", reason, BOLD);
    write(1, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;
}

void printListProductsRequest (char *realm_name) {
    char *buffer = NULL;

    asprintf(&buffer, RESET ">>> LIST PRODUCTS request from %s.\n%s", realm_name, BOLD);
    write(1, buffer, strlen(buffer));
    free(buffer);
    buffer = NULL;
}