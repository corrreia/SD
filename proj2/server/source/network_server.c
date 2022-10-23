#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../include/network_server.h"
#include "../include/tree_skel.h"
#include "../include/sdmessage.pb-c.h"



struct sockaddr_in server_addr;
int server_socket;

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port){
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(server_socket == -1){
        perror("Error creating socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        perror("Error binding socket");
        return -1;
    }

    if(listen(server_socket, 0) == -1){
        perror("Error listening to socket");
        return -1;
    }

    return server_socket;
}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */
int network_main_loop(int listening_socket){
    struct sockaddr_in client_addr;
    int client_socket;
    socklen_t client_addr_size = sizeof(client_addr);

    MessageT *msg;
    int response;

    printf("Waiting for connections...\n");

    while(1){
        client_socket = accept(listening_socket, (struct sockaddr *) &client_addr, &client_addr_size);
        if(client_socket == -1){
            perror("Error accepting connection");
            return -1;
        }

        msg = network_receive(client_socket);
        if(msg == NULL){
            perror("Error receiving message");
            return -1;
        }

        response = invoke(msg);
        if(response == -1){
            perror("Error invoking tree_skel");
            return -1;
        }

        if(network_send(client_socket, msg) == -1){
            perror("Error sending response");
            return -1;
        }

        close(client_socket);
    }
    return 0;
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct _MessageT *network_receive(int client_socket){
    int bytes_read;
    int msg_size;
    char *buffer;
    MessageT *msg;

    bytes_read = read(client_socket, &msg_size, sizeof(int));
    if(bytes_read == -1){
        perror("Error reading message size");
        return NULL;
    }

    buffer = malloc(msg_size);
    if(buffer == NULL){
        perror("Error allocating memory for buffer");
        return NULL;
    }

    bytes_read = read(client_socket, buffer, msg_size);
    if(bytes_read == -1){
        perror("Error reading message");
        return NULL;
    }

    msg = message_t__unpack(NULL, msg_size, buffer);
    if(msg == NULL){
        perror("Error unpacking message");
        return NULL;
    }

    free(buffer);
    return msg;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct _MessageT *msg){
    int bytes_written;
    int size = message_t__get_packed_size(msg);
    char *buffer = (char *) malloc(sizeof(char) * size);

    message_t__pack(msg, buffer);

    bytes_written = write(client_socket, buffer, size);

    if(bytes_written == -1){
        perror("Error writing to socket");
        return -1;
    }

    return 0;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close(){
    //close(server_socket);
    //free(server_addr);
    return 0;
}

