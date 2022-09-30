#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/data.h"

/* Função que cria um novo elemento de dados data_t, reservando a memória
 * necessária para armazenar os dados, especificada pelo parâmetro size 
 */
struct data_t *data_create(int size){
    if(size <= 0) return NULL;

    struct data_t *data = (struct data_t *) malloc(sizeof(struct data_t));

    data->datasize = size;
    data->data = malloc(size);
    
    return data;
}

/* Função que cria um novo elemento de dados data_t, inicializando o campo
 * data com o valor passado no parâmetro data, sem necessidade de reservar
 * memória para os dados.
 */
struct data_t *data_create2(int size, void *data){
    if(size <= 0 || data == NULL) return NULL;
    struct data_t *data2 = (struct data_t *) malloc(sizeof(struct data_t));

    data2->datasize = size;
    data2->data = data;

    return data2;
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 */
void data_destroy(struct data_t *data){
    if(data == NULL) return;
    free(data->data);
    free(data);
}

/* Função que duplica uma estrutura data_t, reservando toda a memória
 * necessária para a nova estrutura, inclusivamente dados.
 */
struct data_t *data_dup(struct data_t *data){
    if(data == NULL || data->datasize <= 0 || data->data == NULL) return NULL;

    struct data_t *data2 = (struct data_t *) malloc(sizeof(struct data_t));

    data2->datasize = data->datasize;
    data2->data = malloc(data->datasize);

    memcpy(data2->data, data->data, data->datasize);
    
    return data2;
}

/* Função que substitui o conteúdo de um elemento de dados data_t.
*  Deve assegurar que destroi o conteúdo antigo do mesmo.
*/
void data_replace(struct data_t *data, int new_size, void *new_data){
    if(data == NULL) return;
    free(data->data);
    data->datasize = new_size;
    data->data = new_data;
}


