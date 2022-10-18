#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
    //if do not have arguments
    if(argc == 1){
        printf("Usage: %s <server>:<port>\n", argv[0]);
        return -1;
    }
    //server ip and port are both on the same string, delimited by ':' 
    char *server_ip = strtok(argv[1], ":");
    char *server_port = strtok(NULL, ":");
    printf("server ip: %s \nserver port: %s \n", server_ip, server_port);
    return 0;
}

