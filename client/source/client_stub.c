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

//Zookeeper
#include <zookeeper/zookeeper.h>

#include "../include/data.h"
#include "../include/entry.h"
#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/network_client.h"
#include "../include/sdmessage.pb-c.h"

#define ZPATHLEN 1024 
struct rtree_t *head_server;
struct rtree_t *tail_server;
zhandle_t *zh;

char *zoo_root = "/chain";
char *zoo_path = "/chain/node";
typedef struct String_vector zoo_string;
zoo_string *children_list;
static char *watcher_ctx = "ZooKeeper Data Watcher";
int is_connected = 0;

int stub_connect(struct rtree_t *rtree, char *info){

    //devide info
    char *host = strtok(info, ":");
    int port = atoi(strtok(NULL, ":"));

    rtree->server.sin_family = AF_INET;
    rtree->server.sin_port = htons(port);

    if(inet_aton(host, &rtree->server.sin_addr) == 0){
        printf("Error converting address\n");
        return -1;
    }

    if(network_connect(rtree) == -1){
        printf("Error connecting to server\n");
        return -1;
    }

    return 0;
}

void watcher(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx){
    if (type == ZOO_SESSION_EVENT){
        if (state == ZOO_CONNECTED_STATE) 
            is_connected = 1;
        else 
            is_connected = 0;
    }
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx){
    //filhos tiveram update entao precisamos ver novos head/tail e ligar a esses
    children_list = (zoo_string *)malloc(sizeof(zoo_string));
    int data_len = 100;
    char headPath[100];
    char tailPath[100];
    char *headInfo = malloc(data_len);
    char *tailInfo = malloc(data_len);

    if (state != ZOO_CONNECTED_STATE && type != ZOO_CHILD_EVENT) {
        return;
    }

    if (ZOK != zoo_wget_children(zh, zoo_root, child_watcher, watcher_ctx, children_list)){
        printf("Error setting watch at %s!\n", zoo_root);
    }

    head_server->identifier = children_list->data[0];
    tail_server->identifier = children_list->data[0];

    for(int i = 0; i < children_list->count; i++){
        if(strcmp(children_list->data[i], head_server->identifier) < 0){ //sempre que houver menor, substitui (head)
            head_server->identifier = children_list->data[i];
        }
        if(strcmp(children_list->data[i], tail_server->identifier) > 0){ //sempre que houver maior, substitui (tail)
            tail_server->identifier = children_list->data[i];
        }
    }

    //HEAD AND TAIL ARE SET
    strcpy(headPath, zoo_root);
    strcat(headPath, "/");
    strcat(headPath, head_server->identifier);

    strcpy(tailPath, zoo_root);
    strcat(tailPath, "/");
    strcat(tailPath, tail_server->identifier);

    //get head info
    if (ZOK != zoo_wget(zh, headPath, child_watcher, watcher_ctx, headInfo, &data_len, NULL)){
        printf("Error setting watch at %s!\n", headPath);
    }

    //get tail info
    if (ZOK != zoo_wget(zh, tailPath, child_watcher, watcher_ctx, tailInfo, &data_len, NULL)){
        printf("Error setting watch at %s!\n", tailPath);
    }

    //disconnect from head and tail
    network_close(head_server);
    network_close(tail_server);

    //connect to head and tail
    stub_connect(head_server, headInfo);
    stub_connect(tail_server, tailInfo);

    free(headInfo);
    free(tailInfo);

    return;
}


int rtree_connect(const char *address_port){  

    if(address_port == NULL){
        return -1;
    }

    head_server = (struct rtree_t *) malloc(sizeof(struct rtree_t));
    tail_server = (struct rtree_t *) malloc(sizeof(struct rtree_t));

    if(head_server == NULL || tail_server == NULL){
        printf("Error allocating memory for rtree");
        return -1;
    }    

    //Zookeeper
    zh = zookeeper_init(address_port, watcher, 10000, 0, 0, 0);
    if(zh == NULL){
        printf("Error connecting to Zookeeper\n");
        return -1;
    }

    sleep(3); //wait for connection

    children_list = (zoo_string *)malloc(sizeof(zoo_string));

    int data_len = 100;
    char headPath[100];
    char tailPath[100];
    char *headInfo = malloc(data_len);
    char *tailInfo = malloc(data_len);

    if(is_connected == 0){
        printf("Error connecting to Zookeeper\n"); //se não estiver conectado, retorna erro
        return -1;
    }

    //check if root exists
    if(zoo_exists(zh, zoo_root, 0, NULL) == ZNONODE){
        printf("Root node doesn't exist\n");
        return -1;
    }

    //get children
    if (ZOK != zoo_wget_children(zh, zoo_root, child_watcher, watcher_ctx, children_list)){
        printf("Error setting watch at %s!\n", zoo_root);
    }

    head_server->identifier = children_list->data[0];
    tail_server->identifier = children_list->data[0];

    for(int i = 0; i < children_list->count; i++){
        if(strcmp(children_list->data[i], head_server->identifier) < 0){ //sempre que houver menor, substitui (head)
            head_server->identifier = children_list->data[i];
        }
        if(strcmp(children_list->data[i], tail_server->identifier) > 0){ //sempre que houver maior, substitui (tail)
            tail_server->identifier = children_list->data[i];
        }
    }

    //HEAD AND TAIL ARE SET
    strcpy(headPath, zoo_root);
    strcat(headPath, "/");
    strcat(headPath, head_server->identifier);

    strcpy(tailPath, zoo_root);
    strcat(tailPath, "/");
    strcat(tailPath, tail_server->identifier);

    //get head info
    if (ZOK != zoo_wget(zh, headPath, child_watcher, watcher_ctx, headInfo, &data_len, NULL)){
        printf("Error setting watch at %s!\n", headPath);
    }

    //get tail info
    if (ZOK != zoo_wget(zh, tailPath, child_watcher, watcher_ctx, tailInfo, &data_len, NULL)){
        printf("Error setting watch at %s!\n", tailPath);
    }

    //connect to head and tail
    stub_connect(head_server, headInfo);
    stub_connect(tail_server, tailInfo);
     
    return 0;
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
int rtree_put(struct entry_t *entry){
    if(head_server == NULL || entry == NULL){
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
    msg = network_send_receive(head_server, msg);

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
struct data_t *rtree_get(char *key){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    message_t__init(msg);

    msg->key = key;
    msg->opcode = MESSAGE_T__OPCODE__OP_GET;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;

    //send message
    msg = network_send_receive(tail_server, msg);
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
int rtree_del(char *key){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    message_t__init(msg);

    msg->key = key;
    msg->opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;

    //send message
    msg = network_send_receive(head_server, msg);

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
int rtree_size(){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    if(msg == NULL) return -1;
    message_t__init(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    //send message
    msg = network_send_receive(tail_server, msg);
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
int rtree_height(){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    if(msg == NULL) return -1;

    message_t__init(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    //send message
    msg = network_send_receive(tail_server, msg);

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
char **rtree_get_keys(){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    if(msg == NULL) return NULL;

    message_t__init(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    //send message
    msg = network_send_receive(tail_server, msg);

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
void **rtree_get_values(){

    MessageT *msg = (MessageT *) malloc(sizeof(MessageT));
    if(msg == NULL) return NULL;

    message_t__init(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    //send message 
    msg = network_send_receive(tail_server, msg);

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
int rtree_verify(int op_n){
    MessageT *msg = malloc(message_t__descriptor.sizeof_message);
    if(msg == NULL) return -1;

    message_t__init(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->op_n = op_n;

    //send message
    msg = network_send_receive(tail_server, msg);

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