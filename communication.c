#include "communication.h"

void initializeReception (ReceptionInfo *reception, Inventory inventory, char *argument) {
    reception->inventory = inventory;
    reception->inventory_file_name = malloc(sizeof(char) * (strlen(argument) + 1));
    reception->sigil_file_path = NULL;
    reception->trade_file_path = NULL;
    reception->list_products_rl = NULL;
    reception->pledge_flag = 0;
    reception->pledge_pending = 0;
    reception->receiving_file = 0;
    reception->sending_file = 0;
    reception->send_listproducts = 0;
    reception->send_sigil = 0;
    reception->send_trade = 0;
    reception->bytes_sent = 0;
    reception->bytes_received = 0;
    reception->realm_name_sending = NULL;
    reception->file_received = NULL;
    strcpy(reception->inventory_file_name, argument);

    reception->hop_bytes_sent = 0;
    reception->hop_total_bytes = 0;

    reception->pledge_status.status_count = 0;
    reception->pledge_status.pledges = NULL;

    reception->realms_stock.inventory.product_count = 0;
    reception->realms_stock.inventory.products = NULL;
    reception->realms_stock.realm_name = NULL;

    reception->response_confirmation = 0;
    reception->pledge_decision = NULL;
    reception->pledge_realm_name = NULL;

    reception->header_md5sum = NULL;

    reception->fd_file = -1;

    pthread_mutex_init(&reception->pledge_status_mutex, NULL);
}

int connectFileTranfer (char* destination_name, Maester maester) {
    int error = -1;
    int i = 0;
    int route_exists = 0;
    int default_pos = -1;

    char *buffer;
    char *buffer2;

    for (i = 0; (i < maester.num_routes) && !route_exists; i++) {

        buffer = (char*)malloc(sizeof(char)*(strlen(maester.routes[i].realm_name) + 1));
        strcpy(buffer, maester.routes[i].realm_name);

        buffer2 = (char*)malloc(sizeof(char)*(strlen(destination_name) + 1));
        strcpy(buffer2, destination_name);
        
        convertToCapitalLetters(buffer);
        convertToCapitalLetters(buffer2);

        if (!strcmp(buffer, buffer2)) {
            free(buffer);
            buffer = NULL;
            buffer = realloc(buffer, sizeof(char) * INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &maester.routes[i].ip, buffer, INET_ADDRSTRLEN);
            
            if ((strcmp("0.0.0.0", buffer)) /*&& (maester.routes[i].active != 0)*/) {
                route_exists = 1;
                error = openClientConn(maester.routes[i].ip, maester.routes[i].port);
            }
        } else if (!strcmp(maester.routes[i].realm_name, "DEFAULT")) {
            default_pos = i;
        }

        free(buffer);
        buffer = NULL;
        free(buffer2);
        buffer2 = NULL;
    }

    if (!route_exists) {
        error = openClientConn(maester.routes[default_pos].ip, maester.routes[default_pos].port);
    }

    return error;
}

int sendProtocol(struct in_addr ip, int port, char *protocol) {
    //LO QUE TENDREMOS QUE HACER CON EL SOCKET
    int fd = 0;

    fd = openClientConn (ip, port);

    if (fd != -1) {
        write(fd, protocol, PROTOCOL_SIZE);
        close(fd);
        return 1;
    }

    return -1;
}

int sendToRoute (char* destination_name, char *protocol, Maester maester) {
    int error = -1;
    int i = 0;
    int route_exists = 0;
    int default_pos = -1;

    char *buffer;
    char *buffer2;

    for (i = 0; (i < maester.num_routes) && !route_exists; i++) {

        buffer = (char*)malloc(sizeof(char)*(strlen(maester.routes[i].realm_name) + 1));
        strcpy(buffer, maester.routes[i].realm_name);

        buffer2 = (char*)malloc(sizeof(char)*(strlen(destination_name) + 1));
        strcpy(buffer2, destination_name);
        
        convertToCapitalLetters(buffer);
        convertToCapitalLetters(buffer2);

        if (!strcmp(buffer, buffer2)) {
            free(buffer);
            buffer = NULL;
            buffer = realloc(buffer, sizeof(char) * INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &maester.routes[i].ip, buffer, INET_ADDRSTRLEN);
            
            if ((strcmp("0.0.0.0", buffer)) /*&& (maester.routes[i].active != 0)*/) {
                route_exists = 1;
                error = sendProtocol(maester.routes[i].ip, maester.routes[i].port, protocol);
            }
        } else if (!strcmp(maester.routes[i].realm_name, "DEFAULT")) {
            default_pos = i;
        }

        free(buffer);
        buffer = NULL;
        free(buffer2);
        buffer2 = NULL;
    }

    if (!route_exists) {
        error = sendProtocol(maester.routes[default_pos].ip, maester.routes[default_pos].port, protocol);
    }

    return error;
}

