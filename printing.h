/***********************************************
*
* @Proposit: 
* @Autor/s: Jon Ballanca y Alejandro Cortés
* @Data creacio: 20 / 10 / 2025
* @Data ultima modificacio: 24 / 10 / 2025
*
************************************************/

// Define Guard
#ifndef PRINTING_H
#define PRINTING_H

// Librerias propias
#include "protocols.h"
//#include "config.h"

#define RED "\e[0;31m"
#define RESET "\x1b[0m"
#define BOLD  "\x1b[1m"

// Declaración del tipo propio del MaesterInfo
typedef struct Maester Maester;

// Declaración del tipo propio del Protocol
typedef struct Protocol Protocol;

// Declaración del tipo propio del Inventory
typedef struct Inventory Inventory;

#define FLECHA "\u2192"
#define CURSOR_UP "\033[A"
#define ALLIANCE_REALM_WORD 1
#define LIST_PROD_FILESIZE_WORD 2
#define LIST_PROD_MDSUM_WORD 3
#define ACK_CONFIRMATION_WORD 1
#define MD5SUM_CONFIRMATION_WORD 1
#define MD5SUM_REALM_WORD
#define ORDER_CONFIRMATION_WORD 1
#define ORDER_REASON_WORD 1
#define REALM_NAME_WORD 2
#define SIGIL_FILESIZE_WORD 3
#define DESTINATION_WORD 1
#define TRADE_FILESIZE_WORD 2

/***********************************************
*
* @Finalitat: Mostrar un mensaje de error para cuando hay un error de string vacio
* @Parametres: ---
* @Retorn:  ---
*
************************************************/
void emptyStringError ();

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void showAvailableProducts (Inventory inventory);

void showProtocolAsString (uint8_t protocol);

void showHopMessage(char* destination_name, Protocol protocol, struct sockaddr_in origin, int error, Maester maester, int bytes_sent, int total_bytes, int previous_bytes);

void showInitializedMaester ();

void printAllianceRequest (char *realm_name);

void printOrderRequest (char *realm_name);

void printSuccessfulAlliance (char *realm_name);

void printFailedAlliance (char *realm_name);

void printReset (char *text);

void printBold (char *text);

void printDisconnection (char *realm_name);

void printIpAddress (struct in_addr ip);

void printRejectReason (char *reason);

void printListProductsRequest (char *realm_name);

#endif