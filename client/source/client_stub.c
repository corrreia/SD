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
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "../include/data.h"
#include "../include/entry.h"
#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/network_client.h"
#include "../include/sdmessage.pb-c.h"


/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port){  

    if(address_port == NULL){
        return NULL;
    }

    struct rtree_t *rtree = (struct rtree_t *) malloc(sizeof(struct rtree_t));

    if(rtree == NULL){
        printf("Error allocating memory for rtree");
        return NULL;
    }    

    char *ap_copy = strdup(address_port);
    if(ap_copy == NULL){
        return NULL;
    }

    char *address = strtok(ap_copy, ":");  
    char *port = strtok(NULL, "\0");

    rtree->server.sin_family = AF_INET;
    rtree->server.sin_port = htons(atoi(port));


    //check if address is an IP or a hostname
    if(inet_pton(AF_INET, address, &rtree->server.sin_addr) <= 0){
        //address is a hostname
        struct hostent *host = gethostbyname(address);   
        //print converted IP
        printf("Provided address is a hostname, converted IP is: %s\n", inet_ntoa(*((struct in_addr *)host->h_addr)));
        if(host == NULL){
            printf("Error getting host by name\n");  //! problem here: when connecting to a host that doesent have a server running, it crashes
            return NULL;
        }

        if(inet_pton(AF_INET, inet_ntoa(*((struct in_addr *)host->h_addr)), &rtree->server.sin_addr) <= 0){
            printf("Error converting IP\n");
            return NULL;
        }

        memcpy(&rtree->server.sin_addr, host->h_addr, host->h_length);
    }

    if(network_connect(rtree) < 0){
        rtree_disconnect(rtree);
        free(ap_copy);
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
        return -1;
    }
    return network_close(rtree);
}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t *rtree, struct entry_t *entry){
    if(rtree == NULL || entry == NULL){
        printf("Error: rtree or entry is NULL");
        return -1;
    }

    //create message
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    message_t__init(msg);
    msg->entry = malloc(message_t__entry__descriptor.sizeof_message);
    message_t__entry__init(msg->entry);
    msg->entry->value = malloc(message_t__data__descriptor.sizeof_message);
    message_t__data__init(msg->entry->value);


    msg->entry->key = strdup(entry->key);
    msg->entry->value->datasize = entry->value->datasize;
    msg->entry->value->data = strdup(entry->value->data);
    msg->opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    //send message
    msg = network_send_receive(rtree, msg);

    if(msg->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        message_t__free_unpacked(msg, NULL);
        return -1;
    }

    int opcode = msg->op_n;

    message_t__free_unpacked(msg, NULL);    

    return opcode;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    message_t__init(msg);

    msg->key = key;
    msg->opcode = MESSAGE_T__OPCODE__OP_GET;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;

    //send message
    msg = network_send_receive(rtree, msg);
    if(msg == NULL){
        return NULL;
    }

    if(msg->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }

    if(msg->opcode == MESSAGE_T__OPCODE__OP_GET+1 &&
        msg->c_type == MESSAGE_T__C_TYPE__CT_NONE){
            message_t__free_unpacked(msg, NULL);
            printf("Key not found\n");
            return data_create(0); //! THIS RETURNS NULL !! TODO: fix this
    }
    
    //create data_t
    struct data_t *data = data_create(msg->value->datasize);
    memcpy(data->data, msg->value->data, msg->value->datasize);

    message_t__free_unpacked(msg, NULL);

    return data;
}

/* Função para remover um elemento da árvore. Vai libertar 
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    message_t__init(msg);

    msg->key = key;
    msg->opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;

    //send message
    msg = network_send_receive(rtree, msg);

    if(msg == NULL){
        return -1;
    }

    if(msg->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        message_t__free_unpacked(msg, NULL);
        return -1;
    }

    int opcode = msg->op_n;

    message_t__free_unpacked(msg, NULL);

    return opcode;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    if(msg == NULL) return -1;
    message_t__init(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    //send message
    msg = network_send_receive(rtree, msg);
    if(msg == NULL) return -1;

    if(msg->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        message_t__free_unpacked(msg, NULL);
        return -1;
    }

    int size = msg->result;
    message_t__free_unpacked(msg, NULL);

    return size;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    if(msg == NULL) return -1;

    message_t__init(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    //send message
    msg = network_send_receive(rtree, msg);

    if(msg == NULL){
        return -1;
    }

    if(msg->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        message_t__free_unpacked(msg, NULL);
        return -1;
    }

    int height = msg->result;
    message_t__free_unpacked(msg, NULL);

    return height;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    if(msg == NULL) return NULL;

    message_t__init(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    //send message
    msg = network_send_receive(rtree, msg);

    if(msg == NULL){
        return NULL;
    }

    if(msg->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }

    char **keys = (char **) calloc(msg->n_keys+1, sizeof(char *));

    for(int i = 0; i < msg->n_keys; i++){
        keys[i] = strdup(msg->keys[i]);
    }
    keys[msg->n_keys] = NULL;

    message_t__free_unpacked(msg, NULL);

    return keys;
}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void **rtree_get_values(struct rtree_t *rtree){
    if(rtree == NULL){
        //printf("Error: rtree is NULL");
        return NULL;
    }

    MessageT *msg = (MessageT *) malloc(sizeof(MessageT));
    if(msg == NULL) return NULL;

    message_t__init(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    //send message 
    msg = network_send_receive(rtree, msg);

    if(msg == NULL){
        return NULL;
    }

    if(msg->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        message_t__free_unpacked(msg, NULL);
        return NULL;
    }

    void **values = (void **) calloc(msg->n_values+1, sizeof(void *));

    for(int i = 0; i < msg->n_values; i++){
        values[i] = strdup(msg->values[i]);
    }

    values[msg->n_values] = NULL;

    message_t__free_unpacked(msg, NULL);

    return values;
}

/* Verifica se a operação identificada por op_n foi executada.
*/
int rtree_verify(struct rtree_t *rtree, int op_n){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    if(msg == NULL) return -1;

    message_t__init(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->op_n = op_n;

    //send message
    msg = network_send_receive(rtree, msg);

    if(msg == NULL){
        return -1;
    }

    if(msg->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        message_t__free_unpacked(msg, NULL);
        return -1;
    }

    int result = msg->result; 
    message_t__free_unpacked(msg, NULL);

    return result;
}