void addAlliance (char *realm_name, Maester *maester, struct in_addr ip, int port) {
    int i = 0;
    char *upper_case1;
    char *upper_case2;

    for (i = 0; i < maester->num_routes; i++) {
        upper_case1 = (char*)malloc(sizeof(char) * (strlen(maester->routes[i].realm_name) + 1));
        upper_case2 = (char*)malloc(sizeof(char) * (strlen(realm_name) + 1));

        strcpy(upper_case1, maester->routes[i].realm_name);
        convertToCapitalLetters(upper_case1);

        strcpy(upper_case2, realm_name);
        convertToCapitalLetters(upper_case2);

        if (!strcmp(upper_case1, upper_case2)) {
            maester->routes[i].alliance_exists = 1;
            maester->routes[i].ip = ip;
            maester->routes[i].port = port;
        }

        free(upper_case1);
    }

}

int searchRoutePos (char* destination_name, Maester maester) {
    int i = 0;
    int route_exists = 0;
    int route_pos = -1;

    char *buffer;

    for (i = 0; (i < maester.num_routes) && !route_exists; i++) {

        buffer = (char*)malloc(sizeof(char)*(strlen(maester.routes[i].realm_name) + 1));
        strcpy(buffer, maester.routes[i].realm_name);

        convertToCapitalLetters(buffer);

        if (!strcmp(buffer, destination_name)) {
            route_exists = 1;
            route_pos = 1;
        }

        free(buffer);
        buffer = NULL;
    }

    return route_pos;
}

void sendDisconection (Maester maester) {
    int i = 0;
    int active_allys = 0;
    int printing = 0;
    char *buffer = NULL;
    char protocol[PROTOCOL_SIZE];

    for (i = 0; i < maester.num_routes; i++) {
        if (maester.routes[i].alliance_exists && maester.routes[i].active) {
            active_allys = 1;
            commDisconnection(protocol, maester, maester.routes[i].realm_name);
            sendToRoute(maester.routes[i].realm_name, protocol, maester);
        }
    }

    if (active_allys) {
        showText("\n\n----- Disconnection sent -----");
    }

    printing = asprintf(&buffer, "\n\n%s signs off. The ravens rest.\n", maester.realm_name);
    write(1, buffer, printing);
    free(buffer);
}

void sendRejection(Maester maester, char *realm_name, char *reason) {
    char protocol[PROTOCOL_SIZE];
    struct in_addr ip;
    int port;
    char *buffer = NULL;
    int printing = 0;
    
    getIpAndPortFromRealm(maester, realm_name, &ip, &port);
    
    buffer = malloc(sizeof(char) * (strlen(reason) + 1 + strlen("REJECT")));
    strcat("REJECT&", reason);

    commOrderResponse(protocol, maester, realm_name, buffer);
    sendProtocol(ip, port, protocol);

    free(buffer);
    buffer = NULL;
    
    printing = asprintf(&buffer, "\nTrade rejected: %s\n", reason);
    write(1, buffer, printing);
    free(buffer);
    
    printBold("$ ");
}

void sendAcceptance(Maester maester, char *realm_name, char *message) {
    char protocol[PROTOCOL_SIZE];
    struct in_addr ip;
    int port;
    int printing = 0;
    
    getIpAndPortFromRealm(maester, realm_name, &ip, &port);
    
    commOrderResponse(protocol, maester, realm_name, "OK&ACCEPT");
    
    sendProtocol(ip, port, protocol);
}

void addActive (char *realm_name, Maester *maester) {
    int i = 0;
    char *upper_case;

    for (i = 0; i < maester->num_routes; i++) {
        upper_case = (char*)malloc(sizeof(char)*(strlen(maester->routes[i].realm_name) + 1));

        strcpy(upper_case, maester->routes[i].realm_name);
        convertToCapitalLetters(upper_case);

        if (!strcmp(upper_case, realm_name)) {
            maester->routes[i].active = 1;
        }

        free(upper_case);
    }
}

void addInactive (char *realm_name, Maester *maester) {
    int i = 0;
    char *upper_case;
    char *upper_case1;

    for (i = 0; i < maester->num_routes; i++) {
        upper_case = (char*)malloc(sizeof(char)*(strlen(maester->routes[i].realm_name) + 1));
        upper_case1 = (char*)malloc(sizeof(char)*(strlen(realm_name) + 1));

        strcpy(upper_case1, realm_name);
        convertToCapitalLetters(upper_case1);

        strcpy(upper_case, maester->routes[i].realm_name);
        convertToCapitalLetters(upper_case);

        if (!strcmp(upper_case, upper_case1)) {
            maester->routes[i].active = 0;
        }

        free(upper_case);
    }
}

void addRealmInventory(ReceptionInfo *reception, Inventory inventory, char *realm_name) {
    int j = 0;
    
    if (reception->realms_stock.inventory.products != NULL) {
        free(reception->realms_stock.inventory.products);
    }
    
    if (reception->realms_stock.realm_name != NULL) {
        free(reception->realms_stock.realm_name);
    }
    
    reception->realms_stock.realm_name = malloc(sizeof(char) * (strlen(realm_name) + 1));
    strcpy(reception->realms_stock.realm_name, realm_name);
    
    reception->realms_stock.inventory.product_count = inventory.product_count;
    reception->realms_stock.inventory.products = malloc(sizeof(Product) * inventory.product_count);
    
    for (j = 0; j < inventory.product_count; j++) {
        reception->realms_stock.inventory.products[j] = inventory.products[j];
    }
}

