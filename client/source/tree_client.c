/* -------------------------------------------------------------
* Grupo: 49
* Membros: Miguel Pato, fc57102
*          Tomás Correia, fc56372
*          João Figueiredo, fc53524
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/data.h"

struct rtree_t *rtree;
char *command = NULL;

void handle_exit(){
    printf("CTRL-C detected, closing...\n");
    rtree_disconnect(rtree);
    free(command);
    exit(0);
}

int main(int argc, char **argv){
    if(argc != 2){ // No arguments
        printf("Usage: %s <server>:<port>\n", argv[0]);
        return -1;
    }

    printf("Connecting to server...\n");

    rtree = rtree_connect(argv[1]);
    if(rtree == NULL){
        printf("Error connecting to server\n");
        return -1;
    }

    //capture CTRL-C
    signal(SIGINT, handle_exit);
    //signal() para ignorar sinais do tipo SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    size_t command_size = 0;
    while(1){
        printf(">> ");

        if(getline(&command, &command_size, stdin) == 0){
            printf("Error reading command\n");
        }

        command[strlen(command) - 1] = '\0'; // remove newline

        //quit
        if(strcmp(command, "quit") == 0){
            free(command);
            break;  // exit loop
        }

        // getkeys
        else if(strcmp(command, "getkeys") == 0){
            char **keys = rtree_get_keys(rtree);
            if(keys == NULL){
                printf("Error getting keys\n");
            }
            else{
                printf("Keys: ");
                int i = 0;
                while(keys[i] != NULL){
                    printf("%s ", keys[i]);
                    free(keys[i]); // free each key
                    i++;
                }
                printf("\n");
                free(keys); // free array of keys

            }
        }

        // getvalues
        else if(strcmp(command, "getvalues") == 0){
            char **values = (char **) rtree_get_values(rtree);
            if(values == NULL){
                printf("Error getting values\n");
            }
            else{
                printf("Values: ");
                int i = 0;
                while(values[i] != NULL){
                    printf("%s ", values[i]);
                    free(values[i]); // free each value
                    i++;
                }
                printf("\n");
                free(values); // free array of values
            }
        }
        
        //put <key> <data>
        else if(strncmp(command, "put", 3) == 0){
            char *key = strtok(command, " ");
            key = strtok(NULL, " ");
            char *data = strtok(NULL, " ");

            if(key == NULL || data == NULL){
                printf("Usage: put <key> <data>\n");
                continue;
            }

            struct data_t *data_t = data_create(strlen(data)+1);
            memcpy(data_t->data, data, strlen(data)+1);
            struct entry_t *entry_t = entry_create(strdup(key), data_t);
            
            int result = rtree_put(rtree, entry_t);
            if(result == -1){
                printf("Error putting entry\n");
            }
            else{
                printf("Entry put, OP: %d\n", result);
            }
            entry_destroy(entry_t);
        }

        // get <key>
        else if(strncmp(command, "get", 3) == 0){
            char *key = strtok(command, " ");
            key = strtok(NULL, " ");

            if(key == NULL){
                printf("Usage: get <key>\n");
                continue;
            }

            struct data_t *data_t = rtree_get(rtree, strdup(key));
            if(data_t == NULL){
                printf("Error getting entry\n");
            }else if(data_t->datasize == 0){
                printf("Entry not found\n");
            }else{
                printf("Entry successfully found\n");
                printf("Datasize: %d\n", data_t->datasize);
                printf("Data: %s\n",(char *)data_t->data);  
                data_destroy(data_t);  
            }
        }

        // del <key>
        else if(strncmp(command, "del", 3) == 0){
            char *key = strtok(command, " ");
            key = strtok(NULL, " ");

            if(key == NULL){
                printf("Usage: del <key>\n");
                continue;
            }

            int result = rtree_del(rtree, strdup(key));
            if(result == -1){
                printf("Error deleting entry\n");
            }
            else{
                printf("Entry deleted, OP: %d\n", result);
            }
        }

        // size
        else if(strcmp(command, "size") == 0){
            int size = rtree_size(rtree);
            if(size == -1){
                printf("Error getting size\n");
            }
            else{
                printf("Size: %d\n", size);
            }
        }

        // height
        else if(strcmp(command, "height") == 0){
            int height = rtree_height(rtree);
            if(height == -1){
                printf("Error getting height\n");
            }
            else{
                printf("Height: %d\n", height);
            }
        }

        //verify <op_n>
        else if(strncmp(command, "verify", 6) == 0){
            char *op_n = strtok(command, " ");
            op_n = strtok(NULL, " ");

            if(op_n == NULL){
                printf("Usage: verify <op_n>\n");
                continue;
            }

            int op_n_int = atoi(op_n);
            int verify = rtree_verify(rtree, op_n_int);
            //1 if done, 0 if not
            if(verify == 0){
                printf("Operation not processed\n");
            }
            else{
                printf("Operation already processed\n");
            }
        }


        else if(strcmp(command, "help") == 0){
            printf("Avaliable commands:\n");
            printf("put <key> <data>\n");
            printf("get <key>\n");
            printf("del <key>\n");
            printf("size\n");
            printf("height\n");
            printf("getkeys\n");
            printf("getvalues\n");
            printf("verify <op_n>\n");
            printf("quit\n");
        }

        else{
            printf("Unknown command. Type 'help' for a list of commands.\n");
        }
    }


    printf("Disconnecting from server...\n");
    if(rtree_disconnect(rtree) != 0){
        printf("Error disconnecting from server\n");
        return -1;
    }
    printf("Disconnected from server\n");

    return 0; // success
}

