#include "envoy.h"

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
                
                buffer = read_until(child_to_parent[i][0], '\0');
                //Hay que definir que envian los envoys
                free(buffer);
            }
        }
    }
}


void envoyMain (int i) {
    EnvoyProtocol envoy_protocol;
    int flag = 0;

    while (!flag) {
        //Aqui lo que hagan los envoys
        read(parent_to_child[i][0], &envoy_protocol, sizeof(EnvoyProtocol));
        
        switch (envoy_protocol.type) {
            case ALLIANCE:
                manageReception(&reception, &protocol_struct, &maester, flag, envoy_protocol.fd_client);
                break;
            case LIST_PRODUCTS:
                break;
            case TRADE:
                break;
            default:
                break;
        }
    }
}