void addPledgeStatus (ReceptionInfo *reception, int status, char* realm_name) {
    int i = 0;
    int found = 0;
    char *upper_case1;
    char *upper_case2;


    pthread_mutex_lock(&reception->pledge_status_mutex);

    for (i = 0; i < reception->pledge_status.status_count; i++) {
        upper_case1 = (char*)malloc(sizeof(char) * (strlen(reception->pledge_status.pledges[i].realm_name) + 1));
        upper_case2 = (char*)malloc(sizeof(char) * (strlen(realm_name) + 1));
        
        strcpy(upper_case1, reception->pledge_status.pledges[i].realm_name);
        strcpy(upper_case2, realm_name);
        
        convertToCapitalLetters(upper_case1);
        convertToCapitalLetters(upper_case2);
        
        if (!strcmp(upper_case1, upper_case2)) {
            reception->pledge_status.pledges[i].status = status;
            found = 1;
        }
        
        free(upper_case1);
        free(upper_case2);
        
        if (found) {
            break;
        }
    }
    
    if (!found) {
        reception->pledge_status.pledges = realloc(reception->pledge_status.pledges, sizeof(Pledge) * (reception->pledge_status.status_count + 1));
        
        reception->pledge_status.pledges[reception->pledge_status.status_count].realm_name = malloc(sizeof(char) * (strlen(realm_name) + 1));
        strcpy(reception->pledge_status.pledges[reception->pledge_status.status_count].realm_name, realm_name);
        reception->pledge_status.pledges[reception->pledge_status.status_count].status = status;
        
        reception->pledge_status.status_count++;
    }

    pthread_mutex_unlock(&reception->pledge_status_mutex);
}

void processAllianceResponse (char data[], char** pledge_decision, char** realm_name) {
    int i = 0;
    int j = 0;

    *pledge_decision = NULL;
    *realm_name = NULL;

    while (data[i] != '&') {
        *pledge_decision = realloc(*pledge_decision, sizeof(char) * (i + 1));
        (*pledge_decision)[i] = data[i];
        i++;
    }
    *pledge_decision = realloc(*pledge_decision, sizeof(char) * (i + 1));
    (*pledge_decision)[i] = '\0';

    i++;

    while (data[i] != '\0') {
        *realm_name = realloc(*realm_name, sizeof(char) * (j + 1));
        (*realm_name)[j++] = data[i++];
    }
    *realm_name = realloc(*realm_name, sizeof(char) * (j + 1));
    (*realm_name)[j] = '\0';
}

void sendNACK () {

}

void sendACK () {

}

int getIfResponseType (Protocol protocol) {
    int response_type = 1;

    switch (protocol.type) {
        case ALLIANCE_REQUEST_TYPE:
            response_type = 0;
            break;
        case ALLIANCE_RESPOND_TYPE:
            response_type = 0;
            break;
        case SIGIL_TRANSFER_TYPE:
            response_type = 0;
            break;
        case LIST_PRODUCT_REQUEST_TYPE:
            response_type = 0;
            break;
        case ORDER_REQUEST_TYPE:
            response_type = 0;
            break;
        case ORDER_TRANSFER_TYPE:
            response_type = 0;
            break;
        case DISCONNECTION_TYPE:
            response_type = 0;
            break;
        default:
            break;
    }

    return response_type;
}

int checkDestinationRequest (char protocol_string[PROTOCOL_SIZE], Protocol protocol, struct sockaddr_in client_addr, Maester *maester, ReceptionInfo **reception) {
    int checksum = 0;
    int error = 0;
    int previous_bytes_sent = 0;
    char *destination_name;
    char *upper_case_one;
    char *upper_case_two;
    char *buffer = NULL;

    if (getIfResponseType(protocol)) {
        error = 0;
        return error;
    }

    checksum = calculateCheckSum(protocol);

    if (!checksum) {
        sendNACK();
        error = 1;
        return error;
    }

    arrayToString(protocol.destination, &destination_name);
    upper_case_one = (char*)malloc(sizeof(char)*(strlen(maester->realm_name) + 1));
    upper_case_two = (char*)malloc(sizeof(char)*(strlen(destination_name) + 1));
    strcpy(upper_case_one, maester->realm_name);
    strcpy(upper_case_two, destination_name);
    convertToCapitalLetters(upper_case_one);
    convertToCapitalLetters(upper_case_two);

    if (!strcmp(upper_case_one, upper_case_two)) {
        // El destino es este maester
        sendACK(protocol);
    } else {
        // El destino no es este maester
        if (protocol.type == ALLIANCE_REQUEST_TYPE) {
            getWordFromProtocol(protocol.data, SIGIL_FILESIZE_WORD, &buffer);
            (*reception)->hop_total_bytes = atoi(buffer);
            free(buffer);
            buffer = NULL;
            (*reception)->hop_bytes_sent = 0;
        } else if (protocol.type == SIGIL_TRANSFER_TYPE) {
            previous_bytes_sent = (*reception)->hop_bytes_sent;
            (*reception)->hop_bytes_sent += protocol.data_length;
        }
        error = sendToRoute(destination_name, protocol_string, *maester);
        showHopMessage(destination_name, protocol, client_addr, error, *maester, (*reception)->hop_bytes_sent, (*reception)->hop_total_bytes, previous_bytes_sent);
        error = -1;
        if ((protocol.type == SIGIL_TRANSFER_TYPE) && ((*reception)->hop_bytes_sent >= (*reception)->hop_total_bytes)) {
            (*reception)->hop_bytes_sent = 0;
            (*reception)->hop_total_bytes = 0;
        }
    }

    free(upper_case_one);
    free(upper_case_two);
    return error;
}

