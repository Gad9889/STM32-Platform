#pragma once

#include "DbSetFunctions.h"


typedef struct
{
	uint32_t id;
	FunctionPointer func;
}hash_member_t;

typedef enum{
    HASH_OK,
    HASH_FULL,
    HASH_EMPTY,
    HASH_ERROR
} HashStatus_t;

// ID's defines

#define PEDAL_ID 0x30


//functions
uint8_t hash_MapFunction(uint32_t id);
HashStatus_t hash_AllocateMemory();
void hash_PrintTable();
uint8_t hash_InsertMember(hash_member_t * hmember);
FunctionPointer hash_Lookup(uint32_t id);
void hash_DeleteMember(uint32_t id);
void hash_FreeTable();
HashStatus_t hash_SetTable();
HashStatus_t hash_Init();