/***********************************************
*
* @Proposit: Controlar el formato de todos los comandos y errores relacionados
*               con comandos, ficheros, y todo el muestreo de los errores.
* @Autor/s: Jon Ballanca y Alejandro Cortés
* @Data creacio: 20 / 10 / 2025
* @Data ultima modificacio: 24 / 10 / 2025
*
************************************************/

// Define Guard
#ifndef COMMANDS_H
#define COMMANDS_H

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
#include "printing.h"

// Constante para fácilitar el muestreo de menajes
#define showText(x) write(1, x, strlen(x))

// Constantes - Errors
#define NO_ERROR 0
#define ERROR 1

// Constantes - Comandos principales
#define INVALID_COMMAND 0
#define LIST_REALMS 1
#define PLEDGE 2
#define PLEDGE_RESPOND 3
#define LIST_PRODUCTS 4
#define LIST_PRODUCTS_NO_ARGS 5
#define START_TRADE 6
#define PLEDGE_STATUS 7
#define ENVOY_STATUS 8
#define EXIT 9

// Constantes - Comandos de trade
#define ADD 1
#define REMOVE 2
#define SEND 3
#define CANCEL 4

// Constantes - Maximos y otras representaciones
#define MAX_COMMANDS 10
#define MAX_TRADE_COMMANDS 5
#define FIRST_ARGUMENT 0
#define SECOND_ARGUMENT 1
#define NUM_SIGILS 14

// Constantes - Colores
#define RED "\e[0;31m"
#define RESET "\x1b[0m"
#define BOLD  "\x1b[1m"

// Constantes - Tipo propio de constante (declarado en el .c por limitaciónes)
typedef struct {
    const char* name;
    int numArgs;
    int indexConstant;
    char* no_args_error;
} Command;

// Declaración del tipo propio del MaesterInfo
typedef struct Maester Maester;

// Declaración del tipo propio del Inventory
typedef struct Inventory Inventory;

//------------------------- Private Functions -----------------------------------------

void printUnknownCommand ();

void printIncorrectArgs (int index, int argument_count);

void printUnknownTradeCommand();

void printProductUnavailable (char **arguments);

void printSynthaxError (char* current_command);

int findTradeIndex (char* current_command);

int getProductPosition (Inventory avail_inventory, char** arguments);

int checkAddRemove (Inventory avail_inventory, Inventory trade, char** arguments, int command_index, int product_pos, int index);

int findCommandIndex (char* current_command, int argument_count);

int checkPledge (Maester maester, char** arguments, int index);

int checkPledgeRespond (Maester maester, char **arguments, int index);

/***********************************************
*
* @Finalitat: Convierte las letras de la palabra que se lee pasa a minúsculas.
* @Parametres:      in/out: word = la palabra que se quiere convertir
* @Retorn: ---.
*
************************************************/
void convertToLowercaseLetters (char* word);

/***********************************************
*
* @Finalitat: Escibe por pantalla que el realm que se lee pasa por parámetro no existe.
* @Parametres:      in: realm_name = el nombre del realm que no existe.
* @Retorn: ---.
*
************************************************/
void messageNoExists (char* realm_name);

/***********************************************
*
* @Finalitat: Escibe por pantalla que no hay una alianza con el realm que se pasa por parámetro.
* @Parametres:      in: realm_name = el nombre del realm con lo cual no hay alianza.
* @Retorn: ---.
*
************************************************/
void messageNoAlliance (char* realm_name);

/***********************************************
*
* @Finalitat: Comprueba si el realm que se ha pasado por parámetro existe en las rutas del maester,
*               que también se pasa por parámetro.
* @Parametres:      in: maester = el maester del cual se lee quieren ver las rutas.
*                   in: realm_name = el nombre del realm que se quiere ver si existe, que es realmente
*                        el argumento que se pasa por parámtero en el comando.
*                   in: index = el index del comando que ha llamado esta función para comprobación
* @Retorn: Devuelve un entero que cuando es 1 representa que el realm existe. Cuando es en 0 representa
*           que el realm no existe.
*
************************************************/
int realmExists (Maester maester, char* realm_name, int index);

