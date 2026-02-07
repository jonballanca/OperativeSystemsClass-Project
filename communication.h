/***********************************************
*
* @Proposit: 
* @Autor/s: Jon Ballanca y Alejandro Cortés
* @Data creacio: 20 / 10 / 2025
* @Data ultima modificacio: 24 / 10 / 2025
*
************************************************/

// Define Guard
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

// Librerias propias
#include "protocols.h"
#include "printing.h"
#include "inventory.h"
//#include "config.h"

#define ACCEPTED 1
#define REJECTED 2
#define PENDING 3

typedef struct {
    char *realm_name;
    int status;
} Pledge;

typedef struct {
    Pledge *pledges;
    int status_count;
} PledgeStatus;

typedef struct {
    Inventory inventory;
    char *realm_name;
} RealmsStock;


typedef struct {
    Inventory inventory;
    PledgeStatus pledge_status;
    RealmsStock realms_stock;

    pthread_mutex_t pledge_status_mutex;
    pthread_mutex_t mutex_sending_vars;

    char* list_products_rl;         // El nombre del realm cuya lista de productos queremos
    char* inventory_file_name;
    char* sigil_file_path;
    char* trade_file_path;
    char* header_md5sum;

    int pledge_flag;
    int pledge_pending;
    char *pledge_realm_name;
    struct in_addr pledge_ip;
    int pledge_port;
    int response_confirmation;
    char *pledge_decision;

    int receiving_file;

    int sending_file;      // En 1 cuando es un list products, en 2 cuando es un sigil
    int send_listproducts;
    int send_sigil;
    int send_trade;
    
    int communication_filesize;
    int bytes_sent;
    int bytes_received;
    char *realm_name_sending;
    char *file_received;

    int hop_bytes_sent;
    int hop_total_bytes;

    int fd_file;
} ReceptionInfo;


void initializeReception (ReceptionInfo *reception, Inventory inventory, char *argument);

int connectFileTranfer (char* destination_name, Maester maester);

int sendProtocol(struct in_addr ip, int port, char *protocol);

int sendToRoute (char* destination_name, char *protocol, Maester maester);

void addAlliance (char *realm_name, Maester *maester, struct in_addr ip, int port);

int searchRoutePos (char* destination_name, Maester maester);

void sendDisconection (Maester maester);

void addActive (char *realm_name, Maester *maester);

void addInactive (char *realm_name, Maester *maester);

void addPledgeStatus(ReceptionInfo *reception, int status, char* realm_name);

int checkTradeInventory(ReceptionInfo *reception, char *realm_name);

Inventory getRealmInventory(ReceptionInfo *reception, char *realm_name);

void processAllianceResponse (char data[], char** pledge_decision, char** realm_name);

int getIfResponseType (Protocol protocol);

int checkDestinationRequest (char protocol_string[PROTOCOL_SIZE], Protocol protocol, struct sockaddr_in client_addr, Maester *maester, ReceptionInfo **reception);

void getRealmNameFromIp (char **realm_name, struct in_addr ip, int port, Maester *maester);

void getIpAndPortFromRealm (Maester maester, char *realm_name, struct in_addr *ip, int *port);

void prepareSendingFile (int *file_size, int *fd, char *file_path, Maester maester);

void manageReception (ReceptionInfo **reception, Protocol *protocol, Maester *maester, int flag, int fd_client);

#endif