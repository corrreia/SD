#include "../include/data.h"
#include "../include/entry.h"
#include "../include/tree.h"
#include "../include/tree-private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//struct tree_t; /* A definir pelo grupo em tree-private.h */

/* Função para criar uma nova árvore tree vazia.
 * Em caso de erro retorna NULL.
 */
struct tree_t *tree_create(){
    struct tree_t *tree = (struct tree_t*) malloc(sizeof(struct tree_t));
    if(tree == NULL) return NULL;
    tree->node = NULL;
    tree->left = NULL;
    tree->right = NULL;
    return tree;
}

/* Função para libertar toda a memória ocupada por uma árvore.
 */
void tree_destroy(struct tree_t *tree){
    if(tree == NULL) return;
    if(tree->node != NULL) entry_destroy(tree->node);
    if(tree->left != NULL) tree_destroy(tree->left);
    if(tree->right != NULL) tree_destroy(tree->right);
    free(tree);
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int tree_put(struct tree_t *tree, char *key, struct data_t *value){
    if(tree == NULL || key == NULL || value == NULL) return -1;
    if(tree->node == NULL){                     // Se a árvore estiver vazia
        tree->node = entry_create(key, value);  // Cria uma nova entrada
        if(tree->node == NULL) return -1;       // Falha na criação da entry
        return 0;                               // Sucesso
    }

    //compare using entry_compare()
    int comp = entry_compare(tree->node, entry_create(key, value));

    if(comp == 0){
        entry_replace(tree->node, key, value);
        return 0;
    }
    if(comp == -1){
        if(tree->left == NULL){
            tree->left = tree_create();
            if(tree->left == NULL) return -1;
        }
        return tree_put(tree->left, key, value);
    }
    
    if(comp == 1){
        if(tree->right == NULL){
            tree->right = tree_create();
            if(tree->right == NULL) return -1;
        }
        return tree_put(tree->right, key, value);
    }

    return -1;
    
}

/* Função para obter da árvore o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou tree_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da árvore,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função. Devolve NULL em caso de erro.
 */
struct data_t *tree_get(struct tree_t *tree, char *key){
    if(tree == NULL || key == NULL) return NULL;    // Verifica se a árvore ou a key são nulas
    if(tree->node == NULL) return NULL;             // Verifica se o nó é nulo
    int cmp = strcmp(key, tree->node->key);         // Compara a key com a key do nó

    if(cmp == 0){                                    // Se forem iguais
        return data_dup(tree->node->value);         // Devolve uma cópia dos dados
    }

    if(cmp < 0){                                    // Se a key for menor que a key do nó
        if(tree->left == NULL) return NULL;         // Se o filho esquerdo for nulo
        return tree_get(tree->left, key);           // Chama a função para o filho esquerdo
    }

    if(cmp > 0){                                    // Se a key for maior que a key do nó
        if(tree->right == NULL) return NULL;        // Se o filho direito for nulo
        return tree_get(tree->right, key);          // Chama a função para o filho direito
    }
    return NULL;                                    // Erro
}

/* Função para remover um elemento da árvore, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação tree_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int tree_del(struct tree_t *tree, char *key){
    if(tree == NULL || key == NULL) return -1;
    if(tree->node == NULL) return -1;
    struct entry_t *entry = tree->node;
    while(entry != NULL){
        if(strcmp(entry->key, key) == 0){
            entry_destroy(entry);
            return 0;
        }
        if(strcmp(entry->key, key) > 0){
            entry = tree->left->node;
        }
        else{
            entry = tree->right->node;
        }
    }
    return -1;
}

/* Função que devolve o número de elementos contidos na árvore.
 */
int tree_size(struct tree_t *tree){
    if(tree == NULL) return -1;
    if(tree->node == NULL) return 0;
    int size = 1;
    if(tree->left != NULL) size += tree_size(tree->left);
    if(tree->right != NULL) size += tree_size(tree->right);
    return size;
}

/* Função que devolve a altura da árvore.
 */
int tree_height(struct tree_t *tree){
    if(tree == NULL) return -1;
    if(tree->node == NULL) return 0;
    int height = 1;
    if(tree->left != NULL) height += tree_height(tree->left);
    if(tree->right != NULL) height += tree_height(tree->right);
    return height;
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária. As keys devem vir ordenadas segundo a ordenação lexicográfica das mesmas.
 */
char **tree_get_keys(struct tree_t *tree){
    if(tree == NULL) return NULL;
    if(tree->node == NULL) return NULL;
    int size = tree_size(tree);
    char **keys = malloc((size + 1) * sizeof(char*));
    if(keys == NULL) return NULL;
    int i = 0;
    if(tree->left != NULL){
        char **left_keys = tree_get_keys(tree->left);
        if(left_keys == NULL){
            free(keys);
            return NULL;
        }
        while(left_keys[i] != NULL){
            keys[i] = left_keys[i];
            i++;
        }
        free(left_keys);
    }
    keys[i] = strdup(tree->node->key);
    if(keys[i] == NULL){
        free(keys);
        return NULL;
    }
    i++;
    if(tree->right != NULL){
        char **right_keys = tree_get_keys(tree->right);
        if(right_keys == NULL){
            free(keys);
            return NULL;
        }
        while(right_keys[i] != NULL){
            keys[i] = right_keys[i];
            i++;
        }
        free(right_keys);
    }
    keys[i] = NULL;
    return keys;
}

/* Função que devolve um array de void* com a cópia de todas os values da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
void **tree_get_values(struct tree_t *tree){
    if(tree == NULL) return NULL;
    if(tree->node == NULL) return NULL;
    int size = tree_size(tree);
    void **values = malloc((size + 1) * sizeof(void*));
    if(values == NULL) return NULL;
    int i = 0;
    if(tree->left != NULL){
        void **left_values = tree_get_values(tree->left);
        if(left_values == NULL){
            free(values);
            return NULL;
        }
        while(left_values[i] != NULL){
            values[i] = left_values[i];
            i++;
        }
        free(left_values);
    }
    values[i] = data_dup(tree->node->value);
    if(values[i] == NULL){
        free(values);
        return NULL;
    }
    i++;
    if(tree->right != NULL){
        void **right_values = tree_get_values(tree->right);
        if(right_values == NULL){
            free(values);
            return NULL;
        }
        while(right_values[i] != NULL){
            values[i] = right_values[i];
            i++;
        }
        free(right_values);
    }
    values[i] = NULL;
    return values;
}


/* Função que liberta toda a memória alocada por tree_get_keys().
 */
void tree_free_keys(char **keys){
    if(keys == NULL) return;
    int i = 0;
    while(keys[i] != NULL){
        free(keys[i]);
        i++;
    }
    free(keys);
}

/* Função que liberta toda a memória alocada por tree_get_values().
 */
void tree_free_values(void **values){
    if(values == NULL) return;
    int i = 0;
    while(values[i] != NULL){
        data_destroy(values[i]);
        i++;
    }
    free(values);
}