void getRealmNameFromIp (char **realm_name, struct in_addr ip, int port, Maester *maester) {
    int i = 0;

    *realm_name = NULL;

    for (i = 0; i < maester->num_routes; i++) {
        if ((!memcmp(&ip, &maester->routes[i].ip, sizeof(struct in_addr))) && (port == maester->routes[i].port)) {
            *realm_name = malloc(sizeof(char) * (strlen(maester->routes[i].realm_name) + 1));
            strcpy(*realm_name, maester->routes[i].realm_name);
        }
    }
}

void getIpAndPortFromRealm (Maester maester, char *realm_name, struct in_addr *ip, int *port) {
    int i = 0;
    int found = 0;

    char *buffer1;
    char *buffer2;

    for (i = 0; (i < maester.num_routes) && !found; i++) {

        buffer1 = (char*)malloc(sizeof(char)*(strlen(maester.routes[i].realm_name) + 1));
        strcpy(buffer1, maester.routes[i].realm_name);
        
        buffer2 = (char*)malloc(sizeof(char)*(strlen(realm_name) + 1));
        strcpy(buffer2, realm_name);

        convertToCapitalLetters(buffer1);
        convertToCapitalLetters(buffer2);

        if (!strcmp(buffer1, buffer2)) {
            *ip = maester.routes[i].ip;
            *port = maester.routes[i].port;

            found = 1;
        }

        free(buffer1);
        buffer1 = NULL;
        free(buffer2);
        buffer2 = NULL;
    }
}

void addReceivedData (char** destination, int* total_bytes, char* new_data, int bytes_received) {
    int i = 0;
    int old_size;
    int total_size;
    
    if (destination[0] == NULL) {
        old_size = 0;
        total_size = bytes_received;
        destination[0] = malloc(sizeof(char) * bytes_received);
    } else {
        old_size = total_bytes[0];
        total_size = old_size + bytes_received;
        destination[0] = realloc(destination[0], sizeof(char) * total_size);
    }
    
    for (i = 0; i < bytes_received; i++) {
        destination[0][old_size + i] = new_data[i];
    }
    
    total_bytes[0] = total_size;
}

int checkTradeInventory(ReceptionInfo *reception, char *realm_name) {
    int found = 0;
    char *upper_case1 = NULL;
    char *upper_case2 = NULL;
    
    if (reception->realms_stock.realm_name == NULL) {
        found = 0;
        return found;
    }
    
    upper_case1 = malloc(sizeof(char) * (strlen(reception->realms_stock.realm_name) + 1));
    upper_case2 = malloc(sizeof(char) * (strlen(realm_name) + 1));
    
    strcpy(upper_case1, reception->realms_stock.realm_name);
    strcpy(upper_case2, realm_name);
    
    convertToCapitalLetters(upper_case1);
    convertToCapitalLetters(upper_case2);
    
    if (!strcmp(upper_case1, upper_case2)) {
        found = 1;
    } else {
        found = 0;
    }
    
    free(upper_case1);
    free(upper_case2);
    
    return found;
}

Inventory getRealmInventory(ReceptionInfo *reception, char *realm_name) {
    int found = 0;
    char *upper_case1 = NULL;
    char *upper_case2 = NULL;
    Inventory empty_inventory;
    
    empty_inventory.products = NULL;
    empty_inventory.product_count = 0;
    
    if (reception->realms_stock.realm_name == NULL) {
        return empty_inventory;
    }
    
    upper_case1 = malloc(sizeof(char) * (strlen(reception->realms_stock.realm_name) + 1));
    upper_case2 = malloc(sizeof(char) * (strlen(realm_name) + 1));
    
    strcpy(upper_case1, reception->realms_stock.realm_name);
    strcpy(upper_case2, realm_name);
    
    convertToCapitalLetters(upper_case1);
    convertToCapitalLetters(upper_case2);
    
    if (!strcmp(upper_case1, upper_case2)) {
        found = 1;
    }
    
    free(upper_case1);
    free(upper_case2);
    
    if (found) {
        return reception->realms_stock.inventory;
    }
    
    return empty_inventory;
}

