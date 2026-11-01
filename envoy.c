#include "envoy.h"

fd_set current_sockets, ready_sockets;
int** parent_to_child = NULL;
int** child_to_parent = NULL;

// Thread
void* readingEnvoy (void* arg) {
    int num_pipes = *((int*)arg);
    int i = 0;
    char* buffer = NULL;


    while(1){
        //Esperar respuesta de los envoys
        ready_sockets = current_sockets;

        int activity = select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            printReset("Error en el select\n");
            continue;
        }

        for (i = 0; i < num_pipes; i++) {
            if (FD_ISSET(child_to_parent[i][0], &ready_sockets)) {
                
                buffer = readUntil(child_to_parent[i][0], '\0');
                //Hay que definir que envian los envoys
                free(buffer);
            }
        }
    }
}

void sendFile (EnvoyProtocol envoy_protocol, ReceptionInfo *reception, int sending_type) {
    int file_size = 0;
    int fd_file;
    int done_sending = 0;
    int bytes_sent = 0;
    int remaining_bytes = 0;
    int file_bytes = 0;
    char data[DATA_MAX_SIZE];
    char protocol[PROTOCOL_SIZE];
    
    prepareSendingFile(&file_size, &fd_file, envoy_protocol.file_path, envoy_protocol.maester);

    while (!done_sending) {
        remaining_bytes = file_size - bytes_sent;
        if (DATA_MAX_SIZE <= remaining_bytes) {
            file_bytes = read(fd_file, data, DATA_MAX_SIZE);

            switch (sending_type) {
                case SENDING_SIGIL:
                    commSigilTransfer(protocol, envoy_protocol.maester, reception->realm_name_sending, data, file_bytes);
                    break;
                case SENDING_INVENTORY:
                    commProductTransfer(protocol, envoy_protocol.maester, reception->realm_name_sending, data, file_bytes);
                    break;
                case SENDING_TRADE:
                    commOrderTransfer(protocol, envoy_protocol.maester, reception->realm_name_sending, data, file_bytes);
                    break;
                default:
                    break;
            }

            write(envoy_protocol.fd_client, protocol, PROTOCOL_SIZE);
            
            bytes_sent += file_bytes;
        } else {
            done_sending = 1;
            close(fd_file);
        }
    }
}


void envoyMain (int i) {
    EnvoyProtocol envoy_protocol;
    int flag = 0;
    ReceptionInfo reception;

    while (!flag) {
        //Aqui lo que hagan los envoys
        read(parent_to_child[i][0], &envoy_protocol, sizeof(EnvoyProtocol));
        
        switch (envoy_protocol.type) {
            case ALLIANCE:
                if (envoy_protocol.protocol_struct.type != SIGIL_TRANSFER_TYPE) {
                    manageReception(&reception, &envoy_protocol.protocol_struct, &envoy_protocol.maester, flag, envoy_protocol.fd_client);
                } else {
                    sendFile(envoy_protocol, &reception, SENDING_SIGIL);
                }
                break;
            case LIST_PRODUCTS:
                if (envoy_protocol.protocol_struct.type != LIST_PRODUCT_TRANSFER_TYPE) {
                    manageReception(&reception, &envoy_protocol.protocol_struct, &envoy_protocol.maester, flag, envoy_protocol.fd_client);
                } else {
                    sendFile(envoy_protocol, &reception, SENDING_INVENTORY);
                }
                break;
            case TRADE:
                if (envoy_protocol.protocol_struct.type != ORDER_TRANSFER_TYPE) {
                    manageReception(&reception, &envoy_protocol.protocol_struct, &envoy_protocol.maester, flag, envoy_protocol.fd_client);
                } else {
                    sendFile(envoy_protocol, &reception, SENDING_TRADE);
                }
                break;
            case EXIT:
                flag = 1;
                break;
            default:
                break;
        }
    }
}