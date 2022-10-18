/* -------------------------------------------------------------
* Grupo: 49
* Membros: Miguel Pato, fc57102
*          Tomás Correia, fc56372
*          João Figueiredo, fc53524
*
*/
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
    tree_destroy(tree->left);
    tree_destroy(tree->right);
    entry_destroy(tree->node);
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
        char *key2 = strdup(key);
        struct data_t *value2 = data_dup(value);
        tree->node = entry_create(key2, value2);  // Cria uma nova entrada
        if(tree->node == NULL) return -1;       // Falha na criação da entry
        return 0;                               // Sucesso
    }

    //compare using entry_compare()
    struct entry_t *entry = entry_create(key, value);
    int comp = entry_compare(tree->node, entry); 
    free(entry);

    if(comp == 0){
        data_destroy(tree->node->value);
        tree->node->value = data_dup(value);
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
    if(tree == NULL || key == NULL) return NULL;
    if(tree->node == NULL) return NULL;

    //compare using entry_compare()
    struct entry_t *entry = entry_create(key, NULL);
    int comp = entry_compare(tree->node, entry);
    free(entry); //only free the entry, not the the key since it is not duplicated

    if(comp == 0){
        return data_dup(tree->node->value);
    }
    if(comp == -1){
        if(tree->left == NULL) return NULL;
        return tree_get(tree->left, key);
    }
    
    if(comp == 1){
        if(tree->right == NULL) return NULL;
        return tree_get(tree->right, key);
    }

    return NULL;
}

/* Função para remover um elemento da árvore, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação tree_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int tree_del(struct tree_t *tree, char *key){
    if(tree == NULL || key == NULL) return -1;
    if(tree->node == NULL) return -1;

    //compare using entry_compare()
    struct entry_t *entry = entry_create(key, NULL);
    int comp = entry_compare(tree->node, entry);
    free(entry); //only free the entry, not the the key since it is not duplicated

    if(comp == 0){  // Se a entrada for a que se pretende remover
        if(tree->left == NULL && tree->right == NULL){
            entry_destroy(tree->node);
            tree->node = NULL;
            return 0;
        }

        if(tree->left == NULL){
            struct tree_t *temp = tree->right;
            entry_destroy(tree->node);
            tree->node = temp->node;
            return 0;
        }
        
        if(tree->right == NULL){
            struct tree_t *temp = tree->left;
            entry_destroy(tree->node);
            tree->node = temp->node;
            return 0;
        }

        if(tree->left != NULL && tree->right != NULL){
            struct tree_t *temp = tree->right;
            struct tree_t *temp2 = tree->right;      
            while(temp->left != NULL){            // Encontra o menor elemento da subárvore direita
                temp2 = temp;                       // Guarda o pai do menor elemento
                temp = temp->left;              
            }
            entry_destroy(tree->node);         // Liberta a entrada que vai ser substituida
            tree->node = temp->node;           // Substitui a entrada
            if(temp2 == temp){                 // Se o menor elemento for o primeiro da subárvore direita
                tree->right = temp->right;   // Substitui a subárvore direita
            }
            else{
                temp2->left = temp->right;   // Substitui o menor elemento pelo seu filho direito
            }
            free(temp);
            return 0;
        }
    }
    
    if(comp == -1){  
        if(tree->left == NULL) return -1;
        return tree_del(tree->left, key);
    }
    
    if(comp == 1){
        if(tree->right == NULL) return -1;
        return tree_del(tree->right, key);
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

/* Função que liberta toda a memória alocada pela árvore.
 */
int tree_get_keys_aux(struct tree_t *tree, char **keys, int i){
    if(tree == NULL) return i;                               //se a árvore for nula retorna o i
    if(tree->node == NULL) return i;                         //se o node for nulo retorna o i

    if(tree->left != NULL) i = tree_get_keys_aux(tree->left, keys, i);  //handle left
    
    keys[i] = strdup(tree->node->key);                     //add current root key to array
    i++;
    
    if(tree->right != NULL) i = tree_get_keys_aux(tree->right, keys, i); //handle right
    return i;
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária. As keys devem vir ordenadas segundo a ordenação alfabética das mesmas.
 */
char **tree_get_keys(struct tree_t *tree){
    if(tree == NULL) return NULL;
    if(tree->node == NULL) return NULL;
    int size = tree_size(tree);                             //usual stuff
    char **keys = malloc((size+1)*sizeof(char*));
    if(keys == NULL) return NULL;
    int i = 0;
    
    if(tree->left != NULL) i = tree_get_keys_aux(tree->left, keys, i);   //handle left side

    keys[i] = strdup(tree->node->key);                                   //handle current root
    i++;

    if(tree->right != NULL) i = tree_get_keys_aux(tree->right, keys, i); //handle right side

    keys[i] = NULL;                                                //add NULL
    return keys;
}

/* Função auxiliar para a função tree_get_values().
 */
int tree_get_values_aux(struct tree_t *tree, void **values, int i){
    if(tree == NULL) return i;                               //se a árvore for nula retorna o i
    if(tree->node == NULL) return i;                         //se o node for nulo retorna o i

    if(tree->left != NULL) i = tree_get_values_aux(tree->left, values, i);  //handle left
    
    values[i] = data_dup(tree->node->value);                     //add current root value to array
    i++;
    
    if(tree->right != NULL) i = tree_get_values_aux(tree->right, values, i); //handle right
    return i;
}

/* Função que devolve um array de void* com a cópia de todas os values da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
void **tree_get_values(struct tree_t *tree){
    if(tree == NULL) return NULL;
    if(tree->node == NULL) return NULL;
    int size = tree_size(tree);                             //usual stuff
    void **values = malloc((size+1)*sizeof(void*));
    if(values == NULL) return NULL;
    int i = 0;
    
    if(tree->left != NULL) i = tree_get_values_aux(tree->left, values, i);   //handle left side

    values[i] = data_dup(tree->node->value);                                   //handle current root
    i++;

    if(tree->right != NULL) i = tree_get_values_aux(tree->right, values, i); //handle right side

    values[i] = NULL;                                                //add NULL
    return values;
}

/* Função que liberta toda a memória alocada por tree_get_keys().
 */
void tree_free_keys(char **keys){
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
    int i = 0;
    while(values[i] != NULL){
        data_destroy(values[i]);
        i++;
    }
    free(values);
}