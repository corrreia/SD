#ifndef _TREE_SKEL_H
#define _TREE_SKEL_H

#include "sdmessage.pb-c.h"
#include "tree.h"
#include <zookeeper/zookeeper.h>

struct request_t {
int op_n; //o número da operação
int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
char* key; //a chave a remover ou adicionar
struct data_t *data; // os dados a adicionar em caso de put, ou NULL em caso de delete
//adicionar campo(s) necessário(s) 
struct request_t *next; //apontador para o próximo pedido  (FIFO) (starts as NULL)
};

struct op_proc {
    int max_proc;
    int in_progress;
};


/* Inicia o skeleton da árvore.
* O main() do servidor deve chamar esta função antes de poder usar a
* função invoke().
* A função deve lançar N threads secundárias responsáveis por atender
* pedidos de escrita na árvore.
* Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
*/
int tree_skel_init();

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy();

/* Função para obter a ultima operação da lista
    */
struct request_t *get_last_request(struct request_t *head);

/* Executa a operação put.
*/
int tree_skel_put(char* key, struct data_t *value);

/* Executa a operação del.
*/
int tree_skel_del(char* key);

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(struct _MessageT *msg);

/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n);

/* Função da thread secundária que vai processar pedidos de escrita.
*/
void * process_request (void *params);

int start_zookeeper(char *zookeeper_addr, char *server_port);

#endif