/***********************************************
*
* @Finalitat: Comprueba si el path del sigil que se ha pasado por parámetro existe en el sistema.
* @Parametres:      in: filename = el nombre del sigil, que al final representa el nombre de su fichero.
* @Retorn: Devuelve un entero que cuando es 1 representa que el sigil existe. Cuando es en 0 representa
*           que el sigil no existe.
*
************************************************/
int sigilExists (char* filename);

/***********************************************
*
* @Finalitat: Comprueba si el argumento que se ha pasado existe realmente como producto en el inventario.
* @Parametres:      in: inventory = el inventario disponible del cual se quieren ver los productos.
*                   in: argument = el nombre del producto que se ha pasado para comprobarse.
* @Retorn: Devuelve un entero que cuando es 1 representa que el producto existe en el inventario pasado. 
*           Cuando es en 0 representa que el producto no existe en el inventario pasado.
*
************************************************/
int productExists (Inventory inventory, char* argument);

/***********************************************
*
* @Finalitat: Comprueba si el producto que se quiere añadir o quitar del "trade" esta dentro de los limites
*               del inventario disponible.
* @Parametres:      in: avail_inventory = el inventario disponible.
*                   in: trade =  el inventario ya selecionado para el "trade"
*                   in: amount_arg = la cantidad del producto que se quiere añadir o quitar. Viene del argumento
*                                       del comando.
*                   in: command_index = el entero que representa si se quiere añadir o quitar el producto desde
*                                       el inventario de "trade".
*                   in: product_arg = el argumento que representa el nombre del producto que se quiere añadir
*                                       o quitar.
* @Retorn: Devuelve un entero que es 1 cuando la cantidad del producto esta dentro de los límites del 
*           inventario disponible y devuelve 0 si la cantidad no esta dentro de los límites.
*
************************************************/
int checkAmountLimit(Inventory avail_inventory, Inventory trade, char* amount_arg, int command_index, int product_pos);

/***********************************************
*
* @Finalitat: Mira si se ha enviado el "trade" pero aún no se ha aceptado o rechazado.
* @Parametres: ---.
* @Retorn: Devuelve un entero que es 1 cuando el "trade" esta pendiente, y 0 cuando no esta pendiente.
*
************************************************/
int tradePending ();

/***********************************************
*
* @Finalitat: Comprueba si la palabra que se la ha pasado es un número.
* @Parametres:      in: word = la palabra que se quiere ver si es un número o no.
* @Retorn: Devuelve un entero que cuando es 1 representa que la palabra es un número. Cuando es en 0
*           representa que el sigil no existe.
*
************************************************/
int isNumber (char* word);


//------------------------- Public Functions ------------------------------------------

int manageCommand (int *word_count, int *argument_count, char **string_read, char ***command_separated, char ***command_args, char **command, Maester maester);

void getCommandInput (char **string_read);

/***********************************************
*
* @Finalitat: Convierte las letras de la palabra que se lee pasa a mayúsculas
* @Parametres:      in/out: word = la palabra que se pasa para ponerse en mayúsculas.
* @Retorn: ---.
*
************************************************/
void convertToCapitalLetters (char* word);

/***********************************************
*
* @Finalitat: Lee donde lee indica el filedescriptor hasta llegar a un delimitador que se puede indicar.
* @Parametres:      in: fd = el filedescriptor que indica de donde leer.
*                   in: delimiter = el delimitador para leer hasta ese punto (sin incluir).
* @Retorn: Devuelve un string de lo que ha leido con el fildescriptor hasta el delimitador.
*
************************************************/
char* readUntil(int fd, char delimiter);

