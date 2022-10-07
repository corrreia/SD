/* -------------------------------------------------------------
* Grupo: 49
* Membros: Miguel Pato, fc57102
*          Tomás Correia, fc56372
*          João Figueiredo, fc53524
*
*/
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
int keyArray_to_buffer(char **keys, char **keys_buf){  //nao sei se é assim que se faz
    //add all keys to buffer separated by theyr size
    int size = 0;
    int i = 0;

    int size_keys = sizeof(keys)/sizeof(keys[0]);
    
    while(keys[i] != NULL){
        size += strlen(keys[i]) + 1;
        i++;
    }

    // for (int k = 0; k < i; k++) {
    //     keys_buf[k] = (char *)malloc(strlen(keys[k]) + 1);
    //     char* test = (char*)malloc(strlen(keys[k]) + 1);
    //     test[0] = (char)strlen(keys[k]);
    //     memcpy(test + 1, keys[k], strlen(keys[k]));
    //     memcpy(keys_buf[k], test, sizeof(test));
    // }

    for (i = 0; i < size_keys*2; ++i) {
        if(i%2 == 0) keys_buf[i] = (char *)malloc(1 + 1);
        if(i%2 == 1) keys_buf[i] = (char *)malloc(strlen(keys[i/2]) + 1);
    }

    for (i = 0; i < size_keys*2; ++i) {
        if(i%2 == 0) {
            keys_buf[i/2] = (char *) strlen(keys[i/2]);
        }
        else { 
            memcpy(keys_buf[i], keys[i/2 + 1], strlen(keys[i/2 + 1]));
        }
    }

    return size;
}

/* De-serializa a mensagem contida em keys_buf, com tamanho
 * keys_buf_size, colocando-a e retornando-a num array char**,
 * cujo espaco em memória deve ser reservado. Devolve NULL
 * em caso de erro.
 */
char** buffer_to_keyArray(char *keys_buf, int keys_buf_size){
    //get all keys from buffer
    int i = 0;
    int j = 0;
    int size = 0;
    int size_keys = 0;
    char** keys = NULL;

    while(i < keys_buf_size){
        size = (int)keys_buf[i];
        size_keys++;
        i += size + 1;
    }

    keys = (char**)malloc(size_keys*sizeof(char*));  //nao conseguimos testar porque a fuxxao do keyArray_to_buffer nao funciona

    i = 0;
    while(i < keys_buf_size){
        size = (int)keys_buf[i];
        keys[j] = (char*)malloc(size*sizeof(char));
        memcpy(keys[j], keys_buf + i + 1, size);
        i += size + 1;
        j++;
    }
    return keys;
}
