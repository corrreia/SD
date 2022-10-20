#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/data.h"

int main(int argc, char **argv){
    if(argc == 1){ // No arguments
        printf("Usage: %s <server>:<port>\n", argv[0]);
        return -1;
    }
    
    char *server_ip = strtok(argv[1], ":");
    char *server_port = strtok(NULL, ":");

    printf("server ip: %s \nserver port: %s \n", server_ip, server_port);

    printf("Connecting to server...\n");

    struct rtree_t *rtree = rtree_connect(argv[1]);

    if(rtree == NULL){
        printf("Error connecting to server (timeout)\n");
        return -1;
    }

    printf("Connected to server\n");


    char *command = NULL;
    size_t command_size = 0;
    while(1){
        printf(">> ");

        if(getline(&command, &command_size, stdin) == 0){
            printf("Error reading command\n");
        }

        command[strlen(command) - 1] = '\0'; // remove newline

        //quit
        if(strcmp(command, "quit") == 0){
            break;  // exit loop
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

            struct data_t *data_t = data_create2(strlen(data), data);
            struct entry_t *entry_t = entry_create(key, data_t);
            if(rtree_put(rtree, entry_t) == -1){
                printf("Error putting entry\n");
            }
            else{
                printf("Entry successfully put\n");
            }
        }

        // get <key>
        else if(strncmp(command, "get", 3) == 0){
            char *key = strtok(command, " ");
            key = strtok(NULL, " ");

            if(key == NULL){
                printf("Usage: get <key>\n");
                continue;
            }

            struct data_t *data_t = rtree_get(rtree, key);
            if(data_t == NULL){
                printf("Error getting entry\n");
            }
            else{
                printf("Entry successfully got\n");
                printf("Data: %p\n", data_t->data);
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

            if(rtree_del(rtree, key) == -1){
                printf("Error deleting entry\n");
            }
            else{
                printf("Entry successfully deleted\n");
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

        // getkeys
        else if(strcmp(command, "getkeys") == 0){
            char **keys = rtree_get_keys(rtree);
            if(keys == NULL){
                printf("Error getting keys\n");
            }
            else{
                printf("Keys: ");
                for(int i = 0; keys[i] != NULL; i++){
                    printf("%p ", keys[i]);
                }
                printf("\n");
            }
        }

        // getvalues
        else if(strcmp(command, "getvalues") == 0){
            struct data_t **values = (struct data_t **) rtree_get_values(rtree);
            if(values == NULL){
                printf("Error getting values\n");
            }
            else{
                printf("Values: ");
                for(int i = 0; values[i] != NULL; i++){
                    printf("%p ", values[i]->data);
                }
                printf("\n");
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
            printf("quit\n");
        }

        else{
            printf("Unknown command. Type 'help' for a list of commands.\n");
        }
    }

    free(command);

    printf("Disconnecting from server...\n");
    if(rtree_disconnect(rtree) != 0){
        printf("Error disconnecting from server\n");
        return -1;
    }
    printf("Disconnected from server\n");

    return 0; // success
}

