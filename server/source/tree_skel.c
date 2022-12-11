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
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

// ZooKeeper client library and headers
#include <zookeeper/zookeeper.h>

#include "../include/tree_skel.h"
#include "../include/tree-private.h"
#include "../include/data.h"
#include "../include/entry.h"
#include "../include/sdmessage.pb-c.h"
#include "../include/zk-private.h"
#include "../include/network_server.h"

// Global variables
#define ZPATHLEN 1024 

struct tree_t *tree = NULL;
struct request_t *queue_head = NULL;
pthread_t thread;
pthread_mutex_t queue_lock, tree_lock  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty =  PTHREAD_COND_INITIALIZER;

int last_assigned = 1; 
struct op_proc op_procs;

int thread_term = 1;

// ZooKeeper client instance
zhandle_t *zh = NULL;
struct zk_info *zk_info;
static char *watcher_ctx = "ZooKeeper Data Watcher";

/* Inicia o skeleton da árvore.
* O main() do servidor deve chamar esta função antes de poder usar a
* função invoke().
* A função deve lançar N threads secundárias responsáveis por atender
* pedidos de escrita na árvore.
* Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
*/
int tree_skel_init(){

    zk_info = (struct zk_info *) malloc(sizeof(struct zk_info));

    tree = tree_create();
    if(tree == NULL){
        zookeeper_close(zh);
        return -1;
    }

    // Create a thread to process requests
    if(pthread_create(&thread, NULL, process_request, NULL) != 0){
        zookeeper_close(zh);
        tree_destroy(tree);
        return -1;
    }

    // Initialize op_procs
    op_procs.max_proc = 0;  //max op_n ever processed
    op_procs.in_progress = 0; //op_n currently being processed

    return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy(){
    if(tree != NULL) tree_destroy(tree);
    free(queue_head);

    // Terminate thread
    thread_term = 0;
    pthread_cond_signal(&queue_not_empty);
    pthread_join(thread, NULL);

    // Free queue
    struct request_t *aux = queue_head;
    while(aux != NULL){
        queue_head = queue_head->next;
        free(aux);
        aux = queue_head;
    }

    // Close ZooKeeper client
    zookeeper_close(zh);
}

/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n){
    // Return 1 if done, 0 if not
    if(op_n > op_procs.max_proc) return 0; //op_n is not in the array
    return 1;
}

/* Função da thread secundária que vai processar pedidos de escrita.
*/
void * process_request (void *params){
    printf("Thread started\n");
    while(thread_term){
        pthread_mutex_lock(&queue_lock);
        while(queue_head == NULL){
            pthread_cond_wait(&queue_not_empty, &queue_lock);
        }
        struct request_t *request = queue_head;
        op_procs.in_progress = queue_head->op_n;
        queue_head = queue_head->next;
        pthread_mutex_unlock(&queue_lock);

        
        // Handle request
        pthread_mutex_lock(&tree_lock);
        
        if(request->op == 0){
            tree_del(tree, request->key);
            
            MessageT *msg = malloc(message_t__descriptor.sizeof_message);
            message_t__init(msg);

            msg->key = request->key;
            msg->opcode = MESSAGE_T__OPCODE__OP_DEL;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;

            network_send(zk_info->next_socket, msg);
        }            

        else if(request->op == 1){
            tree_put(tree, request->key, request->data);

            //create message
            MessageT *msg = malloc(message_t__descriptor.sizeof_message);
            message_t__init(msg);
            msg->entry = malloc(message_t__entry__descriptor.sizeof_message);
            message_t__entry__init(msg->entry);
            msg->entry->value = malloc(message_t__data__descriptor.sizeof_message);
            message_t__data__init(msg->entry->value);


            msg->entry->key = request->key;
            msg->entry->value->datasize = request->data->datasize;
            msg->entry->value->data = request->data->data;
            msg->opcode = MESSAGE_T__OPCODE__OP_PUT;
            msg->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

            network_send(zk_info->next_socket, msg);  //Send message to next server
        }

        op_procs.max_proc = request->op_n;
        pthread_mutex_unlock(&tree_lock);

        // Free request
        free(request);
    }

    return NULL;
}

int add_to_queue(struct request_t *request){
    pthread_mutex_lock(&queue_lock);
    if(queue_head == NULL){
        queue_head = request;
    }
    else{
        struct request_t *aux = queue_head;
        while(aux->next != NULL){
            aux = aux->next;
        }
        aux->next = request;
    }
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_lock);
    return 0;
}