void handleInventoryReception (ReceptionInfo *reception, char* realm_name) {
    Inventory inventory;

    inventory.products = NULL;
    inventory.product_count = 0;
    
    if (!loadInventoryFromData(&inventory, (*reception).file_received, (*reception).bytes_received)) {
        addRealmInventory(reception, inventory, realm_name);
        listProducts(inventory);
        
        if (inventory.products != NULL) {
            free(inventory.products);
            inventory.products = NULL;
        }
    } else {
        printReset("\nError loading inventory from received data!\n");
    }
}

void handleTradeReception (ReceptionInfo *reception, Maester maester, Protocol protocol) {
    Inventory inventory;
    Protocol protocol_send;
    char *realm_name;
    struct in_addr ip;
    int port;
    int i = 0;
    int position = 0;
    int all_valid = 1;

    inventory.products = NULL;
    inventory.product_count = 0;
    
    if (!loadTradeFromData(&inventory, (*reception).file_received, (*reception).bytes_received)) {        
        for (i = 0; i < inventory.product_count; i++) {
            position = getProductPositionByName(inventory.products[i].name, (*reception).inventory);
            if (position == -1) {
                getIpAndPortFromProtocol(protocol.origin, &ip, &port);
                getRealmNameFromIp(&realm_name, ip, port, &maester);
                sendRejection(maester, realm_name, "One of the listed products was not found!");
                all_valid = 0;
                break;
            }
            if (inventory.products[i].amount > (*reception).inventory.products[position].amount) {
                getIpAndPortFromProtocol(protocol.origin, &ip, &port);
                getRealmNameFromIp(&realm_name, ip, port, &maester);
                sendRejection(maester, realm_name, "One of the listed products had a higher amount than what we have in stock.");
                all_valid = 0;
                break;
            }
        }
        
        if (all_valid) {
            getIpAndPortFromProtocol(protocol.origin, &ip, &port);
            getRealmNameFromIp(&realm_name, ip, port, &maester);
            
            for (i = 0; i < inventory.product_count; i++) {
                position = getProductPositionByName(inventory.products[i].name, (*reception).inventory);
                if (position != -1) {
                    (*reception).inventory.products[position].amount -= inventory.products[i].amount;
                }
            }

            writeInventoryFile((*reception).inventory, (*reception).inventory_file_name);

            printReset("[DEBUG TRADE] Order processed successfully. Stock updated.\n");
            sendAcceptance(maester, realm_name, "Trade accepted!");
        }

        if (inventory.products != NULL) {
            free(inventory.products);
            inventory.products = NULL;
        }
        
        if (inventory.products != NULL) {
            free(inventory.products);
            inventory.products = NULL;
        }
    } else {
        printReset("\nError loading inventory from received data!\n");
    }
}

void receiveFileData (Protocol protocol, ReceptionInfo **reception, Maester *maester) {
    struct in_addr ip;
    int port;
    char* realm_name;
    char calculated_md5sum[33];
    char protocol_send[PROTOCOL_SIZE];

    addReceivedData(&(*reception)->file_received, &(*reception)->bytes_received, protocol.data, protocol.data_length);
    
    if ((*reception)->bytes_received >= (*reception)->communication_filesize) {
        printDoneReceiving((*reception)->receiving_file);

        getIpAndPortFromProtocol(protocol.origin, &ip, &port);
        getRealmNameFromIp(&realm_name, ip, port, maester);

        calculateReceivingMD5SUM((*reception)->file_received, (*reception)->bytes_received, calculated_md5sum);

        char *buffer;
        asprintf(&buffer, "\n\nCalculated md5sum checksum: %s\n\n", calculated_md5sum);
        write(1, buffer, strlen(buffer));
        free(buffer);

        asprintf(&buffer, "\n\nReceived md5sum checksum: %s\n\n", (*reception)->header_md5sum);
        write(1, buffer, strlen(buffer));
        free(buffer);
        
        if (!strcmp(calculated_md5sum, (*reception)->header_md5sum)) {
            commMd5Ack(protocol_send, *maester, realm_name, MD5SUM_SUCCESSFUL);
            if ((*reception)->receiving_file == SENDING_INVENTORY) {
                handleInventoryReception(*reception, realm_name);
                printBold("$ ");
            } else if ((*reception)->receiving_file == SENDING_SIGIL) {
                // Mirar md5sum y si está bien, activar el flag
                // Activar una flag
                //(*reception)->pledge_pending = 1;
                addPledgeStatus(*reception, PENDING, realm_name);
                printBold("$ ");
            } else if ((*reception)->receiving_file == SENDING_TRADE) {
                handleTradeReception(*reception, *maester, protocol);
                printBold("$ ");
            }
        } else {
            commMd5Ack(protocol_send, *maester, realm_name, MD5SUM_FAILED);
        }
        sendProtocol(ip, port, protocol_send);

        free(realm_name);
        
        (*reception)->receiving_file = 0;
        (*reception)->bytes_received = 0;
        if ((*reception)->file_received != NULL) {
            free((*reception)->file_received);
            (*reception)->file_received = NULL;
        }
    }
}

