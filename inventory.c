#include "inventory.h"

/* Fase 3
void evaluateOrder (Inventory inventory, char *response) {
    
}*/

void printa (int fd, char* sentence) {
    write (fd, sentence, strlen(sentence));
}

int readInventoryFile (Inventory* inventory ,char* file_name) {
    int fd = 0;             //Numero del file descriptor del fichero
    int read_bytes = 0;     //Numero de bytes leidos
    int result;             //Resultado

    Product product;        //Producto auxliar 

    inventory->product_count = 0;
    
    fd = open (file_name, O_RDONLY);

    if (fd < 0) {
        //Error reading file
        showText("Error reading inventory file\n");
        result = ERROR_INVENTORY_FILE;
    } else {
        //Reading file
        do {
            read_bytes = read (fd, &product, sizeof(Product));

            if (read_bytes > 0) {

                inventory->product_count ++;

                if (inventory->product_count == 1) {

                    inventory->products = (Product*) malloc (sizeof(Product)*inventory->product_count);

                } else {
                    inventory->products = (Product*) realloc (inventory->products, sizeof(Product)*inventory->product_count);
                }
                
                strcpy(inventory->products[inventory->product_count - 1].name, product.name);
                inventory->products[inventory->product_count - 1].amount = product.amount;
                inventory->products[inventory->product_count - 1].weight = product.weight;

            }

        } while (read_bytes > 0);
        result = NO_ERROR;
        close(fd);
    }


    return result;
}

int loadInventoryFromData (Inventory* inventory, char* data, int data_length) {
    int bytes_read;
    int result;
    int i;
    Product product;
    char* product_bytes;
    
    inventory->product_count = 0;
    bytes_read = 0;
    
    if (data == NULL || data_length <= 0) {
        showText("Error loading inventory from buffer\n");
        result = ERROR_INVENTORY_FILE;
    } else {
        while (bytes_read < data_length) {
            
            product_bytes = (char*) &product;
            for (i = 0; i < (int)sizeof(Product); i++) {
                product_bytes[i] = data[bytes_read + i];
            }
            
            bytes_read = bytes_read + sizeof(Product);
            
            inventory->product_count++;
            
            if (inventory->product_count == 1) {
                inventory->products = (Product*) malloc(sizeof(Product) * inventory->product_count);
            } else {
                inventory->products = (Product*) realloc(inventory->products, sizeof(Product) * inventory->product_count);
            }
            
            strcpy(inventory->products[inventory->product_count - 1].name, product.name);
            inventory->products[inventory->product_count - 1].amount = product.amount;
            inventory->products[inventory->product_count - 1].weight = product.weight;
        }
        result = NO_ERROR;
    }
    
    return result;
}

int loadTradeFromData(Inventory* inventory, char* data, int data_length) {
    int i;
    int j;
    int line_length;
    int name_length;
    int name_position;
    int amount;
    int in_product_section;
    int result;
    char* line_start;
    char* line_end;
    char* pipe_pos;
    char product_name[256];
    
    i = 0;
    j = 0;
    line_start = data;
    line_end = NULL;
    pipe_pos = NULL;
    amount = 0;
    in_product_section = 0;
    
    inventory->product_count = 0;
    inventory->products = NULL;
    
    if (data == NULL || data_length <= 0) {
        showText("Error loading trade from buffer\n");
        result = ERROR_INVENTORY_FILE;
    } else {
        for (i = 0; i <= data_length; i++) {
            if (i == data_length || data[i] == '\n') {
                line_end = &data[i];
                line_length = line_end - line_start;
                
                if (line_length > 3 && line_start[0] == '-' && line_start[1] == '-') {
                    in_product_section = !in_product_section;
                } else if (in_product_section && line_length > 0) {
                    pipe_pos = NULL;
                    for (j = 0; j < line_length; j++) {
                        if (line_start[j] == '|') {
                            pipe_pos = &line_start[j];
                            break;
                        }
                    }
                    
                    if (pipe_pos != NULL) {
                        name_length = pipe_pos - line_start;
                        name_position = 0;
                        
                        for (j = 0; j < name_length && name_position < 255; j++) {
                            if (line_start[j] != '\t' && (line_start[j] != ' ' || name_position > 0)) {
                                product_name[name_position++] = line_start[j];
                            }
                        }
                        while (name_position > 0 && product_name[name_position-1] == ' ') {
                            name_position--;
                        }
                        product_name[name_position] = '\0';
                        
                        amount = atoi(pipe_pos + 1);
                        
                        if (amount > 0 && name_position > 0) {
                            inventory->product_count++;
                            
                            if (inventory->product_count == 1) {
                                inventory->products = (Product*) malloc(sizeof(Product) * inventory->product_count);
                            } else {
                                inventory->products = (Product*) realloc(inventory->products, sizeof(Product) * inventory->product_count);
                            }
                            
                            strcpy(inventory->products[inventory->product_count - 1].name, product_name);
                            inventory->products[inventory->product_count - 1].amount = amount;
                            inventory->products[inventory->product_count - 1].weight = 0.0;
                        }
                    }
                }
                
                line_start = &data[i + 1];
            }
        }
        result = NO_ERROR;
    }
    
    return result;
}

