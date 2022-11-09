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
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

#include "../include/network_server.h"
#include "../include/tree_skel.h"
#include "../include/sdmessage.pb-c.h"
#include "../include/message-private.h"

#define NFDESC 6 // numero de sockets (uma para o listening) 1 para o listening e 5 para os clientes

struct sockaddr_in server_addr;
int server_socket;

void ctrl_c_handler(int signum){
    printf("CTRL-C detected, closing...\n");
    network_server_close();
    tree_skel_destroy();
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

    signal(SIGINT,ctrl_c_handler);
    printf("Waiting for connections...\n");

    struct sockaddr_in client; // estrutura para guardar o endereço do cliente
    socklen_t size_client = sizeof(struct sockaddr); // tamanho da estrutura do cliente

    struct pollfd connections[NFDESC];

    for(int i = 0; i < NFDESC; i++) connections[i].fd = -1;  // inicializar a estrutura de poll (coloque todos os descritores a -1 para indicar que não estão a ser usados)    
    connections[0].fd = listening_socket;  // colocar o descritor do socket de listening na estrutura de poll (posição 0)
    connections[0].events = POLLIN; // colocar o evento que se pretende verificar no descritor (POLLIN - para verificar se é possível ler sem bloquear)

    int nfds = 1; // número de descritores a verificar (inicialmente apenas o listening socket)

    int kfds; // número de descritores prontos para ler

    while((kfds = poll(connections, nfds, 50)) >= 0 ){

        if(kfds < 0){ //error checking
            perror("Error polling");
            return -1;
        }

        if((connections[0].revents & POLLIN) && (nfds < NFDESC)){  // se o listening socket está pronto a ler e ainda há espaço na estrutura de poll
            if((connections[nfds].fd = accept(connections[0].fd,(struct sockaddr *) &client, &size_client)) > 0){ // aceitar a ligação do cliente
                connections[nfds].events = POLLIN; // colocar o evento que se pretende verificar no descritor (POLLIN - para verificar se é possível ler sem bloquear)
                //getsockname(nfds, (struct sockaddr *) &client, &size_client); // obter o endereço do cliente
                printf("Client %d connected - IP: %s:%d\n", nfds, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                nfds++;
            }
        }
        
        for(int i = 1; i < nfds; i++){

            if(connections[i].revents & POLLIN){

                 MessageT *message = network_receive(connections[i].fd); // receber a mensagem do cliente

                if(message == NULL){
                    //printf("NULL message\n"); 
                    close(connections[i].fd);
                    connections[i].fd = -1;
                }
                
                invoke(message);

                int value_send = network_send(connections[i].fd ,message);
                message_t__free_unpacked(message, NULL);

                if(value_send == -1){
                    break;
                }
            }
        }

        for(int i = 1 ; i < nfds ;i++){  //se algum cliente fechou a ligação, seu socket fica com valor -1, logo o valor de nfds diminui
            if(connections[i].fd == -1){
                connections[i].fd = connections[nfds].fd;
                nfds --;
            }
        }
    }
    close(listening_socket);
    return 0;
    
}


/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
MessageT *network_receive(int client_socket){
    int length = 0;
    int i = read(client_socket, &length, sizeof(int));
    if(i == -1) return NULL;

    length = ntohl(length);
    if(length == 0) return NULL; //* this sould never happen

    uint8_t *buffer = (uint8_t *) calloc(length, sizeof(char));

    read_all(client_socket, buffer, length);

    MessageT *message = message_t__unpack(NULL, length, buffer);

    free(buffer);
    return message;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, MessageT *msg){
    if(msg == NULL) return -1;
    if(client_socket == -1) return -1;

    int length = message_t__get_packed_size(msg);
    uint8_t *buffer = (uint8_t *) malloc(length);
    message_t__pack(msg, buffer);

    length = htonl(length);
    int i = write(client_socket, &length, sizeof(int));
    length = ntohl(length);

    write_all(client_socket, buffer, length);
    free(buffer);
    return i;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close(){
    close(server_socket);
    return 0;
}