/***********************************************
*
* @Finalitat: Comprueba y muestra por pantalla un mensaje de error en el caso que hay errores de fichero,
*               indicando el fichero. También devuelve un entero que representa si ha habido un error o no.
* @Parametres:      in: error_config_file = el entero que se lee pasa por si ha habido un error en el fichero
*                       de configuración.
*                   in: error_inventory_file = el entero que se lee pasa por si ha habido un error en el fichero
*                       de inventario.
* @Retorn: Devuelve un entero que cuando es 1 indica que ha habido un error en la lectura de ficheros. Cuando
*           es 0 indica que no ha habido ningun error en el fichero.
*
************************************************/
int checkFileErrors (int error_config_file, int error_inventory_file);

/***********************************************
*
* @Finalitat: Separa el string del comando a un array de palabras, comando y argumento.
* @Parametres:      in/out: command_separated = el comando separado en palabras de comando y agumento.
*                   in: string_read = el comando en formato string que se ha leido desde la terminal.
* @Retorn: El numero de palabras en el comando.
*
************************************************/
int separateCommand(char ***command_separated, char *string_read);

/***********************************************
*
* @Finalitat: Comprueba, guarda y separa lo que es un comando y lo que son los argumentos.
* @Parametres:      in/out: command = el string donde se guarda el comando como palabra.
*                   in/out: command_args = el array de strings que guarda todos los argumentos que se
*                           han escrito por la terminal.
*                   in: command_separated = el string entrado por la terminal, separado en palabras.
*                   in: word_count = el número de palabras que contiene el command_separated.
* @Retorn: El numero de argumentos que se han encontrado.
*
************************************************/
int separateArguments (char** command, char*** command_args, char** command_separated, int word_count);

/***********************************************
*
* @Finalitat: Guarda y separa lo que es un comando de la interfaz de "trade" y lo que son
*               los argumentos.
* @Parametres:      in/out: command = el string donde se guarda el comando del "trade" como palabra.
*                   in/out: command_args = el array de strings que guarda todos los argumentos que se
*                           han escrito por la terminal, en la interfaz de "trade".
*                   in: command_separated = el string entrado por la terminal, separado en palabras.
*                   in: word_count = el número de palabras que contiene el command_separated.
* @Retorn: El numero de argumentos que se han encontrado.
*
************************************************/
int separateTradeArgs (char** command, char*** command_args, char** command_separated, int word_count);

/***********************************************
*
* @Finalitat: Comprueba y llama las funciones necesarias para hacer un control completo del comando
*               que se ha entrado por la terminal para el caso de la interfaz del "trade". Devuele
*               un entero que representa un comando, o un comando inválido de la interfaz "trade".
*               Mira si el producto existe, si la cantidad entrada es adecuada, si el número de
*               argumentos es el adecuado para el comando. Hace TODO el control de errores.
* @Parametres:      in: currentCommand = el comando separado como palabra.
*                   in: arguments = un array de palabras que representan los argumentos del comando.
*                   in: avail_inventory = el inventario disponible del realm con lo cual se esta
*                       haciendo el trade.
*                   in: trade = el inventario acumulado del trade.
*                   in: argument_count = el número de argumentos que tiene el comando.
*                   in: maester = el maester que ha pedido esta comprobación del comando.
* @Retorn: Devuele un entero que representa un comando, o un comando invalido de la interfaz "trade".
*
************************************************/
int getTradeIndex (char* currentCommand, char** arguments, Inventory avail_inventory, Inventory trade, int argument_count);

/***********************************************
*
* @Finalitat: Comprueba y llama las funciones necesarias para hacer un control completo del comando
*               que se ha entrado por la terminal. Devuele un entero que representa un comando, o un 
*               comando inválido. Mira si el realm existe, o cualquier control de los argumentos. Hace 
*               TODO el control de errores.
* @Parametres:      in: currentCommand = el comando separado como palabra.
*                   in: arguments = un array de palabras que representan los argumentos del comando.
*                   in: argument_count = el número de argumentos que tiene el comando.
*                   in: maester = el maester que ha pedido esta comprobación del comando.
* @Retorn: Devuele un entero que representa un comando, o un comando invalido.
*
************************************************/
int getCommandIndex (char* currentCommand, char** arguments, int argument_count, Maester maester);

#endif