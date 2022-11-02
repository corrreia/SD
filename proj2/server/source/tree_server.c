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

    int socket = network_server_init(server_port);

    if(socket == -1){
        printf("Error initializing server\n");
        network_server_close();
        tree_skel_destroy();
        exit(1);
    }

    printf("TCP socket initialized on port %d\n", server_port);

    if(network_main_loop(socket)){
        printf("Error in main loop\n");
    }

    network_server_close();
    tree_skel_destroy();
    return 0;
}