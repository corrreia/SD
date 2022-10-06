#include "../include/data.h"
#include "../include/entry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Serializa todas as keys presentes no array de strings keys
 * para o buffer keys_buf que será alocado dentro da função.
 * O array de keys a passar em argumento pode ser obtido através 
 * da função tree_get_keys. Para além disso, retorna o tamanho do
 * buffer alocado ou -1 em caso de erro.
 */
int keyArray_to_buffer(char **keys, char **keys_buf){
    // if(keys == NULL || keys_buf == NULL) return -1;

    // keys_buf = malloc(sizeof(keys));
    // if(keys_buf == NULL) return -1;
    
    // int i = 0;
    // while(keys[i] != NULL){
    //     strcat(*keys_buf, keys[i]);
    //     i++;
    // }
    // return sizeof(keys_buf);
    if(keys == NULL || keys_buf == NULL) return -1;
    int size = htonl(sizeof(keys));                     //tamanho do array de keys em bytes (network byte order)
    int i = sizeof(int) + size;
    *keys_buf = malloc(i);
    if(*keys_buf == NULL) return -1;
    memcpy(*keys_buf, &size, sizeof(int));
    memcpy(*keys_buf + sizeof(int), keys, size);
    return i;
}

/* De-serializa a mensagem contida em keys_buf, com tamanho
 * keys_buf_size, colocando-a e retornando-a num array char**,
 * cujo espaco em memória deve ser reservado. Devolve NULL
 * em caso de erro.
 */
char** buffer_to_keyArray(char *keys_buf, int keys_buf_size){
    if (keys_buf == NULL || keys_buf_size < 0) return NULL;

    int size = 0;                               //tamanho do array de strings
    void *ptr;                                  //ponteiro auxiliar
    memcpy(&size, keys_buf, sizeof(int));       //copia o tamanho do array de strings para a variavel size
    ptr = malloc(size);                         //aloca o espaço para o array de strings
    if(ptr == NULL) return NULL;                //verifica se a alocação foi bem sucedida
    memcpy(ptr, keys_buf + sizeof(int), size);  //copia o array de strings para o espaço alocado
    return ptr;                                 //retorna o array de strings
}
