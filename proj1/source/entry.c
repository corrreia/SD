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

#include "../include/entry.h"

/* Função que cria uma entry, reservando a memória necessária para a
 * estrutura e inicializando os campos key e value, respetivamente, com a
 * string e o bloco de dados passados como parâmetros, sem reservar
 * memória para estes campos.
 */
struct entry_t *entry_create(char *key, struct data_t *data){
    struct entry_t *entry = (struct entry_t *) malloc(sizeof(struct entry_t));

    if (entry == NULL) return NULL;

    entry->key = key;  
    entry->value = data;
    return entry;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry){
    if (entry == NULL) return;

    free(entry->key);
    if (entry->value != NULL) data_destroy(entry->value);
    free(entry);
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry){
    if(entry == NULL) return NULL;

    struct entry_t *entry2 = (struct entry_t *) malloc(sizeof(struct entry_t));

    entry2->key = strdup(entry->key);
    entry2->value = data_dup(entry->value);

    return entry2;
}

/* Função que substitui o conteúdo de uma entrada entry_t.
*  Deve assegurar que destroi o conteúdo antigo da mesma.
*/
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value){
    if(entry == NULL) return;

    free(entry->key);
    if (entry->value != NULL) data_destroy(entry->value);

    entry->key = new_key;
    entry->value = new_value;
}

/* Função que compara duas entradas e retorna a ordem das mesmas.
*  Ordem das entradas é definida pela ordem das suas chaves.
*  A função devolve 0 se forem iguais, -1 se entry1<entry2, e 1 caso contrário.
*/
int entry_compare(struct entry_t *entry1, struct entry_t *entry2){
    int cmp = strcmp(entry1->key, entry2->key);
    if(cmp == 0) return 0;
    else if(cmp < 0) return -1;
    else return 1;
}