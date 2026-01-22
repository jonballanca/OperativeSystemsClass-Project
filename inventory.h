/***********************************************
*
* @Proposit: Controlar la logica de todo lo que tengo que ver con el inventario
* @Autor/s: Jon Ballanca y Alejandro Cortes
* @Data creacio: 20 / 10 / 2025
* @Data ultima modificacio: 24 / 10 / 2025
*
************************************************/

// Define Guard
#ifndef INVENTORY
#define INVENTORY

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

// Librerias propias
#include <math.h>
#include "printing.h"

// Constante para fácilitar el muestreo de menajes
#define showText(x) write(1, x, strlen(x))

// Constantes de error
#define ERROR_INVENTORY_FILE 1
#define NO_ERROR 0

// Constantes - Comandos de trade
#define ADD 1
#define REMOVE 2

// Tipo propio que representa un producto así como se representa en el fichero
typedef struct {
    char name[100];
    int amount;
    float weight;
} Product;

// Tipo propio que representa un inventario, con varios productos
typedef struct Inventory {
    Product* products;
    int product_count;
} Inventory;

//------------------------- Private Functions -----------------------------------------

/***********************************************
*
* @Finalitat: Escribe un string en donde apunta el file descriptor.
* @Parametres:      in: fd = el file descriptor que indica a donde escribir.
*                   in: sentence = el string que se quiere escribir.
* @Retorn: ---.
*
************************************************/
void printa (int fd, char* sentence);



//------------------------- Public Functions ------------------------------------------

/***********************************************
*
* @Finalitat: Lee el fichero del inventario y guarda los productos en las estructuras necesarias.
* @Parametres:      in/out: inventory = el inventario que se quiere leer y guardar.
*                   in: file_name = el string que representa el nombre del fichero.
* @Retorn: Devuelve un entero que en 1 indica que ha habido un error en la lectura del fichero y
*           en 0 indica que no ha habido ningún error en la lectura del fichero.
*
************************************************/
int readInventoryFile (Inventory* inventory, char* file_name);

int loadInventoryFromData (Inventory* inventory, char* data, int data_length);

int loadTradeFromData(Inventory* inventory, char* data, int data_length);

/***********************************************
*
* @Finalitat: Actualiza la información de un inventario. Despues de recibir un intercambio
* @Parametres:      in/out: inventory = el inventario que se desea actualizar.
*                   in: product = el producto que se va a añadir o restar.
                    in: option = si se desea añadir o restar.
* @Retorn:  ---
*
************************************************/
void updateStock (Inventory* inventory, Product product, int option);

/***********************************************
*
* @Finalitat: Crea o reescribe un fichero, con la información del comercio que se desea realizar
* @Parametres:      in: inventory = el inventario que se quiere leer y guardar.
*                   in: file_name = el string que representa el nombre del fichero.
* @Retorn:  ---
*
************************************************/
void tradeFile (Inventory inventory, char* file_name);

int getProductPositionByName(char *product_name, Inventory inventory);

/***********************************************
*
* @Finalitat: Lista los productos de un inventario y los muestra por pantalla
* @Parametres:      in: inventory = el inventario que se desea listar
* @Retorn: ---
*
************************************************/
void listProducts(Inventory inventory);

/***********************************************
*
* @Finalitat: Crea una copia con del inventario que con el que se quiere tradear con el amount en 0
* @Parametres: in: avail_inventory = inventario con el que se desea trader
* @Retorn: La copia del inventario con el que se desea tradear con el amount en 0
*
************************************************/
Inventory createTradeInventory (Inventory avail_inventory);

#endif