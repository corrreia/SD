#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "data.h"

void pee(const char *msg)
{
    perror(msg);
    exit(0);
}

/**************************************************************/
int testCreate() {
	int result;
	struct data_t *data;

	printf("Módulo data -> teste data_create:");

	assert(data_create(-1) == NULL);
	result = (data_create(-1) == NULL);

	assert(data_create(0) == NULL);
	result = result && (data_create(0) == NULL);

	if ((data = data_create(1024)) == NULL)
		pee("  O teste não pode prosseguir");

	memcpy(data->data,"1234567890a",strlen("1234567890a")+1);

	result = result &&
		 ((strcmp(data->data,"1234567890a") == 0) && (data->datasize == 1024));

	data_destroy(data);

	printf("data - testCreate: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/
int testCreate2() {
	int result, data_size;
	struct data_t *data;
	//char *data_s = "1234567890abc";
	char *data_s = strdup("1234567890abc");
	data_size = strlen(data_s)+1;


	printf("Módulo data -> teste data_create2:");

	assert(data_create2(-1, data_s) == NULL);
	result = (data_create2(-1, data_s) == NULL);

    	assert(data_create2(0, data_s) == NULL);
	result = result && (data_create2(0, data_s) == NULL);

	assert(data_create2(data_size, NULL) == NULL);
	result = result && (data_create2(data_size, NULL) == NULL);

	if ((data = data_create2(data_size, data_s)) == NULL)
		pee("  O teste não pode prosseguir");

	result = result && (data->data == data_s);

	result = result &&
		 ((strcmp(data->data, data_s) == 0) && (data->datasize == data_size));

	data_destroy(data);

	printf("data - testCreate2: %s\n",result?"passou":"não passou");
	return result;
}


/**************************************************************/

int testCreate3() {
	int result, data_size;
	struct data_t *data;
	char *data_s = "1234567890abc";
	data_size = strlen(data_s)+1;
	char *data_p = malloc (data_size);
	strcpy (data_p, data_s);

	printf("Módulo data -> teste data_create2:");

	assert(data_create2(-1, data_p) == NULL);
	result = (data_create2(-1, data_p) == NULL);

    	assert(data_create2(0, data_p) == NULL);
	result = result && (data_create2(0, data_p) == NULL);

	assert(data_create2(data_size, NULL) == NULL);
	result = result && (data_create2(data_size, NULL) == NULL);

	if ((data = data_create2(data_size, data_p)) == NULL)
		pee("  O teste não pode prosseguir");

	result = result && (data->data == data_p);

	result = result &&
		 ((strcmp(data->data, data_p) == 0) && (data->datasize == data_size));

	data_destroy(data);

	printf("data - testCreate3: %s\n",result?"passou":"não passou");
	return result;
}

/**************************************************************/

int testDup() {
	//char *data_s = "1234567890abc";
	char *data_s = strdup("1234567890abc");
	int result, data_size = strlen(data_s)+1;
	struct data_t *data;
	struct data_t *data2;

	printf("Módulo data -> teste data_dup: ");

	assert(data_dup(NULL) == NULL);
	result = (data_dup(NULL) == NULL);

	if ((data = data_create(1)) == NULL)
		pee("  O teste não pode prosseguir");

	data->datasize = -1;

	assert(data_dup(data) == NULL);
	result = result && (data_dup(data) == NULL);

	free(data->data);
	
	data->data = NULL;
	assert(data_dup(data) == NULL);
	result = result && (data_dup(data) == NULL);
	
	data->datasize = 1;
	assert(data_dup(data) == NULL);
	result = result && (data_dup(data) == NULL);

	free(data);

	if ((data = data_create2(data_size,data_s)) == NULL)
		pee("  O teste não pode prosseguir");
	
	data2 = data_dup(data);
	
	result = result && (data2 != data);

	result = result && (data->data != data2->data) && 
                 (data->datasize == data2->datasize) &&
                 (memcmp(data->data, data2->data, data->datasize) == 0);

	data_destroy(data);
	data_destroy(data2);

	printf("data - testDup: %s\n",result?"passou":"não passou");
	return result;
}


/**************************************************************/
int testDestroy(){
    printf("Módulo data -> teste data_destroy:");
	data_destroy(NULL);
    printf(" passou\n");
	return 1;
}


/**************************************************************/
int testReplace(){
	int result, data_size, data_size2;
	struct data_t *data;
	//char *data_s = "1234567890abc";
	char *data_s = strdup("123");
	char *data_s2 = strdup("123456");
	data_size = strlen(data_s)+1;
	data_size2 = strlen(data_s2)+1;


	printf("Módulo data -> teste testReplace:");

	data = data_create2(data_size, data_s);
	
	data_replace(data, data_size2, data_s2);

	result = (data->data == data_s2);

	result = result &&
		 ((strcmp(data->data, data_s2) == 0) && (data->datasize == data_size2));

	data_destroy(data);

	printf("data - testCreate2: %s\n",result?"passou":"não passou");
	return result;
}


/**************************************************************/
int main() {
	int score = 0;

	printf("\nIniciando o teste do módulo data \n");

	score += testCreate();

	score += testCreate2();
	
	score += testCreate3();

	score += testDup();
	
	score += testDestroy();
	
	score += testReplace();

	printf("teste data bin: %d/6\n",score);

    if (score == 6)
        return 0;
    else
        return -1;
}
