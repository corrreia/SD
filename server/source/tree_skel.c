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

#include "../include/tree_skel.h"
#include "../include/tree-private.h"
#include "../include/data.h"
#include "../include/entry.h"
#include "../include/sdmessage.pb-c.h"

struct tree_t *tree = NULL;
struct request_t *queue_head;
int n_threads;
pthread_t *threads;
pthread_mutex_t queue_lock, tree_lock  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty =  PTHREAD_COND_INITIALIZER;

int last_assigned = 1; 
struct op_proc op_procs;


/* Inicia o skeleton da árvore.
* O main() do servidor deve chamar esta função antes de poder usar a
* função invoke().
* A função deve lançar N threads secundárias responsáveis por atender
* pedidos de escrita na árvore.
* Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
*/
int tree_skel_init(int N){
    n_threads = N;
    tree = tree_create();
    if(tree == NULL){
        return -1;
    }
    
    //create N secondary threads
    threads = (pthread_t *) malloc(N * sizeof(pthread_t));
    for(int i = 0; i < N; i++){
        if(pthread_create(&threads[i], NULL, &process_request, NULL) != 0) //create thread
            return -1;
        else 
            printf("Thread %d created\n", i);
    }

    //initialize op_procs
    op_procs.max_proc = 0;  //max op_n ever processed
    op_procs.in_progress = calloc(N, sizeof(int)); //array of N ints
    for(int i = 0; i < N; i++){
        op_procs.in_progress[i] = 0;
    }

    return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy(){
    if(tree != NULL) tree_destroy(tree);
    if(op_procs.in_progress != NULL) free(op_procs.in_progress);
    free(queue_head);
    //exit threads
    for(int i = 0; i < n_threads; i++){
        pthread_cancel(threads[i]);
    }
    free(threads);
}

/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n){
    // -1 error, 0 still in line, 1 processed, 2 non existent
    if(op_n > op_procs.max_proc) return 2; //op_n never processed   
    if(op_procs.in_progress[op_n] == 0) return 0; //still in line
    if(op_procs.in_progress[op_n] == 1) return 1; //processed
    return -1; //error
}

/* Função da thread secundária que vai processar pedidos de escrita.
*/
void * process_request (void *params){
    while(1){
        pthread_mutex_lock(&queue_lock);
        while(queue_head == NULL){
            pthread_cond_wait(&queue_not_empty, &queue_lock);
        }
        struct request_t *request = queue_head;
        queue_head = queue_head->next;
        int i = 0;
        while(op_procs.in_progress[i] != 0){
            i++;
        }
        op_procs.in_progress[i] = request->op_n;

        pthread_mutex_unlock(&queue_lock);
        
        //handle request
        if(request->op == 0){ //delete
            pthread_mutex_lock(&tree_lock);
            tree_del(tree, request->key);
            op_procs.max_proc = request->op_n;  //last op_n processed
            pthread_mutex_unlock(&tree_lock);
        }
        else if(request->op == 1){ //put
            pthread_mutex_lock(&tree_lock);
            tree_put(tree, request->key, request->data);
            op_procs.max_proc = request->op_n; //update max_proc to op_n
            pthread_mutex_unlock(&tree_lock);
        }
        
        op_procs.in_progress[i] = 0; //free op_n
    }
    return NULL;
}

struct request_t *get_last_request(struct request_t *head){
    struct request_t *current = head;
    while(current->next != NULL){
        current = current->next;
    }
    return current;
}


int tree_skel_put(char* key, struct data_t *value){
    // add request to queue and send signal to thread
    struct request_t *last_request = get_last_request(queue_head);
    struct request_t *request = (struct request_t *) malloc(sizeof(struct request_t));
    request->op = 1;
    request->key = key;
    request->data = value;
    request->next = NULL;
    request->op_n = last_request->op_n + 1;

    last_request->next = request;    

    //send signal to thread
    pthread_cond_broadcast(&queue_not_empty);

    return request->op_n;
}

int tree_skel_del(char* key){
    // add request to queue and send signal to thread
    struct request_t *last_request = get_last_request(queue_head);
    struct request_t *request = (struct request_t *) malloc(sizeof(struct request_t));
    request->op = 0;
    request->key = key;
    request->data = NULL;
    request->next = NULL;
    request->op_n = last_request->op_n + 1;

    last_request->next = request;    

    //send signal to thread
    pthread_cond_broadcast(&queue_not_empty);

    return request->op_n;
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
                
                data_destroy(data);
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


