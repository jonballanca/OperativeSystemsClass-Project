#include "protocols.h"

int openClientConn (struct in_addr ip, int port) {
    struct sockaddr_in s_addr;          //Creamos la structura
    int fd_socket = -1;

    fd_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //AF_INET -> IPV4, SOCK_STREAM  ,IPPROTO_TCP Protocolo tcp
    if (fd_socket < 0) {
        showText(ERR_SOCKET);
        return -1;
    }

    memset(&s_addr, 0, sizeof(s_addr));         //Llenamos la struct de 0
    s_addr.sin_family = AF_INET;                //Indicas la familia
    s_addr.sin_port = htons(port);              //Pasarle la función biggendian
    s_addr.sin_addr = ip;

    if (connect(fd_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0) { 
        close(fd_socket);
        return -1;
    }

    return fd_socket;
}

// Completado
int createSocket (struct in_addr ip, int port) {
    int fd = -1;
    int bind_state = 0;
    struct sockaddr_in server_addr;

    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (!(fd < 0)) {
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        server_addr.sin_addr = ip;

        bind_state = bind(fd, (void*) &server_addr, sizeof(server_addr));
        
        if (!(bind_state < 0)) {
            listen(fd, MAX_CONNECTIONS);
        } else {
            showText("Error binding to socket.\n");
            close(fd);
            fd = -1;
        }
        
    } else {
        showText("Error openning socket.\n");
        fd = -1;
    }

    return fd;
}

// Completado
void numberToString (int number, char **string) {
    asprintf(string, "%d", number);
}

// Completado
void pathToName (char *path, char **name) {
    int pos = 0;
    int i = 0;
    
    while ((path[pos] != '.') && (path[pos] != '\0')) {
        pos++;
    }
    
    *name = malloc(sizeof(char) * (pos + 1));
    
    for (i = 0; i < pos; i++) {
        (*name)[i] = path[i];
    }
    (*name)[i] = '\0';

}

// Completado
void getFileSize (char *path, char **size) {
    int file_size = 0;
    int fd = 0;
    int bytes;
    char buffer[256];

    fd = open(path, O_RDONLY);

    if (fd < 0) {
        showText("Error getting file size.\n");
        numberToString(file_size, &(*size));
        return;
    }

    do {
        bytes = read(fd, buffer, sizeof(buffer));
        if (bytes > 0) {
            file_size += bytes;
        }
    } while (bytes > 0);

    numberToString(file_size, &(*size));

    close(fd);
}

// Completado
void getFileData (char *path, char **bytes) {
    int fd = 0;
    int read_bytes = 0;
    int pos = 0;
    int i = 0, j = 0;
    char buffer[256];
    char *tmp;

    *bytes = NULL;

    fd = open(path, O_RDONLY);

    if (fd < 0) {
        showText("Error getting file size.\n");
    } else {
        do {
            read_bytes = read(fd, buffer, sizeof(buffer));
            if (read_bytes > 0) {
                pos += read_bytes;
                tmp = realloc(*bytes, sizeof(char) * pos);
                *bytes = tmp;
                while (j < read_bytes) {
                    (*bytes)[i++] = buffer[j++];
                }
                j = 0;
            }
        } while (read_bytes > 0);
        tmp = realloc(*bytes, sizeof(char) * (pos + 1));
        *bytes = tmp;
        (*bytes)[pos] = '\0';
        close(fd);
    }
}

// Completado
void fillPadding (char *data, int data_length) {
    int i = data_length;

    while (i < DATA_MAX_SIZE) {
        data[i] = 0x00;
        i++;
    }
}

// Completado
int calculateCheckSum (Protocol protocol) {
    int i = 0;
    int checksum = 0;

    checksum += protocol.type;

    for (i = 0; i < ORIGIN_SIZE; i++) {
        checksum += protocol.origin[i];
    }

    for (i = 0; i < DESTINATION_SIZE; i++) {
        checksum += protocol.destination[i];
    }

    checksum += protocol.data_length;

    for (i = 0; i < DATA_MAX_SIZE; i++) {
        checksum += protocol.data[i];
    }

    return checksum;
}

int checkMD5SUM (/*char data[]*/) {
    // En la fase 2 no se implementa.
    return 1;       // Devuelva que el MD5SUM ha sido correcto
}

// Completado
void calculateMD5SUM (char **sum) {
    // Para la fase 2, esta función no se implementa. Lo que debería hacer esta función es crear un fork()
    // que puede "sacrificarse" para ejecutar el comando del sistema operativo que consigue este checksum.
    // También se deberian implementar pipes para poder comunicarse con el proceso padre y guardar el varlor obtenido.

    // Por ahora se llena con 0x00, como manera de "hardcode"

    *sum = malloc(sizeof(char) * 32);
    for (int i = 0; i < 32; i++) {
        (*sum)[i] = 0x00;
    }
}



/*********************************************** PROTOCOL HELPERS ***********************************************/

// Completado
void copyIpToOrigin (char origin[], char* ip, int port) {
    char *port_char;
    int i = 0, j = 0;

    for (i = 0; i < (int)strlen(ip); i++) {
        origin[i] = ip[i];
    }
    origin[i] = ':';
    i++;

    numberToString(port, &port_char);

    while (port_char[j] != '\0') {
        origin[i++] = port_char[j++];
    }

    while (i < ORIGIN_SIZE) {
        origin[i++] = '\0';
    }

    free(port_char);
}

// Completado
void copyNameToDest (char destination[], char* realm_name) {
    int i = 0;

    while (realm_name[i] != '\0') {
        destination[i] = realm_name[i];
        i++;
    }

    while (i < DESTINATION_SIZE) {
        destination[i] = '\0';
        i++;
    }
}

// Completado
void copyToData (Protocol *protocol, char* data) {
    int i = 0;

    while (data[i] != '\0') {
        protocol->data[i] = data[i];
        i++;
    }

    protocol->data_length = i;

    fillPadding(protocol->data, i);
}

void getIpAndPortFromProtocol (char origin[], struct in_addr *ip, int *port) {
    int i = 0;
    int j = 0;
    char *ip_string = NULL;
    char *port_string = NULL;

    for (i = 0; origin[i] != ':'; i++) {
        ip_string = realloc(ip_string, sizeof(char) * (i + 1));
        ip_string[i] = origin[i];
    }
    ip_string = realloc(ip_string, sizeof(char) * (i + 1));
    ip_string[i] = '\0';

    inet_pton(AF_INET, ip_string, ip);
    
    i++;

    while (j < 4) {
        port_string = realloc(port_string, sizeof(char) * (j + 1));
        port_string[j++] = origin[i++];
    }
    port_string = realloc(port_string, sizeof(char) * (j + 1));
    port_string[j] = '\0';

    *port = atoi(port_string);

    free(ip_string);
    free(port_string);
}

void getDataString (char **string, char *data) {
    int i = 0;

    *string = malloc(sizeof(char) * (strlen(data) + 1));

    while (data[i] != '\0') {
        (*string)[i] = data[i];
        i++;
    }
    (*string)[i] = '\0';
}

void arrayToString(char* protocol_destination, char** destination_name) {
    *destination_name = (char*)malloc(sizeof(char) * (strlen(protocol_destination) + 1));
    strcpy(*destination_name, protocol_destination); 
}

void getWordFromProtocol (char *data, int word_pos, char **word) {
    int i = 0;
    int j = 0;
    char *buffer = NULL;

    for (i = 0; i < (word_pos - 1); i++) {
        while ((data[j] != '&') && (data[j] != '\0')) {
            j++;
        }
        j++;
    }

    for (i = 0; (data[j] != '&') && (data[j] != '\0'); i++) {
        buffer = realloc(buffer, sizeof(char) * (i + 1));
        buffer[i] = data[j];
        j++;
    }
    buffer = realloc(buffer, sizeof(char) * (i + 1));
    buffer[i] = '\0';

    *word = realloc(*word, sizeof(char) * (strlen(buffer) + 1));
    strcpy(*word, buffer);
    free(buffer);
}

/*********************************************** PROTOCOLS ***********************************************/

// Completado
Protocol comAllianceRequest (Maester maester_orig, char *realm_name, char *sigil_path) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;
    char *sigil_name;
    char *file_size = 0;
    char *mdsum;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = ALLIANCE_REQUEST_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formatting
    pathToName(sigil_path, &sigil_name);
    getFileSize(sigil_path, &file_size);
    calculateMD5SUM(&mdsum);

        // Name length + sigil name length + file size + MD5SUM size + 3 x '&' + '\0'
    data = malloc(sizeof(uint8_t) * (strlen(maester_orig.realm_name) + strlen(sigil_name) + atoi(file_size) + 32 + 3 + 1));
    data[0] = '\0';
    strcat(data, maester_orig.realm_name);
    strcat(data, "&");
    strcat(data, sigil_name);
    strcat(data, "&");
    strcat(data, file_size);
    strcat(data, "&");
    strcat(data, mdsum);

    copyToData(&protocol, data);

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);
    free(sigil_name);
    free(file_size);
    free(mdsum);

    return protocol;
}

