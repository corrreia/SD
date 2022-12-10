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

// ZooKeeper client library and headers
#include <zookeeper/zookeeper.h>

#include "../include/tree_skel.h"
#include "../include/tree-private.h"
#include "../include/data.h"
#include "../include/entry.h"
#include "../include/sdmessage.pb-c.h"

// Global variables
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
const char *zoo_root = "/kvstore"; // path do node normal


/* Inicia o skeleton da árvore.
* O main() do servidor deve chamar esta função antes de poder usar a
* função invoke().
* A função deve lançar N threads secundárias responsáveis por atender
* pedidos de escrita na árvore.
* Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
*/
int tree_skel_init(){
    // Initialize ZooKeeper client
    zh = zookeeper_init("localhost:2181", NULL, 10000, 0, 0, 0);
    if (zh == NULL) {
        fprintf(stderr, "Error initializing ZooKeeper client\n");
        return -1;
    }

    // Create root node for the tree
    int rc = zoo_create(zh, "/kvstore", NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
    if (rc != ZOK) {
        fprintf(stderr, "Error creating root node for the tree\n");
        zookeeper_close(zh);
        return -1;
    }

    // Create a new tree instance
    tree = tree_create();
    if(tree == NULL){
        zookeeper_close(zh);
        return -1;
    }    

    // Initialize tree using the root node created in ZooKeeper
    struct node_t *root = node_create("/kvstore");
    if(root == NULL){
        zookeeper_close(zh);
        tree_destroy(tree);
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
        if(request->op == 0) { // Delete
            // Use ZooKeeper's delete() method to delete
            int rc = zoo_delete(zh, request->key, -1);
            if (rc != ZOK) {
                fprintf(stderr, "Error deleting node %s from the tree\n", request->key);
            }
        } else if(request->op == 1) { // Put
            // Use ZooKeeper's set() method to add a node to the tree
            int rc = zoo_set(zh, request->key, request->data->data, request->data->datasize, -1);
            if (rc != ZOK) {
                fprintf(stderr, "Error adding node %s to the tree\n", request->key);
            }
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
        pthread_cond_signal(&queue_not_empty);
    }
    else{
        struct request_t *aux = queue_head;
        while(aux->next != NULL){
            aux = aux->next;
        }
        aux->next = request;
    }
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