void handleHeaderReception (Protocol protocol, ReceptionInfo **reception, Maester maester, int receiving_type, int fd_client) {
    char protocol_send[PROTOCOL_SIZE];
    struct in_addr ip;
    int port;
    int filesize_pos = 0;
    char* word = NULL;

    switch (receiving_type) {
        case SENDING_INVENTORY:
            filesize_pos = LIST_PROD_FILESIZE_WORD;
            getWordFromProtocol(protocol.data, LIST_PROD_MD5SUM_WORD, &word);
            break;
        case SENDING_SIGIL:
            filesize_pos = SIGIL_FILESIZE_WORD;
            getWordFromProtocol(protocol.data, SIGIL_MD5SUM_WORD, &word);
            break;
        case SENDING_TRADE:
            filesize_pos = TRADE_FILESIZE_WORD;
            getWordFromProtocol(protocol.data, ORDER_MD5SUM_WORD, &word);
            break;
        default:
            break;
    }

    (*reception)->header_md5sum = realloc((*reception)->header_md5sum, sizeof(char) * (strlen(word) + 1));
    strcpy((*reception)->header_md5sum, word);
    free(word);
    word = NULL;

    getIpAndPortFromProtocol(protocol.origin, &ip, &port);

    if ((*reception)->receiving_file) {
        commAckFile(protocol_send, maester, maester.realm_name, "KO");
        sendProtocol(ip, port, protocol_send);
        //sendToRoute(protocol.destination, protocol, maester);
        return;
    }

    if ((*reception)->file_received != NULL) {
        free((*reception)->file_received);
        (*reception)->file_received = NULL;
    }
    (*reception)->bytes_received = 0;

    commAckFile(protocol_send, maester, maester.realm_name, "OK");
    sendProtocol(ip, port, protocol_send);

    (*reception)->fd_file = fd_client;
    (*reception)->receiving_file = receiving_type;
    free(word);
    word = NULL;

    getWordFromProtocol(protocol.data, filesize_pos, &word);
    (*reception)->communication_filesize = atoi(word);

    printExpectedBytes(receiving_type, word);
    free(word);
    word = NULL;

    printBold("$ ");

    free(word);
}

void handleAckConfirmation (Protocol protocol, ReceptionInfo **reception) {
    char *word = NULL;

    getWordFromProtocol(protocol.data, REALM_NAME_WORD, &word);

    switch ((*reception)->sending_file) {
        case SENDING_INVENTORY:
            // Hay que enviar fichero de list product
            printReset("[DEBUG LIST PRODUCTS] Confirmed! Unveiling the vault of goods...\n");
            (*reception)->send_listproducts = 1;
            break;
        case SENDING_SIGIL:
            // Hay que enviar fichero de sigil
            (*reception)->send_sigil = 1;
            break;
        case SENDING_TRADE:
            // Hay que enviar fichero de trade
            (*reception)->send_trade = 1;
            break;
        default:
            break;
    }

    (*reception)->realm_name_sending = realloc((*reception)->realm_name_sending, sizeof(char) * (strlen(word) + 1));
    strcpy((*reception)->realm_name_sending, word);

    free(word);
    word = NULL;
}

void prepareSendingFile (int *file_size, int *fd, ReceptionInfo *reception, Maester maester, int sending_type) {
    char *file_size_str = NULL;

    if (sending_type == SENDING_INVENTORY) {
        *fd = open(reception->inventory_file_name, O_RDONLY);
        getFileSize(reception->inventory_file_name, &file_size_str);
    } else if (sending_type == SENDING_SIGIL) {
        *fd = open(reception->sigil_file_path, O_RDONLY);
        getFileSize(reception->sigil_file_path, &file_size_str);
    } else if (sending_type == SENDING_TRADE) {
        *fd = open(reception->trade_file_path, O_RDONLY);
        getFileSize(reception->trade_file_path, &file_size_str);
    }
    
    *file_size = atoi(file_size_str);
    free(file_size_str);
}

int checkChecksum(Protocol *protocol) {
    uint16_t calculated_checksum = 0;
    uint16_t received_checksum = 0;
    
    received_checksum = protocol->checksum;
    
    calculated_checksum = (uint16_t)calculateCheckSum(*protocol);
    
    if (calculated_checksum == received_checksum) {
        return 1;
    } else {
        return 0;
    }
}

