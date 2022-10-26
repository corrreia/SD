#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/sdmessage.pb-c.h"

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t *rtree){

    rtree->socket = socket(AF_INET, SOCK_STREAM, 0);
    if(rtree->socket < 0){
        perror("Error creating socket");
        free(rtree);
        return -1;
    }

    if(connect(rtree->socket, (struct sockaddr *) &rtree->server, sizeof(rtree->server)) < 0){
        perror("Error connecting to server");
        close(rtree->socket);
        free(rtree);
        return -1;
    }

    printf("Connection established!!\n");
    
    return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtree_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct _MessageT *network_send_receive(struct rtree_t * rtree, struct _MessageT *msg){
    int socket_fd = rtree->socket;

    //serialize msg
    size_t msg_size = message_t__get_packed_size(msg);
    uint8_t *msg_buffer = (uint8_t *) malloc(msg_size);
    message_t__pack(msg, msg_buffer);

    //send msg
    if(send(socket_fd, msg_buffer, msg_size, 0) < 0){
        perror("Error sending message network\n");
        return NULL;
    }

    //receive response
    uint8_t *response_buffer = (uint8_t *) malloc(1000);
    if(recv(socket_fd, response_buffer, 1000, 0) < 0){
        perror("Error receiving response\n");
        return NULL;
    }

    //deserialize response
    struct _MessageT *response = message_t__unpack(NULL, 1000, response_buffer);

    if(response == NULL){
        perror("Error unpacking response\n");
        return NULL;
    }

    return response;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t * rtree){
    if(rtree == NULL){
        return -1;
    }
    
    if(close(rtree->socket) < 0){
        return -1;
    }

    free(rtree);
    return 0;
}
