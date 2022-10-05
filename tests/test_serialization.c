#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "data.h"
#include "entry.h"
#include "serialization.h"

void pee(const char *msg)
{
    perror(msg);
    exit(0);
}

/**************************************************************/
int testSerializeData(){
	int result, data_size;
	struct data_t *data;
    char *data_s = strdup("1234567890abc");
    char *data_buf;
    data_size = strlen(data_s)+1;

	printf("Módulo serialize -> teste serialize_data:");
    
    if ((data = data_create2(data_size, data_s)) == NULL)
        pee("data_create failed - O teste não pode prosseguir");
    
    assert(data_to_buffer(NULL, &data_buf) == -1);
    result = (data_to_buffer(NULL, &data_buf) == -1);
    
    assert(data_to_buffer(data, NULL) == -1);
    result = result && (data_to_buffer(data, NULL) == -1);
    
    int buf_size = data_to_buffer(data, &data_buf);
    assert(buf_size == sizeof(int) + data_size);
    result = result && (buf_size == sizeof(int) + data_size);
    
    data_destroy(data);
    free(data_buf);
    
    printf("serialize - testSerializeData: %s\n",result?"passou":"não passou");
    return result;
}

int testDeserializeData(){
    int result, data_size;
    struct data_t *data, *data2;
    char *data_s = strdup("1234567890abc");
    char *data_buf;
    data_size = strlen(data_s)+1;

    printf("Módulo serialize -> teste deserialize_data:");
    
    if ((data = data_create2(data_size, data_s)) == NULL)
        pee("data_create failed - O teste não pode prosseguir");
    
    int buf_size = data_to_buffer(data, &data_buf);
    if (buf_size <= 0)
        pee("data_to_buffer failed - O teste não pode prosseguir");
    
    assert(buffer_to_data(NULL,-1) == NULL);
    result = (buffer_to_data(NULL,-1) == NULL);
    
    assert(buffer_to_data(data_buf,-1) == NULL);
    result = result & (buffer_to_data(data_buf,-1) == NULL);
    
    data2 = buffer_to_data(data_buf, buf_size);
    
    assert(data->datasize == data2->datasize && memcmp(data->data, data2->data, data->datasize) == 0);
    result = result && (data->datasize == data2->datasize) &&
                 (memcmp(data->data, data2->data, data->datasize) == 0);
    
    data_destroy(data);
    data_destroy(data2);
    free(data_buf);
    
    printf("serialize - testDeserializeData: %s\n",result?"passou":"não passou");
    return result;
}


int testSerializeEntry() {
    int result;
    char* key;
    struct data_t *value;
    struct entry_t *entry;
    char *entry_buf;
    
    printf("Módulo serialize -> teste serialize_entry:");
    
    key = strdup("123abc");
    if ((value = data_create2(strlen("1234567890abc")+1, strdup("1234567890abc"))) == NULL)
        pee("data_create failed - O teste não pode prosseguir");
    
    if ((entry = entry_create(key, value)) == NULL)
        pee("entry_create failed - O teste não pode prosseguir");
    
    assert(entry_to_buffer(NULL, &entry_buf) == -1);
    result = (entry_to_buffer(NULL, &entry_buf) == -1);
    
    assert(entry_to_buffer(entry, NULL) == -1);
    result = result && (entry_to_buffer(entry, NULL) == -1);
    
    int buf_size = entry_to_buffer(entry, &entry_buf);
    assert(buf_size == sizeof(int) + strlen(entry->key)+1 + sizeof(int) + entry->value->datasize);
    result = result && (buf_size == sizeof(int) + strlen(entry->key)+1 + sizeof(int) + entry->value->datasize);
    
    entry_destroy(entry);
    free(entry_buf);
    
    printf("serialize - testSerializeEntry: %s\n",result?"passou":"não passou");
    return result;
}


int testDeserializeEntry() {
    int result;
    char* key;
    struct data_t *value;
    struct entry_t *entry, *entry2;
    char *entry_buf;

    printf("Módulo serialize -> teste deserialize_entry:");
       
    key = strdup("123abc");
    if ((value = data_create2(strlen("1234567890abc")+1, strdup("1234567890abc"))) == NULL)
        pee("data_create failed - O teste não pode prosseguir");
       
    if ((entry = entry_create(key, value)) == NULL)
        pee("entry_create failed - O teste não pode prosseguir");
    
    int buf_size = entry_to_buffer(entry, &entry_buf);
    if (buf_size <= 0)
        pee("entry_to_buffer failed - O teste não pode prosseguir");
    
    assert(buffer_to_entry(NULL,-1) == NULL);
    result = (buffer_to_entry(NULL,-1) == NULL);
    
    assert(buffer_to_entry(entry_buf,-1) == NULL);
    result = result & (buffer_to_entry(entry_buf,-1) == NULL);
    
    entry2 = buffer_to_entry(entry_buf, buf_size);
    
    assert(memcmp(entry->key, entry2->key, strlen(key)) == 0 && entry->value->datasize == entry2->value->datasize && memcmp(entry->value->data, entry2->value->data, entry->value->datasize) == 0);
    result = result && memcmp(entry->key, entry2->key, strlen(key)) == 0 && entry->value->datasize == entry2->value->datasize && memcmp(entry->value->data, entry2->value->data, entry->value->datasize) == 0;
    
    entry_destroy(entry);
    entry_destroy(entry2);
    free(entry_buf);
    
    printf("serialize - testDeserializeData: %s\n",result?"passou":"não passou");
    return result;
}


/**************************************************************/
int main() {
	int score = 0;

	printf("\nIniciando o teste do módulo serialize \n");

	score += testSerializeData();

    score += testDeserializeData();

    score += testSerializeEntry();

    score += testDeserializeEntry();

	printf("teste serialize bin: %d/4\n",score);

    if (score == 4)
        return 0;
    else
        return -1;
}
