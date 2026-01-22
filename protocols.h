/***********************************************
*
* @Proposit: Modulo que contiene todas las funciones que hacen la creacion y organizacion de los 
*           protocolos de comunicación.
* @Autor/s: Jon Ballanca y Alejandro Cortés
* @Data creacio: 20 / 10 / 2025
* @Data ultima modificacio: 24 / 10 / 2025
*
************************************************/

// Define Guard
#ifndef PROTOCOLS_H
#define PROTOCOLS_H

// Librerias del sistema
#define _GNU_SOURCE
#include <stdio.h> // snprintf, asprintf, etc
#include <stdlib.h> // Per atoi
#include <string.h> // Per strlen
#include <unistd.h> // Per read i write
#include <sys/stat.h> // Per constants de modes (permisos del archiu) com S_IRUSR
#include <fcntl.h> // Per open i constants del tipo de open com O_RDONLY
#include <time.h> //Per poder agafar la informacio del temps actual
#include <signal.h> //Signals
#include <pthread.h> // Threads
#include <netinet/in.h> // IP format
#include <ctype.h> // Para clasificar chars

// Librerias propias
#include "inventory.h"
#include "config.h"

#define MAX_CONNECTIONS 5
#define ERR_SOCKET "ERROR: no s'ha pogut crear el socket\n"
#define ERR_CONNECT "ERROR: no s'ha pogut connectar amb el servidor\n"

#define ORIGIN_SIZE 20
#define DESTINATION_SIZE 20
#define DATA_MAX_SIZE 275

#define PROTOCOL_SIZE 320

#define ALLIANCE_REQUEST_TYPE 0x01
#define SIGIL_TRANSFER_TYPE 0x02
#define ALLIANCE_RESPOND_TYPE 0x03
#define LIST_PRODUCT_REQUEST_TYPE 0X11
#define LIST_PRODUCT_RESPOND_TYPE 0X12
#define LIST_PRODUCT_TRANSFER_TYPE 0X13
#define ORDER_REQUEST_TYPE 0x14
#define ORDER_TRANSFER_TYPE 0x15
#define ORDER_RESPOND_TYPE 0x16
#define DISCONNECTION_TYPE 0x27
//TIPOS ERRORES
#define UNKNOWN_REALM_TYPE 0x21
#define NO_ALLIANCE_TYPE 0x25
#define PING_TYPE 0x26
#define ACK_FILE_TYPE 0x31
#define CHECKSUM_TYPE 0x32
#define NACK_TYPE 0x69

typedef struct Protocol {
    uint8_t type;
    char origin[ORIGIN_SIZE];
    char destination[DESTINATION_SIZE];
    uint16_t data_length;
    char data[DATA_MAX_SIZE];
    uint16_t checksum;
} Protocol;

// Declaración del tipo propio del MaesterInfo
typedef struct Maester Maester;

int openClientConn (struct in_addr ip, int port);

int createSocket (struct in_addr ip, int port);

void numberToString (int number, char **string);

void pathToName (char *path, char **name);

void getFileSize (char *path, char **size);

void getFileData (char *path, char **bytes);

void fillPadding (char *data, int data_length);

int calculateCheckSum (Protocol protocol);

int checkMD5SUM (/*char data[]*/);

void calculateMD5SUM (char **sum);



/*********************************************** PROTOCOL HELPERS ***********************************************/

void copyIpToOrigin (char origin[], char* ip, int port);

void copyNameToDest (char destination[], char* realm_name);

void copyToData (Protocol *protocol, char* data);

void getIpAndPortFromProtocol(char origin[], struct in_addr *ip, int *port);

void getDataString (char **string, char *data);

void arrayToString(char* protocol_destination, char** destination_name);

void getWordFromProtocol (char *data, int word_pos, char **word);

/*********************************************** PROTOCOLS ***********************************************/

Protocol comAllianceRequest (Maester maester_orig, char *realm_name, char *sigil_path);

Protocol commSigilTransfer (Maester maester_orig, char *realm_name, char *data, int data_length);

Protocol commAllianceResponse (Maester maester_orig, char *realm_name, char decision[]);

Protocol commProductsRequest (Maester maester_orig, char *realm_name);

Protocol commProductsResponse (Maester maester_orig, char *realm_name, char *file_path);

Protocol commProductTransfer (Maester maester_orig, char *realm_name, char *file_path, char *data, int data_length);

// Completado - No utilizado para la Fase 2
Protocol commOrderRequest (Maester maester_orig, char *realm_name, char *file_path);

// Completado
Protocol commOrderTransfer (Maester maester_orig, char *realm_name, char *data, int data_length);

// Completado
Protocol commOrderResponse (Maester maester_orig, char *realm_name, char response[]);

Protocol commDisconnection (Maester maester_orig, char *realm_name);

/*********************************************** ERRORS ***********************************************/

Protocol commUnknownRealm (Maester maester_orig, char *realm_name);

Protocol commNoAlliance (Maester maester_orig, char *realm_name);

Protocol commAckFile (Maester maester_orig, char *realm_name, char response[]);

Protocol commMd5Ack (Maester maester_orig, char *realm_name, int successful);

Protocol commNACK (char *realm_name);

#endif