int tree_skel_put(char* key, struct data_t *value){
    printf("tree_skel_put\n");
    struct request_t *request = (struct request_t *) malloc(sizeof(struct request_t));
    request->next = NULL;
    request->op_n = last_assigned;
    request->op = 1;
    request->key = key;
    request->data = value;

    add_to_queue(request);

    last_assigned++;
    return last_assigned - 1;
}

int tree_skel_del(char* key){
    printf("tree_skel_del\n");
    struct request_t *request = (struct request_t *) malloc(sizeof(struct request_t));
    request->next = NULL;
    request->op_n = last_assigned;
    request->op = 0;
    request->key = key;
    request->data = NULL;

    add_to_queue(request); //add request to queue

    last_assigned++; //increment last_assigned
    return last_assigned - 1;
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(MessageT *msg){
    if(tree == NULL || msg == NULL){
        return -1;
    }

    MessageT__Opcode opcode = msg->opcode;

    switch(opcode){
        case MESSAGE_T__OPCODE__OP_SIZE:
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_NONE){
                msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
                msg->result = tree_size(tree);
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        case MESSAGE_T__OPCODE__OP_HEIGHT:
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_NONE){
                msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
                msg->result = tree_height(tree);
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        case MESSAGE_T__OPCODE__OP_DEL: {
            msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->op_n = tree_skel_del(msg->key);
            
            break;
        }
        case MESSAGE_T__OPCODE__OP_GET:
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_KEY){
                struct data_t *data = tree_get(tree, msg->key);
                if(data != NULL){
                    msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;

                    msg->value = malloc(message_t__data__descriptor.sizeof_message);
                    message_t__data__init(msg->value);

                    msg->value->data = strdup(data->data);
                    msg->value->datasize = data->datasize;
                    data_destroy(data);
                }
                else{
                    msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    //msg->value->datasize = 0;
                }
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        case MESSAGE_T__OPCODE__OP_PUT:
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_ENTRY){
                msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

                struct data_t *data = data_create(msg->entry->value->datasize);
                memcpy(data->data, msg->entry->value->data, msg->entry->value->datasize);

                msg->op_n = tree_skel_put(msg->entry->key, data);
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        case MESSAGE_T__OPCODE__OP_GETKEYS:
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_NONE && tree_size(tree) > 0){
                msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
                msg->n_keys = tree_size(tree);
                msg->keys = tree_get_keys(tree);
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        case MESSAGE_T__OPCODE__OP_GETVALUES: //OP_GETVALUES
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_NONE && tree_size(tree) > 0){
                msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_VALUES;
                msg->n_values = tree_size(tree);
                //msg->values = (char**) tree_get_values(tree); //COULD NOT GET THIS TO WORK LIKE THIS

                char **keys = tree_get_keys(tree);                                   //so we did it like this
                char **values = (char **) calloc(tree_size(tree), sizeof(char *));   //and it works
                //msg->values is a array of strings
                int i = 0;
                while(keys[i] != NULL){
                    struct data_t *data = tree_get(tree, keys[i]);
                    values[i] = (char *) malloc(data->datasize);
                    memcpy(values[i], data->data, data->datasize);
                    data_destroy(data);
                    i++;
                }
                msg->values = values;

                for(i = 0; i < tree_size(tree); i++){
                    free(keys[i]);
                }
                free(keys);

            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        case MESSAGE_T__OPCODE__OP_VERIFY:                      // CHECK THIS LATER
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_NONE){
                msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                msg->result = verify(msg->op_n);
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        default:
            msg->opcode = MESSAGE_T__OPCODE__OP_BAD;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
    }
    return msg->result;
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	if (state != ZOO_CONNECTED_STATE && type != ZOO_CHILD_EVENT) {
        return;
    }   

    zk_info->children = (zoo_string *) malloc(sizeof(zoo_string));
		
    /* Get the updated children and reset the watch */ 
    if (ZOK != zoo_wget_children(zk_info->zh, zk_info->zoo_root, child_watcher, NULL, zk_info->children)) {
        printf("Error watching children of %s\n", zk_info->zoo_root);
        return;
    }

    char* nextPath = malloc(ZPATHLEN);
    strcpy(nextPath, "NULL");  //nextPath is the path of the next node in the chain (NULL if there is no next node)

    printf("Connecting to next node in chain");

    for (int i = 0; i < zk_info->children->count; i++)  { //find the next node in the chain
        if(strcmp(nextPath, "NULL") == 0 && strcmp(zk_info->children->data[i], zk_info->identifier) > 0)   //compares this identifier to data[i] and if data[i] is greater than this identifier, then exists a next node
            strcpy(nextPath, zk_info->children->data[i]); // then exists a next node, so we set nextPath to the next node
        
        if(strcmp(zk_info->children->data[i], zk_info->identifier)>0 && strcmp(zk_info->children->data[i], nextPath)<0) //if the next node is greater than this identifier and less than the nextPath, then we set nextPath to the next node
            strcpy(nextPath, zk_info->children->data[i]);
    }

    sleep(3); //wait 3 seconds before connecting to the next node (needed to avoid connection problems)

    if(strcmp(nextPath, "NULL") != 0){ //if there is a next node, connect to it

        zk_info->next_identifier= strdup(nextPath);
        
        int data_size = 100;
        char *data = malloc(data_size); //data is the data of the next node in the chain, will be the ip and port of the next node in the chain

        char dataPath[100];  //dataPath is the path of the next node in the chain
        strcpy(dataPath, zk_info->zoo_root);
        strcat(dataPath, "/");
        strcat(dataPath, nextPath);

        zoo_get(zk_info->zh, dataPath, 0, data, &data_size, NULL); //get the data of the next node in the chain
        
        //CONNECT TO THE NEXT NODE IN THE CHAIN
        
        //get host and port from data
        char *host = strtok(data, ":");
        int port = atoi(strtok(NULL, ":"));

        zk_info->next_server.sin_family = AF_INET;
        zk_info->next_server.sin_port = htons(port);

        if (inet_pton(AF_INET, host, &zk_info->next_server.sin_addr) < 1) {
            printf("Error converting IP address\n");
            exit(1);
        }
        //Criar socket TCP
        if((zk_info->next_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("Error creating TCP socket for the next node in the chain");
            exit(1);
        }
        
        //Estabelece conexao com o servidor
        if(connect(zk_info->next_socket,(struct sockaddr *)&zk_info->next_server, sizeof(zk_info->next_server)) < 0){
            perror("Error connecting to the next node in the chain");
            close(zk_info->next_socket);
            exit(1);
        }

        printf("Connected to next node in chain\n");
        free(data);
    }
    free(nextPath);
 
}

void watcher(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx){
    if (type == ZOO_SESSION_EVENT){
        if (state == ZOO_CONNECTED_STATE) 
            zk_info->is_connected = 1;
        else 
            zk_info->is_connected = 0;
    } 
}  

int start_zookeeper(char *zookeeper_addr, char *server_port){

    zk_info->zoo_root = "/chain";
    zk_info->zoo_path = (char *) malloc(strlen(zk_info->zoo_root) + 6);
    sprintf(zk_info->zoo_path, "%s/node", zk_info->zoo_root);

    zk_info->zh = zookeeper_init(zookeeper_addr, watcher, 10000, 0, 0, 0);
    if(zk_info->zh == NULL){
        printf("Error connecting to zookeeper\n");
        return -1;
    }

    //timeout to wait for connection to zookeeper
    sleep(3);

    if(zk_info->is_connected){ //if connected to zookeeper
        
        if(ZNONODE == zoo_exists(zk_info->zh, zk_info->zoo_root, 0, NULL)){
            printf("The node %s does not exist. Creating it now...\n", zk_info->zoo_root);

            //create the root node
            if(ZOK == zoo_create(zk_info->zh, zk_info->zoo_root, NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)){
                printf("Root node created\n");
            }
            else{
                printf("Error creating root node\n");
                free(zk_info);
                return -1;
            }
        }
    }

    //convert to string and set the zk_info->identifier
    // zk_info->identifier = (char *) malloc(4);
    // sprintf(zk_info->identifier, "%ld", zk_info->children->count);
    // printf("Server identifier: %s\n", zk_info->identifier);

    //handle ip and stuff
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;  //* needs to be a global variable

    char *portcpy = malloc(sizeof(int)+1);
    gethostname(hostbuffer,sizeof(hostbuffer));
    host_entry = gethostbyname(hostbuffer);
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    strcat(IPbuffer,":");
    sprintf(portcpy, "%s", server_port);
    strcat(IPbuffer,portcpy);

    char* nodePath = malloc(ZPATHLEN);

    if(ZOK != zoo_create(zk_info->zh, zk_info->zoo_path, IPbuffer, 42, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, nodePath, ZPATHLEN)){ //42 (THE ANSWER TO LIFE, THE UNIVERSE AND EVERYTHING)
        printf("Error creating node\n");
        free(zk_info);
        return -1;
    }

    printf("Node created: %s\n", nodePath);
    
    strtok(nodePath, "/");
    zk_info->identifier = strdup(strtok(NULL, ""));

    printf("Server identifier: %s\n", zk_info->identifier);

    if(ZOK != zoo_wget_children(zk_info->zh, zk_info->zoo_root, child_watcher, watcher_ctx, zk_info->children)){
        printf("Error getting children\n");
        free(zk_info);
        return -1;
    }

    return 0;
}

