#ifndef HASHTABLE_H
#define HASHTABLE_H
#include "DbSetFunctions.h"
#include <inttypes.h>

/* ---- configuration ---------------------------------------------------- */
#define TABLE_SIZE     128
#define HASH_EMPTY_ID  0xFFFFFFFF   /* sentinel that marks a free slot     */

/* ---- types ------------------------------------------------------------ */
typedef void (*Set_Function_t)(uint8_t *arg);

typedef struct {
    uint32_t       id;
    Set_Function_t Set_Function;
} hash_member_t;

typedef enum {
    HASH_OK,
    HASH_FULL,
    HASH_ERROR
} HashStatus_t;

uint8_t      hash_MapFunction(uint32_t id);
void         hash_PrintTable(void);
HashStatus_t hash_InsertMember(const hash_member_t *member);
Set_Function_t hash_Lookup(uint32_t id);
void         hash_DeleteMember(uint32_t id);
HashStatus_t hash_SetTable(void);
HashStatus_t hash_Init(void);

#endif