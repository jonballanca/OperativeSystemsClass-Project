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
#define SIGIL_MD5SUM_WORD 4
#define LIST_PROD_MD5SUM_WORD 3
#define ORDER_MD5SUM_WORD 3
#define ACK_CONFIRMATION_WORD 1
#define MD5SUM_CONFIRMATION_WORD 1
#define MD5SUM_REALM_WORD
#define ORDER_CONFIRMATION_WORD 1
#define ORDER_REASON_WORD 1
#define REALM_NAME_WORD 2
#define SIGIL_FILESIZE_WORD 3
#define DESTINATION_WORD 1
#define TRADE_FILESIZE_WORD 2

#define SENDING_INVENTORY 1
#define SENDING_SIGIL 2
#define SENDING_TRADE 3

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

/***********************************************
*
* @Finalitat: Muesta el tipo de protocolo como un string
* @Parametres:  in: protocol = protocolo que se desea printar
* @Retorn:  ---
*
************************************************/
void showProtocolAsString (uint8_t protocol);

/***********************************************
*
* @Finalitat: Mostrar el mensaje del hop, si es sigil_protocol y es el primero que se recibe, 
              envia el tipo de hop que esta recibiendo y cuando ya no es el primero, envia el porcentaje enviado.
              Si no es el sigil_protcol indica el tipo de protocolo que es
* @Parametres:  in: destination_name = nombre del realm destino
                in: protocol         = tipo de protocolo
                in: origin           = 
                in: error            =
                in: maester          =   
                in: bytes_sent       =
                in: total_bytes      =
                in: previous_bytes                
* @Retorn:  ---
*
************************************************/
void showHopMessage(char* destination_name, Protocol protocol, struct sockaddr_in origin, int error, Maester maester, int bytes_sent, int total_bytes, int previous_bytes);

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void showInitializedMaester ();

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void printAllianceRequest (char *realm_name);

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void printOrderRequest (char *realm_name);

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void printSuccessfulAlliance (char *realm_name);

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void printFailedAlliance (char *realm_name);

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void printReset (char *text);

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void printBold (char *text);

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void printDisconnection (char *realm_name);

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void printIpAddress (struct in_addr ip);

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void printRejectReason (char *reason);

void printExpectedBytes (int receiving_type, char *word);

void printDoneReceiving (int receiving_type);

/***********************************************
*
* @Finalitat: Mostrar los productos que estan disponibles para tradear
* @Parametres:  in: inventory = productos que estan disponibles para tradear
* @Retorn:  ---
*
************************************************/
void printListProductsRequest (char *realm_name);

#endif