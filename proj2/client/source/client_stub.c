#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../include/data.h"
#include "../include/entry.h"
#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/network_client.h"
#include "../include/sdmessage.pb-c.h"

/* Remote tree. A definir pelo grupo em client_stub-private.h
 */
//struct rtree_t;


/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port){  

    if(address_port == NULL){
        return NULL;
    }

    struct rtree_t *rtree = (struct rtree_t *) malloc(sizeof(struct rtree_t));

    char *ap_copy = strdup(address_port);

    if(ap_copy == NULL){
        return NULL;
    }

    char *address = strtok(ap_copy, ":");   //FIXME: Segmentation fault
    char *port = strtok(NULL, "\0");

    printf("address: %s \nport: %s \n", address, port);

    //populate rtree struct and call network_connect()

    if(rtree == NULL){
        printf("Error allocating memory for rtree");
        free(ap_copy);
        return NULL;
    }    

    rtree->server.sin_family = AF_INET;
    rtree->server.sin_port = htons(atoi(port));

    if(inet_pton(AF_INET, address, &rtree->server.sin_addr) <= 0){
        printf("Error converting address");
        free(ap_copy);
        free(rtree);
        return NULL;
    }

    if(network_connect(rtree) < 0){
        //printf("Error connecting  to server\n");
        close(rtree->socket);
        return NULL;
    }

    free(ap_copy);

    return rtree;
}

/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree){
    if(rtree == NULL){
        //printf("Error: rtree is NULL");
        return -1;
    }

    if(network_close(rtree) < 0){
        //printf("Error closing connection");
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
    if(rtree == NULL || entry == NULL){
        //printf("Error: rtree or entry is NULL");
        return -1;
    }

    //create entry
    struct _MessageT__Entry entry_msg = MESSAGE_T__ENTRY__INIT;
    entry_msg.key = entry->key;
    entry_msg.value.len = entry->value->datasize;  //FIXME: might me better to use memcpy
    entry_msg.value.data = entry->value->data;
    
    //create message to send
    struct _MessageT msg = MESSAGE_T__INIT;
    msg.opcode = 50;
    msg.c_type = 30;
    msg.entry = &entry_msg;

    //send message
    struct _MessageT *response = network_send_receive(rtree, &msg);

    if(response == NULL){
        //printf("Error sending message");
        return -1;
    }

    if(response->opcode == 51){
        //printf("Success, entry added");
    }

    if(response->opcode == 99){
        //printf("Error, entry not added");
        return -1;
    }

    return 0;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key){
    if(rtree == NULL || key == NULL){
        //printf("Error: rtree or key is NULL");
        return NULL;
    }

    //create message to send
    struct _MessageT msg = MESSAGE_T__INIT;
    msg.opcode = 40;
    msg.c_type = 10;
    msg.key = key;

    //send message
    struct _MessageT *response = network_send_receive(rtree, &msg);

    if(response == NULL){
        //printf("Error sending message");
        return NULL;
    }

    if(response->opcode == 41){
        //printf("Success, entry found");
        struct data_t *data = data_create2(response->value.len, response->value.data);
        return data;
    }

    if(response->opcode == 99){
        //printf("Error, entry not found");
        return NULL;
    }

    return NULL;
}

/* Função para remover um elemento da árvore. Vai libertar 
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key){
    if(rtree == NULL || key == NULL){
        //printf("Error: rtree or key is NULL");
        return -1;
    }

    //create message to send
    struct _MessageT msg = MESSAGE_T__INIT;
    msg.opcode = 30;
    msg.c_type = 10;
    msg.key = key;

    //send message
    struct _MessageT *response = network_send_receive(rtree, &msg);

    if(response == NULL){
        //printf("Error sending message");
        return -1;
    }

    if(response->opcode == 31){
        //printf("Success, entry deleted");
        return 0;
    }

    if(response->opcode == 99){
        //printf("Error, entry not deleted");
        return -1;
    }

    return -1;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree){
    if(rtree == NULL){
        //printf("Error: rtree is NULL");
        return -1;
    }

    //create message to send
    struct _MessageT msg = MESSAGE_T__INIT;
    msg.opcode = 10;
    msg.c_type = 70;

    //send message
    struct _MessageT *response = network_send_receive(rtree, &msg);

    if(response == NULL){
        //printf("Error sending message");
        return -1;
    }

    if(response->opcode == 11){
        //printf("Success, size found");
        return response->result;
    }

    if(response->opcode == 99){
        //printf("Error, size not found");
        return -1;
    }

    return -1;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree){
    if(rtree == NULL){
        //printf("Error: rtree is NULL");
        return -1;
    }

    //create message to send
    struct _MessageT msg = MESSAGE_T__INIT;
    msg.opcode = 20;
    msg.c_type = 70;

    //send message
    struct _MessageT *response = network_send_receive(rtree, &msg);

    if(response == NULL){
        //printf("Error sending message");
        return -1;
    }

    if(response->opcode == 21){
        //printf("Success, height found");
        return response->result;
    }

    if(response->opcode == 99){
        //printf("Error, height not found");
        return -1;
    }

    return -1;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree){
    if(rtree == NULL){
        //printf("Error: rtree is NULL");
        return NULL;
    }

    //create message to send
    struct _MessageT msg = MESSAGE_T__INIT;
    msg.opcode = 60;
    msg.c_type = 70;

    //send message
    struct _MessageT *response = network_send_receive(rtree, &msg);

    if(response == NULL){
        //printf("Error sending message");
        return NULL;
    }

    if(response->opcode == 61){
        //printf("Success, keys found");
        char **keys = malloc(sizeof(char*) * (response->n_keys + 1));
        for(int i = 0; i < response->n_keys; i++){
            keys[i] = strdup(response->keys[i]);
        }
        keys[response->n_keys] = NULL;
        return keys;
    }

    if(response->opcode == 99){
        //printf("Error, keys not found");
        return NULL;
    }

    return NULL;
}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void **rtree_get_values(struct rtree_t *rtree){
    if(rtree == NULL){
        //printf("Error: rtree is NULL");
        return NULL;
    }

    //create message to send
    struct _MessageT msg = MESSAGE_T__INIT;
    msg.opcode = 70;
    msg.c_type = 70;

    //send message
    struct _MessageT *response = network_send_receive(rtree, &msg);

    if(response == NULL){
        //printf("Error sending message");
        return NULL;
    }

    if(response->opcode == 71){
        //printf("Success, values found");
        void **values = malloc(sizeof(void*) * (response->n_values + 1));
        for(int i = 0; i < response->n_values; i++){
            values[i] = data_create2(response->values[i].len, response->values[i].data);
        }
        values[response->n_values] = NULL;
        return values;
    }

    if(response->opcode == 99){
        //printf("Error, values not found");
        return NULL;
    }

    return NULL;
}