void manageReception (ReceptionInfo **reception, Protocol *protocol, Maester *maester, int flag, int fd_client) {
    char protocol_send[PROTOCOL_SIZE];
    char* realm_name = NULL;
    char* word = NULL;
    char* pledge_decision = NULL;
    // char* mdsum;
    char *buffer = NULL;
    struct in_addr ip;
    int port;
    int correct_checksum = 0;


    uint16_t calculated_checksum;
    calculated_checksum = (uint16_t)calculateCheckSum(*protocol);

    if (!checkChecksum(protocol) && (protocol->type != NACK_TYPE)) {
        //handleIncorrectChecksum(reception, *protocol);
        correct_checksum = 1;
        protocol->type = NACK_TYPE;
    }

    switch (protocol->type) {
        //Deberiamos hacernos flags para saber que estamos esperando un mensaje 
        case ALLIANCE_REQUEST_TYPE:
            if ((*reception)->pledge_pending) {
                if ((*reception)->pledge_realm_name != NULL) {
                    free((*reception)->pledge_realm_name);
                    (*reception)->pledge_realm_name = NULL;
                }
            }

            handleHeaderReception(*protocol, reception, *maester, SENDING_SIGIL, fd_client);

            //Envia RealmName SigilName FileSize MD5SUM
            getIpAndPortFromProtocol(protocol->origin, &ip, &port);

            if ((*reception)->pledge_realm_name != NULL) {
                free((*reception)->pledge_realm_name);
                (*reception)->pledge_realm_name = NULL;
            }
            
            getWordFromProtocol(protocol->data, ALLIANCE_REALM_WORD, &(*reception)->pledge_realm_name);

            (*reception)->pledge_ip = ip;
            (*reception)->pledge_port = port;
            (*reception)->pledge_pending = 1;

            printAllianceRequest((*reception)->pledge_realm_name);
            
            free(realm_name);
            realm_name = NULL;
            break;
        case SIGIL_TRANSFER_TYPE:
            receiveFileData(*protocol, reception, maester);
            break;
    case ALLIANCE_RESPOND_TYPE:
            //d no ok
            processAllianceResponse(protocol->data, &pledge_decision, &realm_name);

            getIpAndPortFromProtocol(protocol->origin, &ip, &port);

            if (checkTimedOut(*maester, realm_name)) {
                // send NACK
                commNACK(protocol_send, maester->realm_name);
                sendProtocol(ip, port, protocol_send);

                addPledgeStatus(*reception, REJECTED, realm_name);

                changeTimedOut(realm_name, RESET_TIMED_OUT, maester);
                printReset("\n[DEBUG PLEDGE] Late pledge response received. Sending NACK...\n");
                printBold("$ ");

                if (pledge_decision != NULL) {
                    free(pledge_decision);
                    pledge_decision = NULL;
                }
                if (realm_name != NULL) {
                    free(realm_name);
                    realm_name = NULL;
                }
                break;
            }

            commAckFile(protocol_send, *maester, realm_name, "OK");
            sendProtocol(ip, port, protocol_send);

            if (!strcmp(pledge_decision, "ACCEPT")) {
                printSuccessfulAlliance(realm_name);
                addAlliance(realm_name, maester, ip, port);
                addActive(realm_name, maester);
                addPledgeStatus(*reception, ACCEPTED, realm_name);
            } else if (!strcmp(pledge_decision, "REJECT")) {
                printFailedAlliance(realm_name);
                addPledgeStatus(*reception, REJECTED, realm_name);
            }

            (*reception)->pledge_flag = 0;
            
            free(realm_name);
            realm_name = NULL;
            break;
        case LIST_PRODUCT_REQUEST_TYPE:
            getIpAndPortFromProtocol(protocol->origin, &ip, &port);

            getDataString(&realm_name, protocol->data);

            printListProductsRequest(realm_name);

            commProductsResponse(protocol_send, *maester, realm_name, (*reception)->inventory_file_name);
            sendProtocol(ip, port, protocol_send);

            (*reception)->sending_file = SENDING_INVENTORY;

            printReset("[DEBUG LIST PRODUCTS] Response sent!\n");
            //printBold("$ ");

            //protocol_send = commProductTransfer(*maester, realm_name, (*reception)->inventory_file_name);
            //sendProtocol(ip, port, protocol_send);

            //printReset("\nFile transfer sent!\n");
            //printBold("$ ");

            if (realm_name != NULL) {
                free(realm_name);
                //realm_name = NULL;
            }
            break;
        case LIST_PRODUCT_RESPOND_TYPE:
            //d no ok
            printReset("\n[DEBUG LIST PRODUCTS] Response received!");
            //printBold("$ ");

            handleHeaderReception(*protocol, reception, *maester, SENDING_INVENTORY, fd_client);

            free(word);
            word = NULL;

            break;
        case LIST_PRODUCT_TRANSFER_TYPE:
            receiveFileData(*protocol, reception, maester);
            //printBold("$ ");
            break;
        case ORDER_REQUEST_TYPE:
            handleHeaderReception(*protocol, reception, *maester, SENDING_TRADE, fd_client);

            getIpAndPortFromProtocol(protocol->origin, &ip, &port);
            getRealmNameFromIp(&realm_name, ip, port, maester);

            printOrderRequest(realm_name);
            
            free(realm_name);
            //realm_name = NULL;
            break;
        case ORDER_TRANSFER_TYPE:
            receiveFileData(*protocol, reception, maester);
            break;
        case ORDER_RESPOND_TYPE:
            //d no ok
            getWordFromProtocol(protocol->data, ORDER_CONFIRMATION_WORD, &word);
            if (!strcmp(word, "OK")) {
                printReset("\n[DEBUG TRADE] The order was accepted!\n");
                handleAckConfirmation(*protocol, reception);
            } else {
                free(word);
                word = NULL;
                getWordFromProtocol(protocol->data, ORDER_REASON_WORD, &word);
                printReset("\n[DEBUG TRADE] The order was rejected because:\n");
                printRejectReason(word);
                (*reception)->sending_file = 0;
            }

            if ((*reception)->trade_file_path != NULL) {
                free((*reception)->trade_file_path);
                (*reception)->trade_file_path = NULL;
            }

            (*reception)->send_trade = 0;

            printBold("$ ");

            free(word);
            word = NULL;
            break;
        case DISCONNECTION_TYPE:
            //D ok pero alianza
            getIpAndPortFromProtocol(protocol->origin, &ip, &port);
            getRealmNameFromIp(&realm_name, ip, port, maester);
            printDisconnection(realm_name);
            printBold("$ ");
            addInactive(realm_name, maester);
            free(realm_name);
            //realm_name = NULL;
            break;
        case UNKNOWN_REALM_TYPE:
            //D no ok
            break;
        case NO_ALLIANCE_TYPE:
            //D no ok
            break;
        case PING_TYPE:

            break;
        case ACK_FILE_TYPE:
            //D no ok
            getWordFromProtocol(protocol->data, ACK_CONFIRMATION_WORD, &word);
            if (!strcmp(word, "OK")) {
                if ((*reception)->response_confirmation) {
                    getIpAndPortFromProtocol(protocol->origin, &ip, &port);
                    (*reception)->response_confirmation = 0;
                    if (!strcmp((*reception)->pledge_decision, "ACCEPT")) {
                        printSuccessfulAlliance((*reception)->pledge_realm_name);
                        addAlliance((*reception)->pledge_realm_name, maester, ip, port);
                        addActive((*reception)->pledge_realm_name, maester);
                        addPledgeStatus(*reception, ACCEPTED, (*reception)->pledge_realm_name);
                    } else if (!strcmp((*reception)->pledge_decision, "REJECT")) {
                        printFailedAlliance((*reception)->pledge_realm_name);
                        addPledgeStatus(*reception, REJECTED, (*reception)->pledge_realm_name);
                    }

                    free((*reception)->pledge_decision);
                    (*reception)->pledge_decision = NULL;

                    free((*reception)->pledge_realm_name);
                    (*reception)->pledge_realm_name = NULL;
                    (*reception)->pledge_flag = 0;
                    printBold("$ ");
                } else {
                    handleAckConfirmation(*protocol, reception);
                }
            } else {
                printReset("\nThe confirmation was not successful. Aborting sending the file...\n");
                (*reception)->sending_file = 0;
                printBold("$ ");
            }

            free(word);
            word = NULL;
            break;
        case CHECKSUM_TYPE:
            //D no ok
            printReset("\n[DEBUG PROTOCOL] MD5SUM confirmation received!\n");
            printBold("$ ");

            getWordFromProtocol(protocol->data, MD5SUM_CONFIRMATION_WORD, &word);
            if (!strcmp(word, "CHECK_OK")) {
                printReset("\n[DEBUG PROTOCOL] Successful!\n");
                if ((*reception)->sending_file == SENDING_SIGIL) {
                    getIpAndPortFromProtocol(protocol->origin, &ip, &port);
                    getRealmNameFromIp(&realm_name, ip, port, maester);
                    addPledgeStatus(*reception, PENDING, realm_name);
                    free(realm_name);
                }
            } else if (!strcmp(word, "CHECK_KO")) {
                printReset("\n[DEBUG PROTOCOL] The MD5SUM checksum failed.\n");
            } else {
                // Send 0x69
            }

            free(word);
            word = NULL;

            if ((*reception)->sending_file != SENDING_TRADE) {
                (*reception)->sending_file = 0;
            }
            break;

        case NACK_TYPE:
            //D no ok
            if ((*reception)->response_confirmation) {
                printReset("\n[DEBUG PLEDGE] Too late to respond to this pledge! Alliance failed.\n");
                (*reception)->response_confirmation = 0;
                free((*reception)->pledge_decision);
                (*reception)->pledge_decision = NULL;
                free((*reception)->pledge_realm_name);
                (*reception)->pledge_realm_name = NULL;

                printBold("$ ");
            } else if (!correct_checksum) {
                printReset("\n[DEBUG PROTOCOL] Error detecting protocol frame.\n");
            } else {
                printReset("\n[DEBUG PROTOCOL] Incorrect checksum detected.\n");
            }
            break;
        default:
            getIpAndPortFromProtocol(protocol->origin, &ip, &port);
            commNACK(protocol_send, maester->realm_name);
            sendProtocol(ip, port, protocol_send);
            break;
    }
}

