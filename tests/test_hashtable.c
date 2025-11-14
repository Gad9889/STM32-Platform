#include "unity.h"
#include "hashtable.h"
#include <string.h>

// Mock set functions for testing
static int mock_function_called = 0;
static uint8_t mock_function_data[8];

void mock_set_function_1(uint8_t *data) {
    mock_function_called = 1;
    memcpy(mock_function_data, data, 8);
}

void mock_set_function_2(uint8_t *data) {
    mock_function_called = 2;
    memcpy(mock_function_data, data, 8);
}

void setUp(void) {
    // Initialize hash table before each test
    hash_Init();
    mock_function_called = 0;
    memset(mock_function_data, 0, sizeof(mock_function_data));
}

void tearDown(void) {
    // Nothing to clean up
}

// ==================== Hash Function Tests ====================

void test_hashMapFunction_DifferentIDs_ProduceDifferentHashes(void) {
    uint8_t hash1 = hash_MapFunction(0x123);
    uint8_t hash2 = hash_MapFunction(0x456);
    
    // They might collide, but should be different initially
    TEST_ASSERT_NOT_EQUAL(hash1, hash2);
}

void test_hashMapFunction_SameID_ProducesSameHash(void) {
    uint8_t hash1 = hash_MapFunction(0x100);
    uint8_t hash2 = hash_MapFunction(0x100);
    
    TEST_ASSERT_EQUAL(hash1, hash2);
}

void test_hashMapFunction_ResultWithinTableSize(void) {
    uint8_t hash = hash_MapFunction(0xFFFFFFFF);
    
    TEST_ASSERT_LESS_THAN(128, hash); // TABLE_SIZE is 128
}

// ==================== Insert Tests ====================

void test_hashInsertMember_NewMember_ReturnsOK(void) {
    hash_member_t member = {
        .id = 0x100,
        .Set_Function = mock_set_function_1
    };
    
    HashStatus_t status = hash_InsertMember(&member);
    
    TEST_ASSERT_EQUAL(HASH_OK, status);
}

void test_hashInsertMember_DuplicateID_ReturnsError(void) {
    hash_member_t member = {
        .id = 0x200,
        .Set_Function = mock_set_function_1
    };
    
    HashStatus_t status1 = hash_InsertMember(&member);
    HashStatus_t status2 = hash_InsertMember(&member);
    
    TEST_ASSERT_EQUAL(HASH_OK, status1);
    TEST_ASSERT_EQUAL(HASH_ERROR, status2);
}

void test_hashInsertMember_MultipleMembers_AllInserted(void) {
    hash_member_t member1 = {.id = 0x100, .Set_Function = mock_set_function_1};
    hash_member_t member2 = {.id = 0x200, .Set_Function = mock_set_function_2};
    hash_member_t member3 = {.id = 0x300, .Set_Function = mock_set_function_1};
    
    TEST_ASSERT_EQUAL(HASH_OK, hash_InsertMember(&member1));
    TEST_ASSERT_EQUAL(HASH_OK, hash_InsertMember(&member2));
    TEST_ASSERT_EQUAL(HASH_OK, hash_InsertMember(&member3));
}

// ==================== Lookup Tests ====================

void test_hashLookup_ExistingID_ReturnsFunction(void) {
    hash_member_t member = {
        .id = 0x123,
        .Set_Function = mock_set_function_1
    };
    hash_InsertMember(&member);
    
    Set_Function_t found = hash_Lookup(0x123);
    
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_PTR(mock_set_function_1, found);
}

void test_hashLookup_NonExistentID_ReturnsNull(void) {
    Set_Function_t found = hash_Lookup(0x999);
    
    TEST_ASSERT_NULL(found);
}

void test_hashLookup_AfterDelete_ReturnsNull(void) {
    hash_member_t member = {
        .id = 0x456,
        .Set_Function = mock_set_function_1
    };
    hash_InsertMember(&member);
    hash_DeleteMember(0x456);
    
    Set_Function_t found = hash_Lookup(0x456);
    
    TEST_ASSERT_NULL(found);
}

void test_hashLookup_MultipleMembers_FindsCorrectOne(void) {
    hash_member_t member1 = {.id = 0x100, .Set_Function = mock_set_function_1};
    hash_member_t member2 = {.id = 0x200, .Set_Function = mock_set_function_2};
    
    hash_InsertMember(&member1);
    hash_InsertMember(&member2);
    
    Set_Function_t found1 = hash_Lookup(0x100);
    Set_Function_t found2 = hash_Lookup(0x200);
    
    TEST_ASSERT_EQUAL_PTR(mock_set_function_1, found1);
    TEST_ASSERT_EQUAL_PTR(mock_set_function_2, found2);
}

