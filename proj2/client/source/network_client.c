#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
int network_connect(struct rtree_t *rtree){  //FIXME: This needs to be changed 
    struct sockaddr_in server_addr;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1){
        printf("Error creating TCP socket");
        return -1;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(rtree->port);
    server_addr.sin_addr.s_addr = inet_addr(rtree->address);
    if(connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        printf("Error connecting to server");
        return -1;
    }
    rtree->socket = socket_fd;
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
struct message_t *network_send_receive(struct rtree_t * rtree, struct message_t *msg){
    return NULL;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t * rtree){
    return 0;
}
