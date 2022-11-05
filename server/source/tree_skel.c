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
            struct data_t *data = tree_get(tree, msg->key);
            if(msg->c_type == MESSAGE_T__C_TYPE__CT_KEY && (data != NULL)){
                msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                tree_del(tree, msg->key);
            }
            else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }
            if (data != NULL) data_destroy(data);
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
                if(tree_put(tree, msg->entry->key, data)){
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                }
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
        default:
            msg->opcode = MESSAGE_T__OPCODE__OP_BAD;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
    }
    return msg->result;
}