void updateStock (Inventory* inventory, Product product, int option) {
    int i = 0;              // Contador
    int found = 0;          //Flag que indica si se ha encontrado o no un producto

    //Bucle para encontar el producto para poder añadirlo o restarlo
    for (i = 0; i < inventory->product_count && !found; i++) {
        if (strcmp (inventory->products[i].name, product.name) == 0) {
            found = 1;

            if (option == ADD) {
                inventory->products[i].amount += product.amount;

            } else {
                if (option == REMOVE) {
                    inventory->products[i].amount -= product.amount;

                    if (inventory->products[i].amount < 0) {
                        inventory->products[i].amount = 0;
                    }
                }
            }
        }
    }
    
}

void tradeFile (Inventory inventory, char* file_name) {
    int fd = 0;         //Numero del file descriptor del fichero
    int i = 0;          //Contador
    int buffer_size;    //Tamaño del buffer

    char* buffer;       //Variable auxiliar para printar por pantalla

    fd = open(file_name ,O_WRONLY|O_CREAT|O_TRUNC,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd < 0) {
        showText ("Error opening send file\n");
    } else {
        printa (fd, "--- Trade File ---\n");
        printa (fd, "Item\t\t\t| Amount\n");
        printa (fd, "----------------------------------------\n");    

        //Crea un fichero con la información del inventario puesta
        for (i = 0; i < inventory.product_count; i++) {
            buffer_size = asprintf(&buffer, "%s", inventory.products[i].name);
            write (fd, buffer, buffer_size);
            free (buffer);

            if (strlen(inventory.products[i].name) < 8) {
                write(fd, "\t\t\t", strlen("\t\t\t"));
            } else if ((strlen(inventory.products[i].name) >= 8) && (strlen(inventory.products[i].name) < 16)) {
                write(fd, "\t\t", strlen("\t\t"));
            } else if ((strlen(inventory.products[i].name) >= 16) && (strlen(inventory.products[i].name) < 24)) {
                write(fd, "\t", strlen("\t"));
            }

            buffer_size = asprintf(&buffer, "| %d\t\n", inventory.products[i].amount);
            write (fd, buffer, buffer_size);
            free (buffer);
        }
        
        printa (fd, "----------------------------------------\n");
        
        buffer_size = asprintf (&buffer, "Total Entries: %d\n\n", inventory.product_count);
        write (fd, buffer, buffer_size);
        free (buffer);

        
    }

}

int getProductPositionByName (char *product_name, Inventory inventory) {
    int i = 0;
    char *upper_case1;
    char *upper_case2;
    int position = -1;
    
    upper_case2 = malloc(sizeof(char) * (strlen(product_name) + 1));
    strcpy(upper_case2, product_name);
    convertToCapitalLetters(upper_case2);
    
    for (i = 0; i < inventory.product_count; i++) {
        upper_case1 = malloc(sizeof(char) * (strlen(inventory.products[i].name) + 1));
        strcpy(upper_case1, inventory.products[i].name);
        convertToCapitalLetters(upper_case1);
        
        if (!strcmp(upper_case1, upper_case2)) {
            position = i;
            free(upper_case1);
            break;
        }
        
        free(upper_case1);
    }
    
    free(upper_case2);
    return position;
}

void listProducts(Inventory inventory) {
    int i = 0;                  //Contadores
    int buffer_size;            //Tamaño del buffer 

    char* buffer;               //Variable auxiliar para printar

    printReset ("\n--- Trade Ledger ---\n");
    printReset ("Item\t\t\t| Amount \t| Weight (Stone)\n");
    printReset ("--------------------------------------------------------\n");    

    //Muestra los productos
    for (i = 0; i < inventory.product_count; i++) {
        buffer_size = asprintf(&buffer, RESET "%s", inventory.products[i].name);
        write (1, buffer, buffer_size);
        free (buffer);

        if (strlen(inventory.products[i].name) < 8) {
            printReset("\t\t\t");
        } else if ((strlen(inventory.products[i].name) >= 8) && (strlen(inventory.products[i].name) < 16)) {
            printReset("\t\t");
        } else if ((strlen(inventory.products[i].name) >= 16) && (strlen(inventory.products[i].name) < 24)) {
            printReset("\t");
        }

        buffer_size = asprintf(&buffer, RESET "| %d\t\t| %.1f\n", inventory.products[i].amount, inventory.products[i].weight);
        write (1, buffer, buffer_size);
        free (buffer);
    }
    
    printReset ("--------------------------------------------------------\n");
    
    buffer_size = asprintf (&buffer, "Total Entries: %d\n\n", inventory.product_count);
    write (1, buffer, buffer_size);
    free (buffer);

}

/***********************************************
*
* @Finalitat: Crea una copia con del inventario que con el que se quiere tradear con el amount en 0
* @Parametres: in: avail_inventory = inventario con el que se desea trader
* @Retorn: La copia del inventario con el que se desea tradear con el amount en 0
*
************************************************/
Inventory createTradeInventory (Inventory avail_inventory) {
    Inventory trade;    // Inventario que se crea y representa el trade del start trade
    int i = 0;          // Contador

    trade.products = (Product*)malloc(sizeof(Product) * avail_inventory.product_count);

    for (i = 0; i < avail_inventory.product_count; i++) {
        strcpy(trade.products[i].name, avail_inventory.products[i].name);
        trade.products[i].amount = 0;
        trade.products[i].weight = avail_inventory.products[i].weight;
    }

    return trade;
}