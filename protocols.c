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

    checksum += (unsigned char)protocol.type;

    for (i = 0; i < ORIGIN_SIZE; i++) {
        checksum += (unsigned char)protocol.origin[i];
    }

    for (i = 0; i < DESTINATION_SIZE; i++) {
        checksum += (unsigned char)protocol.destination[i];
    }

    checksum += (unsigned char)(protocol.data_length >> 8);
    checksum += (unsigned char)(protocol.data_length & 0xFF);

    for (i = 0; i < protocol.data_length; i++) {
        checksum += (unsigned char)protocol.data[i];
    }

    return checksum % 65536;
}

int checkMD5SUM (/*char data[]*/) {
    // En la fase 2 no se implementa.
    return 1;       // Devuelva que el MD5SUM ha sido correcto
}

void calculateMD5SUM (char* file_name, char* buffer) {
    int fd[2];
    int status;
    pid_t pid;
    char* temp_buffer;
    int i;
    
    pipe(fd);

    pid = fork();
    
    if (pid == 0) {
        // Hijo
        close(fd[0]); 
        dup2(fd[1], STDOUT_FILENO); 
        close(fd[1]);
        execlp("md5sum", "md5sum", file_name, NULL); 
        exit(1);
    } else {
        // Padre
        close(fd[1]);
        temp_buffer = readUntil(fd[0], ' ');
        close(fd[0]);

        i = 0;
        while (temp_buffer[i] != '\0') {
            buffer[i] = temp_buffer[i];
            i = i + 1;
        }
        buffer[i] = '\0';
        
        free(temp_buffer);
        waitpid(pid, &status, 0);
    }
}

