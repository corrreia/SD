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
#include "../include/message-private.h"

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

    int msg_size = message_t__get_packed_size(msg);
    int msg_size_n = htonl(msg_size);

    uint8_t *msg_serialized = (uint8_t *) malloc(msg_size);

    message_t__pack(msg, msg_serialized);

    int w = write(socket_fd, &msg_size_n, sizeof(int));
    if(w < 0) return NULL;
    write_all(socket_fd, msg_serialized, msg_size);

    free(msg_serialized);

    int len_received;
    int r = read(socket_fd, &len_received, sizeof(int));
    if (r < 0) return NULL;
    len_received = ntohl(len_received);

    uint8_t *str = (uint8_t *) malloc(len_received);
    read_all(socket_fd, str, len_received);

    str[len_received] = '\0';
    
    struct _MessageT *response = message_t__unpack(NULL, len_received, str);

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