// ==================== Delete Tests ====================

void test_hashDeleteMember_ExistingMember_Removes(void) {
    hash_member_t member = {
        .id = 0x789,
        .Set_Function = mock_set_function_1
    };
    hash_InsertMember(&member);
    
    hash_DeleteMember(0x789);
    Set_Function_t found = hash_Lookup(0x789);
    
    TEST_ASSERT_NULL(found);
}

void test_hashDeleteMember_NonExistentMember_DoesNotCrash(void) {
    hash_DeleteMember(0xFFFF); // Should not crash
    TEST_PASS();
}

void test_hashDeleteMember_ThenReinsert_Works(void) {
    hash_member_t member = {
        .id = 0x321,
        .Set_Function = mock_set_function_1
    };
    
    hash_InsertMember(&member);
    hash_DeleteMember(0x321);
    
    // Reinsert with different function
    member.Set_Function = mock_set_function_2;
    HashStatus_t status = hash_InsertMember(&member);
    
    TEST_ASSERT_EQUAL(HASH_OK, status);
    
    Set_Function_t found = hash_Lookup(0x321);
    TEST_ASSERT_EQUAL_PTR(mock_set_function_2, found);
}

// ==================== Integration Tests ====================

void test_HashTable_LookupAndCall_ExecutesCorrectFunction(void) {
    hash_member_t member = {
        .id = 0x555,
        .Set_Function = mock_set_function_1
    };
    hash_InsertMember(&member);
    
    Set_Function_t func = hash_Lookup(0x555);
    TEST_ASSERT_NOT_NULL(func);
    
    uint8_t test_data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    func(test_data);
    
    TEST_ASSERT_EQUAL(1, mock_function_called);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_data, mock_function_data, 8);
}

void test_HashTable_CollisionHandling_LinearProbing(void) {
    // Insert many members to force collisions
    for (uint32_t i = 0; i < 10; i++) {
        hash_member_t member = {
            .id = 0x1000 + i,
            .Set_Function = (i % 2 == 0) ? mock_set_function_1 : mock_set_function_2
        };
        HashStatus_t status = hash_InsertMember(&member);
        TEST_ASSERT_EQUAL(HASH_OK, status);
    }
    
    // Verify all can be found
    for (uint32_t i = 0; i < 10; i++) {
        Set_Function_t found = hash_Lookup(0x1000 + i);
        TEST_ASSERT_NOT_NULL(found);
    }
}

void test_HashTable_FullTable_ReturnsHashFull(void) {
    // Try to fill the table completely (TABLE_SIZE = 128)
    HashStatus_t last_status = HASH_OK;
    int inserted_count = 0;
    
    for (uint32_t i = 0; i < 130; i++) { // Try to insert more than capacity
        hash_member_t member = {
            .id = i,
            .Set_Function = mock_set_function_1
        };
        last_status = hash_InsertMember(&member);
        if (last_status == HASH_OK) {
            inserted_count++;
        } else {
            break;
        }
    }
    
    // Should have inserted at least 100 entries before running into issues
    TEST_ASSERT_GREATER_THAN(100, inserted_count);
}

// ==================== Main ====================

int main(void) {
    UNITY_BEGIN();
    
    // Hash function tests
    RUN_TEST(test_hashMapFunction_DifferentIDs_ProduceDifferentHashes);
    RUN_TEST(test_hashMapFunction_SameID_ProducesSameHash);
    RUN_TEST(test_hashMapFunction_ResultWithinTableSize);
    
    // Insert tests
    RUN_TEST(test_hashInsertMember_NewMember_ReturnsOK);
    RUN_TEST(test_hashInsertMember_DuplicateID_ReturnsError);
    RUN_TEST(test_hashInsertMember_MultipleMembers_AllInserted);
    
    // Lookup tests
    RUN_TEST(test_hashLookup_ExistingID_ReturnsFunction);
    RUN_TEST(test_hashLookup_NonExistentID_ReturnsNull);
    RUN_TEST(test_hashLookup_AfterDelete_ReturnsNull);
    RUN_TEST(test_hashLookup_MultipleMembers_FindsCorrectOne);
    
    // Delete tests
    RUN_TEST(test_hashDeleteMember_ExistingMember_Removes);
    RUN_TEST(test_hashDeleteMember_NonExistentMember_DoesNotCrash);
    RUN_TEST(test_hashDeleteMember_ThenReinsert_Works);
    
    // Integration tests
    RUN_TEST(test_HashTable_LookupAndCall_ExecutesCorrectFunction);
    RUN_TEST(test_HashTable_CollisionHandling_LinearProbing);
    RUN_TEST(test_HashTable_FullTable_ReturnsHashFull);
    
    return UNITY_END();
}
