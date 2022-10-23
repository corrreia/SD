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

    switch(msg->opcode){  //TODO: CHECK FOR ERRORS, VERY IMPORTANT IN A SERVER ENVIRONMENT
        case 10: //OP_SIZE
            if(msg->c_type == 70){
                msg->opcode = 11;
                msg->c_type = 60;
                msg->result = tree_size(tree);
            }
            else{
                msg->opcode = 99;
                msg->c_type = 70;
            }
            break;
        case 20: //OP_HEIGHT
            if(msg->c_type == 70){
                msg->opcode = 21;
                msg->c_type = 60;
                msg->result = tree_height(tree);
            }
            else{
                msg->opcode = 99;
                msg->c_type = 70;
            }
            break;
        case 30: //OP_DEL
            if(msg->c_type == 10){
                msg->opcode = 31;
                msg->c_type = 70;
                msg->result = tree_del(tree, msg->key);
            }
            else{
                msg->opcode = 99;
                msg->c_type = 70;
            }
            break;
        case 40: //OP_GET
            if(msg->c_type == 10){
                struct data_t *data = tree_get(tree, msg->key);
                if(data == NULL){
                    msg->opcode = 99;
                    msg->c_type = 70;
                }
                else{
                    msg->opcode = 41;
                    msg->c_type = 20;
                    msg->value.len = data->datasize;
                    msg->value.data = data->data;
                }
            }
            else{
                msg->opcode = 99;
                msg->c_type = 70;
            }
            break;
        case 50: //OP_PUT
            if(msg->c_type == 30){
                //type is an entry
                struct entry_t *entry = entry_create(msg->entry->key, data_create2(msg->entry->value.len, msg->entry->value.data));
                msg->opcode = 51;
                msg->c_type = 70;
                tree_put(tree, entry->key, entry->value);
            }
            else{
                msg->opcode = 99;
                msg->c_type = 70;
            }
            break;
        case 60: //OP_GETKEYS
            if(msg->c_type == 70){
                msg->opcode = 61;
                msg->c_type = 40;
                msg->keys = tree_get_keys(tree);
            }
            else{
                msg->opcode = 99;
                msg->c_type = 70;
            }
            break;
        case 70: //OP_GETVALUES
            if(msg->c_type == 70){
                msg->opcode = 71;
                msg->c_type = 50;
                msg->values = tree_get_values(tree);
            }
            else{
                msg->opcode = 99;
                msg->c_type = 70;
            }
            break;
        default:
            msg->opcode = 99;
            msg->c_type = 70;
            break;
    }
    return 0;
}