void calculateReceivingMD5SUM (char* data, int data_length, char* buffer) {
    int fd_pipe[2];
    int fd_temp;
    int status;
    pid_t pid;
    char* temp_buffer;
    char* temp_file_path;
    int i;
    
    temp_file_path = malloc(sizeof(char) * 50);
    sprintf(temp_file_path, "TEMP_%d.txt", getpid());
    
    fd_temp = open(temp_file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    write(fd_temp, data, data_length);
    close(fd_temp);
    
    pipe(fd_pipe);
    pid = fork();
    
    if (pid == 0) {
        close(fd_pipe[0]); 
        dup2(fd_pipe[1], STDOUT_FILENO); 
        close(fd_pipe[1]);
        execlp("md5sum", "md5sum", temp_file_path, NULL); 
        exit(1);
    } else {
        close(fd_pipe[1]);
        temp_buffer = readUntil(fd_pipe[0], ' ');
        close(fd_pipe[0]);

        i = 0;
        while (temp_buffer[i] != '\0') {
            buffer[i] = temp_buffer[i];
            i = i + 1;
        }
        buffer[i] = '\0';
        
        free(temp_buffer);
        waitpid(pid, &status, 0);
        
        unlink(temp_file_path);
        free(temp_file_path);
    }
}

// Completado
/*void calculateMD5SUM (char **sum) {
    // Para la fase 2, esta función no se implementa. Lo que debería hacer esta función es crear un fork()
    // que puede "sacrificarse" para ejecutar el comando del sistema operativo que consigue este checksum.
    // También se deberian implementar pipes para poder comunicarse con el proceso padre y guardar el varlor obtenido.

    // Por ahora se llena con 0x00, como manera de "hardcode"

    *sum = malloc(sizeof(char) * 32);
    for (int i = 0; i < 32; i++) {
        (*sum)[i] = 0x00;
    }
}*/



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

/*Protocol stringToProtocol (char *buffer) {
    Protocol protocol;
    int i = 0;

    protocol.type = buffer[0];

    for (i = 0; i < ORIGIN_SIZE; i++) {
        protocol.origin[i] = buffer[1 + i];
    }

    for (i = 0; i < DESTINATION_SIZE; i++) {
        protocol.destination[i] = buffer[1 + ORIGIN_SIZE + i];
    }

    protocol.data_length = (uint16_t)((buffer[1 + ORIGIN_SIZE + DESTINATION_SIZE] << 8) | buffer[1 + ORIGIN_SIZE + DESTINATION_SIZE + 1]);

    for (i = 0; i < DATA_MAX_SIZE; i++) {
        if (i < protocol.data_length) {
            protocol.data[i] = buffer[1 + ORIGIN_SIZE + DESTINATION_SIZE + 2 + i];
        } else {
            protocol.data[i] = 0x00;
        }
    }

    protocol.checksum = (uint16_t)((buffer[1 + ORIGIN_SIZE + DESTINATION_SIZE + 2 + (int)protocol.data_length] << 8) | buffer[1 + ORIGIN_SIZE + DESTINATION_SIZE + 2 + DATA_MAX_SIZE + 1]);

    return protocol;
}*/

Protocol stringToProtocol (char *buffer) {
    Protocol protocol;
    int i;

    protocol.type = buffer[0];

    for (i = 0; i < ORIGIN_SIZE; i++) {
        protocol.origin[i] = buffer[TYPE_SIZE + i];
    }

    for (i = 0; i < DESTINATION_SIZE; i++) {
        protocol.destination[i] = buffer[TYPE_SIZE + ORIGIN_SIZE + i];
    }

    protocol.data_length = (uint16_t)(((unsigned char)buffer[TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE] << 8) | 
                                       (unsigned char)buffer[TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE + 1]);

    for (i = 0; i < DATA_MAX_SIZE; i++) {
        if (i < protocol.data_length) {
            protocol.data[i] = buffer[TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE + DATALENGTH_SIZE + i];
        } else {
            protocol.data[i] = 0x00;
        }
    }

    protocol.checksum = (uint16_t)(((unsigned char) buffer[318] << 8) | (unsigned char) buffer[319]);

    return protocol;
}









void copyOriginToProtocol (char protocol[320], char *ip, int port) {
    char *port_char;
    int i = 1, j = 0;

    while (ip[j] != '\0') {
        protocol[i] = ip[j];
        i++;
        j++;
    }
    protocol[i] = ':';
    i++;

    numberToString(port, &port_char);

    j = 0;
    while (port_char[j] != '\0') {
        protocol[i++] = port_char[j++];
    }

    while (i < (TYPE_SIZE + ORIGIN_SIZE)) {
        protocol[i++] = '\0';
    }

    free(port_char);
}

void copyDestinationToProtocol (char protocol[320], char *realm_name) {
    int i = 0, j = 0;

    i = TYPE_SIZE + ORIGIN_SIZE;

    while (realm_name[j] != '\0') {
        protocol[i] = realm_name[j];
        i++;
        j++;
    }

    while (i < (TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE)) {
        protocol[i] = '\0';
        i++;
    }
}

int copyDataToProtocol (char protocol[320], char *data) {
    int i = 0, j = 0;
    u_int16_t datalength;

    i = TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE + DATALENGTH_SIZE;

    while (data[j] != '\0') {
        protocol[i] = data[j];
        i++;
        j++;
    }

    i = TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE;

    datalength = (u_int16_t) j;

    protocol[i] = (datalength >> 8) & 0xFF;
    protocol[i + 1] = datalength & 0xFF;

    return j;
}

void copyCalculateChecksum (char protocol[320], int data_length) {
    int i = 0;
    int result = 0;
    u_int16_t checksum;

    result += protocol[0];

    for (i = 0; i < ORIGIN_SIZE; i++) {
        result += (unsigned char)protocol[TYPE_SIZE + i];
    }

    for (i = 0; i < DESTINATION_SIZE; i++) {
        result += (unsigned char)protocol[TYPE_SIZE + ORIGIN_SIZE + i];
    }

    result += (unsigned char)protocol[TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE];
    result += (unsigned char)protocol[TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE + 1];

    for (i = 0; i < data_length; i++) {
        result += (unsigned char)protocol[TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE + DATALENGTH_SIZE + i];
    }

    checksum = (u_int16_t) result;
    
    protocol[318] = (checksum >> 8) & 0xFF;
    protocol[319] = checksum & 0xFF;
}

void copyPadding (char protocol[320], int data_length) {
    int i = 0;

    i = TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE + DATALENGTH_SIZE + data_length;

    while (i < 318) {
        protocol[i] = '\0';
        i++;
    }
}






int calculateChecksumFromString(char *buffer, int data_length) {
    int i = 0;
    int checksum = 0;
    
    for (i = 0; i < 318; i++) {
        checksum += (unsigned char)buffer[i];
    }
    
    return checksum % 65536;
}

/*********************************************** PROTOCOLS ***********************************************/

void commAllianceRequest (char protocol[320], Maester maester_orig, char *realm_name, char *sigil_path) {
    char ip_string[INET_ADDRSTRLEN];
    char *data = NULL;
    char *sigil_name;
    char *file_size = 0;
    char mdsum[33];
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = ALLIANCE_REQUEST_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data formatting
    pathToName(sigil_path, &sigil_name);
    getFileSize(sigil_path, &file_size);
    calculateMD5SUM(sigil_path, mdsum);

        // Name length + sigil name length + file size + MD5SUM size + 3 x '&' + '\0'
    data = malloc(sizeof(uint8_t) * (strlen(maester_orig.realm_name) + strlen(sigil_name) + strlen(file_size) + 32 + 3 + 1));
    data[0] = '\0';
    strcat(data, maester_orig.realm_name);
    strcat(data, "&");
    strcat(data, sigil_name);
    strcat(data, "&");
    strcat(data, file_size);
    strcat(data, "&");
    strcat(data, mdsum);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, data);;

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);

    free(data);
    free(sigil_name);
    free(file_size);
}

