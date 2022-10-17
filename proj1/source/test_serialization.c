#define NDEBUG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../include/data.h"
#include "../include/entry.h"
#include "../include/tree.h"
#include "../include/serialization.h"


int testSerialization() {
	int result = 1;
	struct tree_t *tree = tree_create();
	char **keys;
	char *k[4] = {"abc","bcd","cde","def"};
	struct data_t *d = data_create(5);

	tree_put(tree,k[3],d);
	tree_put(tree,k[2],d);
	tree_put(tree,k[1],d);
	tree_put(tree,k[0],d);

	data_destroy(d);

	keys = tree_get_keys(tree);
	
	//print keys
	for(int i=0; keys[i] != NULL; i++) {
		printf("key: %s\n", keys[i]);
	}
	printf("key: %s\n", keys[4]);

	char **serialized;

	int size = keyArray_to_buffer(keys, serialized);


	printf("size: %d\n", size);

	for(int i = 0; i < size; i++) {
		printf("serialized[%d]: %s\n", i, serialized[i]);
	}

	//free(serialized);
	tree_free_keys(keys);
	tree_destroy(tree);

	printf("tree - testGetKeys: %s\n",result?"passou":"não passou");
	return result;
}

int main() {
	int score = 0;

	printf("iniciando teste tree bin\n");

	score += testSerialization();


	printf("teste serialization: %d/7\n",score);

    if (score == 7)
        return 0;
    else
        return -1;
}
