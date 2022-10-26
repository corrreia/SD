#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#include "../include/network_server.h"
#include "../include/tree_skel.h"
#include "../include/sdmessage.pb-c.h"



struct sockaddr_in server_addr;
int server_socket;

void ctrl_c_handler(int signum){
    printf("CTRL-C detected, closing...\n");
    network_server_close();
    exit(0);
}

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port){
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
        printf("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

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
    struct sockaddr client_addr;
    int client_socket;
    socklen_t client_addr_size;// = sizeof(client_addr);

    signal(SIGINT, ctrl_c_handler);

    printf("Waiting for connections...\n");

    while((client_socket = accept(listening_socket, (struct sockaddr *) &client_addr, &client_addr_size)) != -1){
        printf("Connection accepted\n");
    
        int receiving_info = 0;
        while(receiving_info != 1){

            struct _MessageT *message = network_receive(client_socket);

            // caso para o quit
            // if(message == NULL){
            //     printf("Client Disconnected!\n");
            //     receiving_info = 1;
            //     close(client_socket);
            //     continue;
            // }
        
            invoke(message);

            int value_send = network_send(client_socket,message);

            if(value_send == -1){
                close(client_socket);
                return -1;
            }
        }

        close(client_socket);
        printf("Connection closed\n");
    }

    return 0;
}


/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct _MessageT *network_receive(int client_socket){
    int len_received;
    int value;
    if( (value = read(client_socket,&len_received, sizeof(int))) == 0){
        return NULL;
    }

    len_received = ntohl(len_received);

    uint8_t str [len_received];

    int amount_read = 0;
    int result;

    while(amount_read < len_received){
        result = read(client_socket, str + amount_read, len_received - amount_read);
        if(result < 1){
            break;
        }
        amount_read += result;
    }

    str[len_received] = '\0';

    struct Message_T *msg = message_t__unpack(NULL,len_received,str);

    return msg;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct _MessageT *msg){
    int len = message_t__get_packed_size(msg);
    uint8_t str[len];

    message_t__pack(msg,str);

    len = htonl(len);

    int value = write(client_socket,&len,sizeof(int));

    if(value == -1){
        perror("Error writing to socket");
        return -1;
    }

    value = write(client_socket,str,sizeof(str));

    if(value == -1){
        perror("Error writing to socket");
        return -1;
    }

    return 0;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close(){
    close(server_socket);
    //free(server_addr); wtf
    return 0;
}

