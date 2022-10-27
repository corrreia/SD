#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/tree_skel.h"
#include "../include/tree-private.h"
#include "../include/data.h"
#include "../include/entry.h"
#include "../include/sdmessage.pb-c.h"

struct tree_t *tree = NULL;

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). 
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init(){
    tree = tree_create();

    if(tree == NULL){
        return -1;
    }

    return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy(){
    tree_destroy(tree);
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(struct _MessageT *msg){
    if(tree == NULL){
        return -1;
    }

    MessageT__Opcode opcode = msg->opcode;

    switch(opcode){  
        case MESSAGE_T__OPCODE__OP_SIZE: //OP_SIZE  WORKS
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
        case MESSAGE_T__OPCODE__OP_HEIGHT: //OP_HEIGHT WORKS
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
        case MESSAGE_T__OPCODE__OP_DEL: //OP_DEL WORKS
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_KEY && tree_get(tree, msg->key) != NULL){
                msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                tree_del(tree, msg->key);
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        case MESSAGE_T__OPCODE__OP_GET: //OP_GET 
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_KEY){
                struct data_t *data = tree_get(tree, msg->key);
                printf("key: %s\n", msg->key);
                printf("data: %s\n", data->data);
                printf("datasize: %d\n", data->datasize);
                if(data != NULL){
                    msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
                    msg->value->data = data->data;
                    msg->value->datasize = data->datasize;
                }
                else{
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    return -1;
                }
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        case MESSAGE_T__OPCODE__OP_PUT: //OP_PUT
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_ENTRY){    
                msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        
                if(tree_put(tree, msg->entry->key, data_create2(msg->entry->value->datasize, msg->entry->value->data))){
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                }
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        case MESSAGE_T__OPCODE__OP_GETKEYS: //OP_GETKEYS
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_NONE && tree_size(tree) > 0){
                msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
                msg->keys = tree_get_keys(tree);
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        case MESSAGE_T__C_TYPE__CT_NONE: //OP_GETVALUES
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_NONE && tree_size(tree) > 0){
                msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_VALUES;
                //using memcpy to copy the array of data_t
                memcpy(msg->values, tree_get_values(tree), tree_size(tree) * sizeof(struct data_t));
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            break;
        default:
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
    }
    return msg->result;
}
