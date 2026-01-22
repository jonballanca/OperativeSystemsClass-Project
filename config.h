/***********************************************
*
* @Proposit: Extraer la información del fichero de configuración para que el maester se la pueda guardar
* @Autor/s: Jon Ballanca y Alejandro Cortés
* @Data creacio: 20 / 10 / 2025
* @Data ultima modificacio: 22 / 10 / 2025
*
************************************************/

// Define Guard
#ifndef CONFIG
#define CONFIG

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
#include <arpa/inet.h> // Para hacer conversiones directas de IP a binario o texto

// Librerias propias
#include "commands.h"

// Constante de error del fichero de configuración
#define ERROR_CONFIG_FILE 1

// Constantes de casos de lectura de Routes
#define REALM_NAME 1
#define IP_ADDRESS 2
#define PORT_NUMBER 3

#define TIMED_OUT 1
#define RESET_TIMED_OUT 2

// Tipo propio de las rutas de communicación entre realms
typedef struct {
    char* realm_name;
    struct in_addr ip;
    int port;
    int alliance_exists;
    int active;
    int timed_out;
} Route;

// Tipo propio que guarda toda la información de un realm y representa ese realm con rutas y
// más información para la communicación entre realms
typedef struct Maester {
    char* realm_name;
    char* path;
    int num_envoys;
    struct in_addr ip;
    int port;
    Route* routes;
    int num_routes;
} Maester;

void changeTimedOut (char *realm_name, int action, Maester *maester);

int checkTimedOut (Maester maester, char *realm_name);

void removeAmpersand (char **buffer);

/***********************************************
*
* @Finalitat: Descompone el string de la ruta que se lee del fichero de configuración y guarda
*               cada elemento en la parte necesaria del maester.
* @Parametres:      in: maester = el maester del cual se esta leyendo el fichero de configuración.
*                   in: buffer = el string que se ha leido desde el fichero, una única ruta.
*                   in: route_pos = el número de ruta listado.
* @Retorn: Devuelve el maester con las rutas guardadas.
*
************************************************/
Maester decomposeRoute (Maester maester, char* buffer, int route_pos);

/***********************************************
*
* @Finalitat: Lee y guarda en el maester toda la información del fichero de configuración.
* @Parametres:      in/out: maester = el maester del cual se quiere leer la información.
*                   in: filename = el nombre del fichero para leer.
* @Retorn: Devuelve un entero que en 1 indica que el fichero se ha leido bien y en 0 indica que
*           el fichero no se ha leido bien.
*
************************************************/
int readConfigFile (Maester* maester, char* filename);

#endif