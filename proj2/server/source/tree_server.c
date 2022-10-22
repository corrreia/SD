#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
    //first arg is the server port

    if(argc != 2){
        printf("Usage: %s <server_port>\n", argv[0]);
        exit(1);
    }

    int server_port = atoi(argv[1]);

    
}