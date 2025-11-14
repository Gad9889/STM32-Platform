#include "hashtable.h"

hash_member_t HashTable[TABLE_SIZE];

static void clear_table(void)
{
	for (int i = 0; i < TABLE_SIZE; ++i) {
		HashTable[i].id          = HASH_EMPTY_ID;
		HashTable[i].Set_Function = NULL;
	}
}

/* hashtable.c ----------------------------------------------------------- */
uint8_t hash_MapFunction(uint32_t id)
{
	// Input validation - check for reserved IDs
	if (id == HASH_EMPTY_ID) {
		return 0; // Return default hash for invalid ID
	}
	
	/* ---- perfect hash for the two blocks we care about --------------- */
	if (id >= 0x180 && id <= 0x19E)                 /* 0x180-0x19E → 0-30 */
		return (uint8_t)(id - 0x180);

	if (id >= 0x280 && id <= 0x29E)                 /* 0x280-0x29E → 31-61 */
		return (uint8_t)(31 + (id - 0x280));

	/* ---- fallback for everything else -------------------------------- */
	id ^= id >> 16;  id *= 0x45d9f3b;  id ^= id >> 16;
	id *= 0x45d9f3b; id ^= id >> 16;
	return id % TABLE_SIZE;
}

HashStatus_t hash_InsertMember(const hash_member_t *member)
{
	if (!member) return HASH_ERROR;
	
	// Validate member data
	if (member->id == HASH_EMPTY_ID || member->Set_Function == NULL) {
		return HASH_ERROR;
	}

	int start = hash_MapFunction(member->id);
	for (int i = 0; i < TABLE_SIZE; ++i) {
		int idx = (start + i) % TABLE_SIZE;

		if (HashTable[idx].id == HASH_EMPTY_ID) {   /* free slot */
			HashTable[idx] = *member;               /* structure copy */
			return HASH_OK;
		}
	}
	return HASH_FULL;
}
Set_Function_t hash_Lookup(uint32_t id)
{
	int start = hash_MapFunction(id);
	for (int i = 0; i < TABLE_SIZE; ++i) {
		int idx = (start + i) % TABLE_SIZE;
		if (HashTable[idx].id == id)
			return HashTable[idx].Set_Function;
	}
	return NULL;
}

void hash_DeleteMember(uint32_t id)
{
	int start = hash_MapFunction(id);
	for (int i = 0; i < TABLE_SIZE; ++i) {
		int idx = (start + i) % TABLE_SIZE;
		if (HashTable[idx].id == id) {
			HashTable[idx].id = HASH_EMPTY_ID;
			HashTable[idx].Set_Function = NULL;
			return;
		}
	}
}

/*
void hash_PrintTable(void)
{
	puts("Start");
	for (int i = 0; i < TABLE_SIZE; ++i) {
		if (HashTable[i].id == HASH_EMPTY_ID)
			printf("\t%2d\t---\n", i);
		else
			printf("\t%2d\t0x%" PRIx32 "\n", i, HashTable[i].id);
	}
	puts("End");
}
*/

HashStatus_t hash_SetTable(void)
{
	hash_member_t member;

	/* ---- set the table with the functions we care about ---------------- */
	member.id = INV1_AV1_ID;
	member.Set_Function = setInv1Av1Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	member.id = INV1_AV2_ID;
	member.Set_Function = setInv1Av2Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;


	member.id = INV2_AV1_ID;
	member.Set_Function = setInv2Av1Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	member.id = INV2_AV2_ID;
	member.Set_Function = setInv2Av2Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;



	member.id = INV3_AV1_ID;
	member.Set_Function = setInv3Av1Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	member.id = INV3_AV2_ID;
	member.Set_Function = setInv3Av2Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;


	member.id = INV4_AV1_ID;
	member.Set_Function = setInv4Av1Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	member.id = INV4_AV2_ID;
	member.Set_Function = setInv4Av2Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;


	member.id = STAGE_0_ID;
	member.Set_Function = setStage0Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	member.id = STAGE_1_ID;
	member.Set_Function = setStage1Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	member.id = STAGE_2_ID;
	member.Set_Function = setStage2Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	member.id = STAGE_3_ID;
	member.Set_Function = setStage3Parameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;



	member.id = BMS_ID;
	member.Set_Function = setBmsParameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	member.id = RES_ID;
	member.Set_Function = setResParameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	member.id = PEDAL_ID;
	member.Set_Function = setPedalParameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	member.id = DB_ID;
	member.Set_Function = setDBParameters;
	if (hash_InsertMember(&member) != HASH_OK) return HASH_ERROR;

	return HASH_OK;

}

HashStatus_t hash_Init(void)
{
	clear_table();             /* replaces malloc/calloc/free path */
	//return hash_SetTable();
}

