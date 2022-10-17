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
	
	int size = 0;
	//print keys
	printf("key: %s\n",keys[0]);
	printf("key: %s\n",keys[1]);
	printf("key: %s\n",keys[2]);
	printf("key: %s\n",keys[3]);
	char **serialized;
	size = keyArray_to_buffer(keys, serialized);
	printf("size: %d\n", size);
	printf("serialized: %s\n", serialized[0]);
	printf("serialized: %s\n", serialized[1]);
	printf("serialized: %s\n", serialized[2]);
	printf("serialized: %s\n", serialized[3]);
	printf("serialized: %s\n", serialized[4]);
	printf("serialized: %s\n", serialized[5]);
	printf("serialized: %s\n", serialized[6]);
	printf("serialized: %s\n", serialized[7]);

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