// Completado
Protocol commSigilTransfer (Maester maester_orig, char *realm_name, char *data, int data_length) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    int i = 0;
    
    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));
    
    // Type
    protocol.type = SIGIL_TRANSFER_TYPE;

    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);
    
    // Data formatting
    for (i = 0; i < data_length; i++) {
        protocol.data[i] = data[i];
    }
    protocol.data_length = data_length;
    fillPadding(protocol.data, data_length);
    
    // Checksum
    protocol.checksum = calculateCheckSum(protocol);
    
    return protocol;
}

// Completado
Protocol commAllianceResponse (Maester maester_orig, char *realm_name, char* decision) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = ALLIANCE_RESPOND_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formatting
    data = malloc(sizeof(char) * (strlen(decision) + strlen(maester_orig.realm_name) + 2));
    data[0] = '\0';
    strcat(data, decision);
    strcat(data, "&");
    strcat(data, maester_orig.realm_name);
    copyToData(&protocol, data);

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);

    return protocol;
}

// Completado
Protocol commProductsRequest (Maester maester_orig, char *realm_name) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = LIST_PRODUCT_REQUEST_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formatting
    copyToData(&protocol, maester_orig.realm_name);

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);

    return protocol;
}

// Completado
Protocol commProductsResponse (Maester maester_orig, char *realm_name, char *file_path) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;
    char *file_size = 0;
    char *mdsum;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = LIST_PRODUCT_RESPOND_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formatting
    getFileSize(file_path, &file_size);
    calculateMD5SUM(&mdsum);

        // Name length + sigil name length + file size + MD5SUM size + 2 x '&' + '\0'
    data = malloc(sizeof(uint8_t) * (strlen(file_path) + atoi(file_size) + 32 + 2 + 1));
    data[0] = '\0';
    strcat(data, file_path);
    strcat(data, "&");
    strcat(data, file_size);
    strcat(data, "&");
    strcat(data, mdsum);

    copyToData(&protocol, data);

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);
    free(file_size);
    free(mdsum);

    return protocol;
}