void commSigilTransfer (char protocol[320], Maester maester_orig, char *realm_name, char *data, int data_length) {
    char ip_string[INET_ADDRSTRLEN];
    int i;
    int data_start;
    
    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));
    
    // Type
    protocol[0] = SIGIL_TRANSFER_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);
    
    // Data + datalength
    data_start = TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE + DATALENGTH_SIZE;
    for (i = 0; i < data_length; i++) {
        protocol[data_start + i] = data[i];
    }
    
    i = TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE;
    protocol[i] = (data_length >> 8) & 0xFF;
    protocol[i + 1] = data_length & 0xFF;
    
    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);
}

void commAllianceResponse (char protocol[320], Maester maester_orig, char *realm_name, char *decision) {
    char ip_string[INET_ADDRSTRLEN];
    char *data;
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = ALLIANCE_RESPOND_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data formatting
    data = malloc(sizeof(char) * (strlen(decision) + strlen(maester_orig.realm_name) + 2));
    data[0] = '\0';
    strcat(data, decision);
    strcat(data, "&");
    strcat(data, maester_orig.realm_name);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, data);

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);

    free(data);
}

void commProductsRequest (char protocol[320], Maester maester_orig, char *realm_name) {
    char ip_string[INET_ADDRSTRLEN];
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = LIST_PRODUCT_REQUEST_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, maester_orig.realm_name);

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);
}

void commProductsResponse (char protocol[320], Maester maester_orig, char *realm_name, char *file_path) {
    char ip_string[INET_ADDRSTRLEN];
    char *data;
    char *file_size;
    char mdsum[33];
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = LIST_PRODUCT_RESPOND_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data formatting
    getFileSize(file_path, &file_size);
    calculateMD5SUM(file_path, mdsum);

    data = malloc(sizeof(uint8_t) * (strlen(file_path) + strlen(file_size) + 32 + 2 + 1));
    data[0] = '\0';
    strcat(data, file_path);
    strcat(data, "&");
    strcat(data, file_size);
    strcat(data, "&");
    strcat(data, mdsum);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, data);

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);

    free(data);
    free(file_size);
    free(mdsum);
}

void commProductTransfer (char protocol[320], Maester maester_orig, char *realm_name, char *data, int data_length) {
    char ip_string[INET_ADDRSTRLEN];
    int i;
    int data_start;
    
    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));
    
    // Type
    protocol[0] = LIST_PRODUCT_TRANSFER_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);
    
    // Data + datalength
    data_start = TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE + DATALENGTH_SIZE;
    for (i = 0; i < data_length; i++) {
        protocol[data_start + i] = data[i];
    }
    
    i = TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE;
    protocol[i] = (data_length >> 8) & 0xFF;
    protocol[i + 1] = data_length & 0xFF;
    
    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);
}

