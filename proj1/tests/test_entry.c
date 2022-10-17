#include "data.h"
#include "entry.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**************************************************************/
int testCreate() {
	char *key = strdup("123abc");
	struct data_t *value = data_create2(strlen("1234567890abc")+1, strdup("1234567890abc"));

	struct entry_t *entry = entry_create(key, value);

	int result = (entry->key == key) && 
                     (entry->value == value);

	entry_destroy(entry);

	printf("entry - testCreate: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/

int testDup() {
	int result;
	char *key = strdup("123abc");
	struct data_t *value = data_create2(strlen("1234567890abc")+1, strdup("1234567890abc"));

	struct entry_t *entry = entry_create(key, value);

	struct entry_t *entry2 = entry_dup(entry);

	result = entry2 != entry;
	
	result = result && (entry->key != entry2->key) &&
		              (strcmp(entry->key,entry2->key) == 0) && 
                        (entry->value != entry2->value) &&
		              (entry->value->datasize == entry2->value->datasize) &&
                        (memcmp(entry->value->data, entry2->value->data, entry->value->datasize) == 0);

	entry_destroy(entry);
	entry_destroy(entry2);

	printf("entry - testDup: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/

int testDestroy(){
    printf("Módulo entry -> teste entry_destroy:");
	entry_destroy(NULL);
    printf(" passou\n");
	return 1;
}

/**************************************************************/

int testReplace(){
	int result;
	char *key = strdup("123abc");
	char *new_key = strdup("123abc456");
	struct data_t *value = data_create2(strlen("1234567890abc")+1, strdup("1234567890abc"));
	struct data_t *new_value = data_create2(strlen("123456abc")+1, strdup("123456abc"));

	struct entry_t *entry = entry_create(key, value);

	entry_replace(entry, new_key, new_value);

	result = (entry->key == new_key) && (strcmp(entry->key, new_key) == 0);

	result = result && (entry->value == new_value);

	entry_destroy(entry);

	printf("entry - testReplace: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/

int testCompare(){
	int result;
	char *key1 = strdup("a");
	char *key2 = strdup("b");
	char *key3 = strdup("f");
	char *key4 = strdup("a");
	struct data_t *value1 = data_create2(strlen("1234567890abc")+1, strdup("1234567890abc"));
	struct data_t *value2 = data_create2(strlen("1234567890abc")+1, strdup("1234567890abc"));
	struct data_t *value3 = data_create2(strlen("sabc")+1, strdup("sabc"));
	struct data_t *value4 = data_create2(strlen("abc")+1, strdup("abc"));

	struct entry_t *entry1 = entry_create(key1, value1);
	struct entry_t *entry2 = entry_create(key2, value2);
	struct entry_t *entry3 = entry_create(key3, value3);
	struct entry_t *entry4 = entry_create(key4, value4);

	result = (entry_compare(entry1, entry2) == -1);
	result = result && (entry_compare(entry4, entry2) == -1);
	result = result && (entry_compare(entry2, entry1) == 1);
	result = result && (entry_compare(entry3, entry1) == 1);
	result = result && (entry_compare(entry3, entry2) == 1);
	result = result && (entry_compare(entry2, entry3) == -1);
	result = result && (entry_compare(entry1, entry4) == 0);
	result = result && (entry_compare(entry4, entry1) == 0);

	entry_destroy(entry1);
	entry_destroy(entry2);
	entry_destroy(entry3);
	entry_destroy(entry4);

	printf("entry - testCompare: %s\n",result?"passou":"não passou");
	return result;
}


/**************************************************************/

int main() {
	int score = 0;

	printf("iniciando teste entry bin\n");

	score += testCreate();

	score += testDup();
	
	score += testDestroy();
	
	score += testReplace();
	
	score += testCompare();

	printf("teste entry bin: %d/5\n",score);

    if (score == 5)
        return 0;
    else
        return -1;
}
