#include "../Inc/hashtable.h"
#include <inttypes.h>

#define TABLE_SIZE 50

hash_member_t ** pHashTable;


//Map name to location in the hash table
uint8_t hash_MapFunction(uint32_t id) 
{
	// Need to fix to hash function that give different inxd to the true_ can ids

	id ^= id >> 16;
	id *= 0x45d9f3b;
	id ^= id >> 16;
	id *= 0x45d9f3b;
	id ^= id >> 16;

	// Return index in hash table
	return id % TABLE_SIZE;
}


//init hash table in heap
HashStatus_t hash_AllocateMemory()
{
    hash_member_t **new_table = (hash_member_t **)calloc(TABLE_SIZE, sizeof(hash_member_t *));
    if (new_table == NULL) {
        return HASH_ERROR; // Allocation failed
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
        new_table[i] = (hash_member_t*)calloc(1,sizeof(hash_member_t));
        if(new_table[i] == NULL){return HASH_ERROR;}
    }
    
    pHashTable = new_table;
    return HASH_OK;
}

//TODO -- need to be fixed to work with stm32 mayby by UART
void hash_PrintTable() {
	printf("Start\n");
	for (int i=0; i<TABLE_SIZE; i++) {
		if (pHashTable[i] == NULL) {
			printf("\t%i\t---\n",i);
		}
		else {
			printf("\t%i\t0x%" PRIx32 "\n",i, pHashTable[i]->id);
		}
	}
	printf("End\n");
}

//hash table insert
uint8_t hash_InsertMember(hash_member_t * hmember) {
	if (hmember == NULL) return 0;
	int index = hash_MapFunction(hmember->id);
	for (int i=0; i<TABLE_SIZE; i++) {
		int try = (i+index) % TABLE_SIZE;

		if (pHashTable[try] == NULL) {
			pHashTable[try] = hmember;
			return 1;
		}
	}
    return 0;
}

//hash lookup function
FunctionPointer hash_Lookup(uint32_t id) {
	uint8_t index = hash_MapFunction(id);
	for (int i=0; i<TABLE_SIZE; i++) {
		int try = (i+index) % TABLE_SIZE;
		if (pHashTable[try] != NULL && pHashTable[try]->id == id) {
			
            return pHashTable[try]->func;
		}
	}
		return NULL;
}

void hash_DeleteMember(uint32_t id) {

	uint8_t index = hash_MapFunction(id);

	for (int i=0; i<TABLE_SIZE; i++) {
		int try = (i+index) % TABLE_SIZE;
		if (pHashTable[try] != NULL && pHashTable[try]->id == id) {
			
			free(pHashTable[try]);
		}
	}
}

void hash_FreeTable()
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        free(pHashTable[i]);
    }
    free(pHashTable);
}
HashStatus_t hash_SetTable()
{
    //pedal member
    hash_member_t pedal_member = {.id = PEDAL_ID ,.func = &setPedalParameters};
    if(!(hash_InsertMember(&pedal_member))){return HASH_ERROR;}


    //add more members
}


HashStatus_t hash_Init()
{   
    HashStatus_t hash_status = HASH_OK;
    hash_status = hash_AllocateMemory();

    if(hash_status == HASH_ERROR){return hash_status;}//dont continue to insert memebers

    hash_status = hash_SetTable();
    return hash_status;
}
/*

void Can_RxCallback(can_message_t * can_massage)
{
	FunctionPointer set_function = hash_Lookup(can_massage->id);
	set_function(can_massage->data);

}



*/