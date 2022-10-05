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
    int i, size = 0;
    for(i = 0; keys[i] != NULL; i++){
        size += strlen(keys[i]) + 1;
    }

    *keys_buf = malloc(size);
    if(*keys_buf == NULL){
        return -1;
    }

    int offset = 0;
    for(i = 0; keys[i] != NULL; i++){
        strcpy(*keys_buf + offset, keys[i]);
        offset += strlen(keys[i]) + 1;
    }

    return size;
}

/* De-serializa a mensagem contida em keys_buf, com tamanho
 * keys_buf_size, colocando-a e retornando-a num array char**,
 * cujo espaco em memória deve ser reservado. Devolve NULL
 * em caso de erro.
 */
char** buffer_to_keyArray(char *keys_buf, int keys_buf_size){
    int i, offset = 0;
    char **keys = malloc(sizeof(char*) * (keys_buf_size + 1));
    if(keys == NULL){
        return NULL;
    }
    for(i = 0; offset < keys_buf_size; i++){
        keys[i] = keys_buf + offset;
        offset += strlen(keys[i]) + 1;
    }
    keys[i] = NULL;
    return keys;
}
