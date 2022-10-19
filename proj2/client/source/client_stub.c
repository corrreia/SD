#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "../include/data.h"
#include "../include/entry.h"
#include "../include/client_stub.h"
#include "../include/client_stub-private.h"

/* Remote tree. A definir pelo grupo em client_stub-private.h
 */
//struct rtree_t;


/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port){

    struct rtree_t *rtree = (struct rtree_t *) malloc(sizeof(struct rtree_t));

    if(rtree == NULL){
        printf("Error allocating memory for rtree");
    }


    // char *ap_copy = strdup(address_port);    //tranform const char * in char *
    // char *address = strtok(ap_copy, ":");   //FIXME: WHY DOES THIS NOT WORK?
    // char *port = strtok(NULL, ":");

    char *address = (char *) malloc(sizeof(char) * 100);
    char *port = (char *) malloc(sizeof(char) * 100);

    int i = 0;
    while(address_port[i] != ':'){
        address[i] = address_port[i];
        i++;
    }
    address[i] = '\0';
    i++;
    int j = 0;
    while(address_port[i] != '\0'){
        port[j] = address_port[i];
        i++;
        j++;
    }
    port[j] = '\0';

    rtree->socket = socket(AF_INET, SOCK_STREAM, 0);

    if(rtree->socket == -1){
        printf("Error creating TCP socket");
        return NULL;
    }

    rtree->server.sin_family = AF_INET;
    rtree->server.sin_port = htons(atoi(port));

    if (inet_pton(AF_INET, address, &rtree->server.sin_addr) < 1) { // Endereço IP
        printf("Erro ao converter IP\n");
        close(rtree->socket);
        return NULL;
    }

    if(connect(rtree->socket, (struct sockaddr *) &rtree->server, sizeof(rtree->server)) < 0){
        close(rtree->socket);
        return NULL;
    }

    return rtree;
}

/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree){
    if(close(rtree->socket) < 0){
        return -1;
    }
    free(rtree);
    return 0;
}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t *rtree, struct entry_t *entry){
    return 0;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key){
    return NULL;
}

/* Função para remover um elemento da árvore. Vai libertar 
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key){
    return 0;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree){
    return 0;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree){
    return 0;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree){
    return NULL;
}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void **rtree_get_values(struct rtree_t *rtree){
    return NULL;
}