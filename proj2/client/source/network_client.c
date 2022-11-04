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
MessageT *network_send_receive(struct rtree_t * rtree, MessageT *msg){
    int socket_fd = rtree->socket;

    int msg_size = message_t__get_packed_size(msg);
    int msg_size_n = htonl(msg_size);

    uint8_t *buffer = (uint8_t *) calloc(msg_size, sizeof(char));

    message_t__pack(msg, buffer);

    int w = write(socket_fd, &msg_size_n, sizeof(int));
    if(w < 0) return NULL;
    write_all(socket_fd, buffer, msg_size);

    if(msg != NULL) message_t__free_unpacked(msg, NULL);
    free(buffer);

    int len_response;
    int r = read(socket_fd, &len_response, sizeof(int));
    if (r < 0) return NULL;
    len_response = ntohl(len_response);

    uint8_t *buffer_response = (uint8_t *) calloc(len_response, sizeof(char));
    read_all(socket_fd, buffer_response, len_response);

    //add /0 to the end of the buffer
    //buffer_response[len_response] = '\0'; //* not needed

    msg = message_t__unpack(NULL, len_response,buffer_response);
    free(buffer_response);
    return msg;
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