void commOrderRequest (char protocol[320], Maester maester_orig, char *realm_name, char *file_path) {
    char ip_string[INET_ADDRSTRLEN];
    char *data;
    char *file_size;
    char mdsum[33];
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = ORDER_REQUEST_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data formatting
    getFileSize(file_path, &file_size);
    calculateMD5SUM(file_path, &mdsum);

    data = malloc(sizeof(uint8_t) * (strlen(file_path) + strlen(file_size) + 32 + 2 + 1));
    data[0] = '\0';
    strcat(data, file_path);
    strcat(data, "&");
    strcat(data, file_size);
    strcat(data, "&");
    strcat(data, mdsum);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, data);

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);

    free(data);
    free(file_size);
    free(mdsum);
}

void commOrderTransfer (char protocol[320], Maester maester_orig, char *realm_name, char *data, int data_length) {
    char ip_string[INET_ADDRSTRLEN];
    int i;
    int data_start;
    
    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));
    
    // Type
    protocol[0] = ORDER_TRANSFER_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);
    
    // Data + datalength
    data_start = TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE + DATALENGTH_SIZE;
    for (i = 0; i < data_length; i++) {
        protocol[data_start + i] = data[i];
    }
    
    i = TYPE_SIZE + ORIGIN_SIZE + DESTINATION_SIZE;
    protocol[i] = (data_length >> 8) & 0xFF;
    protocol[i + 1] = data_length & 0xFF;
    
    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);
}

void commOrderResponse (char protocol[320], Maester maester_orig, char *realm_name, char *response) {
    char ip_string[INET_ADDRSTRLEN];
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = ORDER_RESPOND_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, response);

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);
}

void commDisconnection (char protocol[320], Maester maester_orig, char *realm_name) {
    char ip_string[INET_ADDRSTRLEN];
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = DISCONNECTION_TYPE;

    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, "DISCONNECT");

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);
}


/*********************************************** ERRORS ***********************************************/


void commUnknownRealm (char protocol[320], Maester maester_orig, char *realm_name) {
    char ip_string[INET_ADDRSTRLEN];
    char *data;
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = UNKNOWN_REALM_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data formatting
    data = malloc(sizeof(char) * (strlen("UNKNOWN_REALM") + strlen(realm_name) + 2));
    data[0] = '\0';
    strcat(data, "UNKNOWN_REALM");
    strcat(data, "&");
    strcat(data, realm_name);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, data);

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);

    free(data);
}

void commNoAlliance (char protocol[320], Maester maester_orig, char *realm_name) {
    char ip_string[INET_ADDRSTRLEN];
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = NO_ALLIANCE_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, "AUTH");

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);
}

void commAckFile (char protocol[320], Maester maester_orig, char *realm_name, char *response) {
    char ip_string[INET_ADDRSTRLEN];
    char *data;
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = ACK_FILE_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data formatting
    data = malloc(sizeof(char) * (strlen(response) + strlen(realm_name) + 2));
    data[0] = '\0';
    strcat(data, response);
    strcat(data, "&");
    strcat(data, realm_name);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, data);

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);

    free(data);
}

void commMd5Ack (char protocol[320], Maester maester_orig, char *realm_name, int successful) {
    char ip_string[INET_ADDRSTRLEN];
    char *data;
    int data_length;

    inet_ntop(AF_INET, &maester_orig.ip, ip_string, sizeof(ip_string));

    // Type
    protocol[0] = CHECKSUM_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, ip_string, maester_orig.port);

    // Destination
    copyDestinationToProtocol(protocol, realm_name);

    // Data formatting
    data = malloc(sizeof(char) * (strlen("CHECK_OK") + strlen(realm_name) + 2));
    data[0] = '\0';
    
    if (successful) {
        strcat(data, "CHECK_OK");
    } else {
        strcat(data, "CHECK_KO");
    }
    strcat(data, "&");
    strcat(data, realm_name);

    // Data + datalength
    data_length = copyDataToProtocol(protocol, data);

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);

    free(data);
}

void commNACK (char protocol[320], char *realm_name) {
    int data_length;

    // Type
    protocol[0] = NACK_TYPE;
    
    // Origin
    copyOriginToProtocol(protocol, "", 0);

    // Destination
    copyDestinationToProtocol(protocol, "");

    // Data + datalength
    data_length = copyDataToProtocol(protocol, realm_name);

    // Checksum
    copyCalculateChecksum(protocol, data_length);

    // Padding
    copyPadding(protocol, data_length);
}