// Completado
Protocol commProductTransfer (Maester maester_orig, char *realm_name, char *file_path, char *data, int data_length) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    int i = 0;
    
    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));
    
    // Type
    protocol.type = LIST_PRODUCT_TRANSFER_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);
    
    // Destination
    copyNameToDest(protocol.destination, realm_name);
    
    // Data formatting
    for (i = 0; i < data_length; i++) {
        protocol.data[i] = data[i];
    }
    protocol.data_length = data_length;
    fillPadding(protocol.data, data_length);
    
    // Checksum
    protocol.checksum = calculateCheckSum(protocol);
    
    return protocol;
}

// Completado
Protocol commOrderRequest (Maester maester_orig, char *realm_name, char *file_path) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;
    char *file_size = 0;
    char *mdsum;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = ORDER_REQUEST_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formatting
    getFileSize(file_path, &file_size);
    calculateMD5SUM(&mdsum);

        // Name length + sigil name length + file size + MD5SUM size + 2 x '&' + '\0'
    data = malloc(sizeof(uint8_t) * (strlen(file_path) + atoi(file_size) + 32 + 2 + 1));
    data[0] = '\0';
    strcat(data, file_path);
    strcat(data, "&");
    strcat(data, file_size);
    strcat(data, "&");
    strcat(data, mdsum);

    copyToData(&protocol, data);

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);
    free(file_size);
    free(mdsum);

    return protocol;
}

