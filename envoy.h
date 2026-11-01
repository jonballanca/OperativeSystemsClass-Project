/***********************************************
*
* @Proposit: 
* @Autor/s: Jon Ballanca y Alejandro Cortés
* @Data creacio: 20 / 10 / 2025
* @Data ultima modificacio: 24 / 10 / 2025
*
************************************************/

// Define Guard
#ifndef ENVOY_H
#define ENVOY_H

#include <stdio.h> // snprintf, asprintf, etc
#include <stdlib.h> // Per atoi
#include <string.h> // Per strlen
#include <unistd.h> // Per read i write
#include <sys/stat.h> // Per constants de modes (permisos del archiu) com S_IRUSR
#include <fcntl.h> // Per open i constants del tipo de open com O_RDONLY
#include <signal.h> //Signals
#include <pthread.h> // Threads

#include <ctype.h> // Para clasificar chars
#include <errno.h> //Errores de select

// Librerias propias
#include "config.h"
#include "communication.h"

#define ALLIANCE 1
#define TRADE 2

extern fd_set current_sockets, ready_sockets;  //Falta liberar memoria
extern int** parent_to_child;         //Falta liberar memoria
extern int** child_to_parent;          //Falta liberar memoria

//De momento para definir
typedef struct {
    uint8_t type;
    Maester maester;
    Protocol protocol_struct;
    int fd_client;
    char *file_path;
} EnvoyProtocol;


void* readingEnvoy(void* arg);

void envoyMain (int i);

#endif