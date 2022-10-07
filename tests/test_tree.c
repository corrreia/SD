#define NDEBUG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../include/data.h"
#include "../include/entry.h"
#include "../include/tree.h"

/**************************************************************/
int testTreeVazia() {
	struct tree_t *tree = tree_create();

	int result = tree != NULL && tree_size(tree) == 0;

	tree_destroy(tree);
	
	printf("tree - testTreeVazia: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/

int testPutInexistente() {
	int result, i;
	struct tree_t *tree = tree_create();
	char *key[1024];
	struct data_t *data[1024], *d;

	for(i=0; i<1024; i++) {
		key[i] = (char*)malloc(16*sizeof(char));
		sprintf(key[i],"a/key/b-%d",i);
		data[i] = data_create2(strlen(key[i])+1,strdup(key[i]));

		tree_put(tree,key[i],data[i]);
	}

	assert(tree_size(tree) == 1024);
	result = (tree_size(tree) == 1024);

	for(i=0; i<1024; i++) {
		d = tree_get(tree,key[i]);

		assert(d->datasize == data[i]->datasize);
		assert(memcmp(d->data,data[i]->data,d->datasize) == 0);
		assert(d->data != data[i]->data);

		result = result && (d->datasize == data[i]->datasize && 
                           memcmp(d->data,data[i]->data,d->datasize) == 0 &&
                           d->data != data[i]->data);
		data_destroy(d);
	}

	for(i=0; i<1024; i++) {
		free(key[i]);
		data_destroy(data[i]);
	}

	tree_destroy(tree);
	
	printf("tree - testPutInexistente: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/

int testPutExistente() {
	int result, i;
	struct tree_t *tree = tree_create();
	char *key[1024];
	struct data_t *data[1024], *d;

	for(i=0; i<1024; i++) {
		key[i] = (char*)malloc(16*sizeof(char));
		sprintf(key[i],"a/key/b-%d",i);
		data[i] = data_create2(strlen(key[i])+1,strdup(key[i]));

		tree_put(tree,key[i],data[i]);
	}


	assert(tree_size(tree) == 1024);
	result = (tree_size(tree) == 1024);

	d = data_create2(strlen("256")+1,strdup("256"));
	tree_put(tree,key[256],d);
	data_destroy(d);

	assert(tree_size(tree) == 1024);
	result = result && (tree_size(tree) == 1024);

	for(i=0; i<1024; i++) {
		d = tree_get(tree,key[i]);
		
		if(i==256) {
			result = result && (d->datasize == strlen("256")+1 && 
        	                   memcmp(d->data,"256",d->datasize) == 0);
		} else {
			result = result && (d->datasize == data[i]->datasize && 
        	                   memcmp(d->data,data[i]->data,d->datasize) == 0 &&
        	                   d->data != data[i]->data);
		}

		data_destroy(d);
	}

	for(i=0; i<1024; i++) {
		free(key[i]);
		data_destroy(data[i]);
	}

	tree_destroy(tree);
	
	printf("tree - testPutExistente: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/

int testDelInexistente() {
	int result, i;
	struct tree_t *tree = tree_create();
	char *key;
	struct data_t *data;

	for(i=0; i<1024; i++) {
		key = (char*)malloc(16*sizeof(char));
		sprintf(key,"a/key/b-%d",i);
		data = data_create2(strlen(key)+1,key);

		tree_put(tree,key,data);

		data_destroy(data);
	}

	assert(tree_size(tree) == 1024);
	result = (tree_size(tree) == 1024);

	result = result && (tree_get(tree,"a/key/b-1024") == NULL) &&
			   (tree_get(tree,"abc") == NULL);

	result = result && (tree_del(tree,"a/key/b-1024") != 0) &&
			   (tree_del(tree,"abc") != 0);

	result = result && (tree_get(tree,"a/key/b-1024") == NULL) &&
			   (tree_get(tree,"abc") == NULL);

	assert(tree_size(tree) == 1024);
	result = result && (tree_size(tree) == 1024);

	tree_destroy(tree);
	
	printf("tree - testDelInexistente: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/

int testDelExistente() {
	int result, i;
	struct tree_t *tree = tree_create();
	char *key;
	struct data_t *data, *data2 = NULL;

	for(i=0; i<1024; i++) {
		key = (char*)malloc(16*sizeof(char));
		sprintf(key,"a/key/b-%d",i);
		data = data_create2(strlen(key)+1,key);

		tree_put(tree,key,data);

		data_destroy(data);
	}

	assert(tree_size(tree) == 1024);
	result = (tree_size(tree) == 1024);

	result = result && ((data = tree_get(tree,"a/key/b-1023")) != NULL) &&
			   ((data2 = tree_get(tree,"a/key/b-45")) != NULL);

	data_destroy(data);
	data_destroy(data2);

	result = result && (tree_del(tree,"a/key/b-1023") == 0) &&
			   (tree_del(tree,"a/key/b-45") == 0);

	result = result && (tree_get(tree,"a/key/b-1023") == NULL) &&
			   (tree_get(tree,"a/key/b-45") == NULL);

	assert(tree_size(tree) == 1022);
	result = result && (tree_size(tree) == 1022);

	tree_destroy(tree);
	
	printf("tree - testDelExistente: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/
//este teste não verifica se as keys são retornadas por ordem lexicografica
//no entanto, pode ser facilmente modificado para testar isso
int testGetKeys() {
	int result = 1,i,j,achou;
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
	
	for(i=0; keys[i] != NULL; i++) {
		achou = 0;
		for(j=0; j<4; j++) {
			achou = (achou || (strcmp(keys[i],k[j]) == 0));
		}
		result = (result && achou);
	}

	result = result && (tree_size(tree) == i);

	tree_free_keys(keys);

	tree_destroy(tree);

	printf("tree - testGetKeys: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/

int testGetValues() {
	int result = 1,i,j,achou;
	struct tree_t *tree = tree_create();
	char **keys;
	char *k[4] = {"abc","bcd","cde","def"};
	char *v[4] = {"abc","bcd","cde","def"};
	struct data_t *d[4];

	for (i = 0; i < 4; i++) {
		d[i] = data_create2(4, strdup(v[i]));
	}

	tree_put(tree,k[3],d[3]);
	tree_put(tree,k[2],d[2]);
	tree_put(tree,k[1],d[1]);
	tree_put(tree,k[0],d[0]);

	keys = tree_get_keys(tree);
	void **values = tree_get_values(tree);

	for (i = 0; i < 4; i++) {
		data_destroy(d[i]);
	}

	for(i=0; keys[i] != NULL; i++) {
		achou = 0;
		for(j=0; j<4; j++) {
			achou = (achou || (strcmp(keys[i],k[j]) == 0));
		}
		result = (result && achou);
	}

	for (i = 0; values[i] != NULL; i++) {
		for (j = 0; j < 4; j++) {
			achou = (achou || (strcmp(values[i], v[j]) == 0));
		}
	}

	result = result && (tree_size(tree) == i);

	tree_free_keys(keys);
	tree_free_values(values);

	tree_destroy(tree);

	printf("tree - testGetKeys: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/

int main() {
	int score = 0;

	printf("iniciando teste tree bin\n");

	score += testTreeVazia();

	score += testPutInexistente();

	score += testPutExistente();

	score += testDelInexistente();

	score += testDelExistente();

	score += testGetKeys();
	
	score += testGetValues();

	printf("teste tree bin: %d/7\n",score);

    if (score == 7)
        return 0;
    else
        return -1;
}
