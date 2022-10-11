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

/* Converts a int to a string
 */
char *int_to_string(int n){
    char *str = (char *) malloc(13);
    sprintf(str, "%d", n);
    return str;
}

/* Serializa todas as keys presentes no array de strings keys
 * para o buffer keys_buf que será alocado dentro da função.
 * O array de keys a passar em argumento pode ser obtido através 
 * da função tree_get_keys. Para além disso, retorna o tamanho do
 * buffer alocado ou -1 em caso de erro.
 */
int keyArray_to_buffer(char **keys, char **keys_buf){

    //keys_buf = sould be a array of strings folowing the format: 
    //  [size of the key][key][size of the key][key]...
    // so the number of elements in the buffer is the number of keys * 2

    //count the number of keys
    int size_buffer = 0; //number of elements in the buffer
    while(keys[size_buffer] != NULL){
        size_buffer++;
    }
    size_buffer *= 2;

    //allocate the buffer with a for loop
    int h = 0;
    for(int i = 0; i < size_buffer; i++){
        if (i % 2 == 0){
            //allocate the size of the key
            keys_buf[i] = int_to_string(strlen(keys[h]));
        } else {
            //allocate the key
            keys_buf[i] = (char *) malloc(sizeof(char) * strlen(keys[h]));
            keys_buf[i] = strdup(keys[h]);
            h++;
        }
    }

    return size_buffer;
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