// Completado
Protocol commOrderTransfer (Maester maester_orig, char *realm_name, char *data, int data_length) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    int i = 0;
    
    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));
    
    // Type
    protocol.type = ORDER_TRANSFER_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);
    
    // Destination
    copyNameToDest(protocol.destination, realm_name);
    
    // Data formatting
    for (i = 0; i < data_length; i++) {
        protocol.data[i] = data[i];
    }

    protocol.data_length = data_length;
    fillPadding(protocol.data, data_length);
    
    // Checksum
    protocol.checksum = calculateCheckSum(protocol);
    
    return protocol;
}

// Completado - No utilizado para la Fase 2
Protocol commOrderResponse (Maester maester_orig, char *realm_name, char *response) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = ORDER_RESPOND_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formatting
    copyToData(&protocol, response);

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);

    return protocol;
}

// Completado
Protocol commDisconnection (Maester maester_orig, char *realm_name) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = DISCONNECTION_TYPE;

    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formatting
    copyToData(&protocol, "DISCONNECT");

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);

    return protocol;
}

/*********************************************** ERRORS ***********************************************/

// Completado
Protocol commUnknownRealm (Maester maester_orig, char *realm_name) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = UNKNOWN_REALM_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formatting
    copyToData(&protocol, "UNKNOWN_REALM");

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);

    return protocol;
}

// Completado
Protocol commNoAlliance (Maester maester_orig, char *realm_name) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = NO_ALLIANCE_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formatting
    copyToData(&protocol, "AUTH");

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);

    return protocol;
}

// Completado
Protocol commAckFile (Maester maester_orig, char *realm_name, char response[]) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = ACK_FILE_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formattin
    data = malloc(sizeof(char) * (strlen(response) + 1 + strlen(realm_name) + 1));
    data[0] = '\0';
    strcpy(data, response);
    strcat(data, "&");
    strcat(data, realm_name);
    copyToData(&protocol, data);

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);

    return protocol;
}

// Completado
Protocol commMd5Ack (Maester maester_orig, char *realm_name, int successful) {
    Protocol protocol;
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol.type = CHECKSUM_TYPE;
    
    // Origin
    copyIpToOrigin(protocol.origin, ip_string, maester_orig.port);

    // Destination
    copyNameToDest(protocol.destination, realm_name);

    // Data formatting

    if (successful) {
        copyToData(&protocol, "CHECK_OK");
    } else {
        copyToData(&protocol, "CHECK_KO");
    }

    // Checksum
    protocol.checksum = calculateCheckSum(protocol);

    free(data);

    return protocol;
}

// Completado
Protocol commNACK (char *realm_name) {
    Protocol protocol;

    // Type
    protocol.type = NACK_TYPE;

    // Data formatting
    copyToData(&protocol, realm_name);

    return protocol;
}