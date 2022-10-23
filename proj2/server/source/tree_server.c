#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/tree_skel.h"
#include "../include/network_server.h"

int main(int argc, char **argv){
    //first arg is the server port

    if(argc != 2){
        printf("Usage: %s <server_port>\n", argv[0]);
        exit(1);
    }

    int server_port = atoi(argv[1]);

    tree_skel_init();
    /* inicialização da camada de rede */
    int socket = network_server_init(server_port);

    if(socket == -1){
        printf("Error initializing network server\n");
        tree_skel_destroy(); //free tree if server fails to initialize 
        return -1;
    }

    printf("TCP socket initialized on port %d\n", server_port);

    if(network_main_loop(socket)){
        printf("Error in main loop\n");
        tree_skel_destroy();
        return -1;
    }

    tree_skel_destroy();
    return 0;
}