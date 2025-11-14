#include "unity.h"
#include "database.h"
#include <stdlib.h>
#include <string.h>

static database_t *test_db = NULL;

void setUp(void) {
    // Allocate fresh database before each test
    test_db = db_AllocateMemory();
}

void tearDown(void) {
    // Free database after each test
    if (test_db != NULL) {
        db_FreeMemory(test_db);
        test_db = NULL;
    }
}

// ==================== Allocation Tests ====================

void test_dbAllocateMemory_Success_ReturnsValidPointer(void) {
    TEST_ASSERT_NOT_NULL(test_db);
}

void test_dbAllocateMemory_AllocatesAllNodes(void) {
    TEST_ASSERT_NOT_NULL(test_db->pedal_node);
    TEST_ASSERT_NOT_NULL(test_db->sub_node);
    TEST_ASSERT_NOT_NULL(test_db->vcu_node);
    TEST_ASSERT_NOT_NULL(test_db->dashboard_node);
}

void test_dbAllocateMemory_InitializesToZero(void) {
    // Check that all fields are zero-initialized (calloc behavior)
    TEST_ASSERT_EQUAL(0, test_db->pedal_node->gas_value);
    TEST_ASSERT_EQUAL(0, test_db->pedal_node->brake_value);
    TEST_ASSERT_EQUAL(0, test_db->vcu_node->SDC_state);
}

// ==================== Free Tests ====================

void test_dbFreeMemory_ValidDatabase_FreesAllNodes(void) {
    database_t *db = db_AllocateMemory();
    TEST_ASSERT_NOT_NULL(db);
    
    db_FreeMemory(db);
    
    // Can't directly test if freed, but shouldn't crash
    TEST_PASS();
}

// ==================== Init Tests ====================

void test_dbInit_ReturnsInitializedDatabase(void) {
    database_t *db = db_Init();
    
    TEST_ASSERT_NOT_NULL(db);
    TEST_ASSERT_NOT_NULL(db->pedal_node);
    TEST_ASSERT_NOT_NULL(db->sub_node);
    TEST_ASSERT_NOT_NULL(db->vcu_node);
    TEST_ASSERT_NOT_NULL(db->dashboard_node);
    
    db_FreeMemory(db);
}

void test_dbGetDBPointer_AfterInit_ReturnsSamePointer(void) {
    database_t *db = db_Init();
    database_t *retrieved = db_GetDBPointer();
    
    TEST_ASSERT_EQUAL_PTR(db, retrieved);
    
    db_FreeMemory(db);
}

// ==================== Data Access Tests ====================

void test_Database_PedalNode_CanStoreAndRetrieveData(void) {
    test_db->pedal_node->gas_value = 500;
    test_db->pedal_node->brake_value = 750;
    test_db->pedal_node->steering_wheel_angle = -45;
    test_db->pedal_node->BIOPS = 0xAB;
    
    TEST_ASSERT_EQUAL(500, test_db->pedal_node->gas_value);
    TEST_ASSERT_EQUAL(750, test_db->pedal_node->brake_value);
    TEST_ASSERT_EQUAL(-45, test_db->pedal_node->steering_wheel_angle);
    TEST_ASSERT_EQUAL(0xAB, test_db->pedal_node->BIOPS);
}

void test_Database_VCUNode_CanStoreInverterData(void) {
    test_db->vcu_node->inverters[0].actual_speed = 1500;
    test_db->vcu_node->inverters[0].torque = 250;
    test_db->vcu_node->inverters[0].motor_temperature = 65;
    
    TEST_ASSERT_EQUAL(1500, test_db->vcu_node->inverters[0].actual_speed);
    TEST_ASSERT_EQUAL(250, test_db->vcu_node->inverters[0].torque);
    TEST_ASSERT_EQUAL(65, test_db->vcu_node->inverters[0].motor_temperature);
}

void test_Database_VCUNode_MultipleInverters(void) {
    for (int i = 0; i < 4; i++) {
        test_db->vcu_node->inverters[i].actual_speed = 1000 + i * 100;
    }
    
    TEST_ASSERT_EQUAL(1000, test_db->vcu_node->inverters[0].actual_speed);
    TEST_ASSERT_EQUAL(1100, test_db->vcu_node->inverters[1].actual_speed);
    TEST_ASSERT_EQUAL(1200, test_db->vcu_node->inverters[2].actual_speed);
    TEST_ASSERT_EQUAL(1300, test_db->vcu_node->inverters[3].actual_speed);
}

void test_Database_VCUNode_ErrorGroup(void) {
    test_db->vcu_node->error_group.system_error = 0x01;
    test_db->vcu_node->error_group.inv1_error = 0x02;
    test_db->vcu_node->error_group.inv2_error = 0x03;
    test_db->vcu_node->error_group.canbus_error = 0xFF;
    
    TEST_ASSERT_EQUAL(0x01, test_db->vcu_node->error_group.system_error);
    TEST_ASSERT_EQUAL(0x02, test_db->vcu_node->error_group.inv1_error);
    TEST_ASSERT_EQUAL(0x03, test_db->vcu_node->error_group.inv2_error);
    TEST_ASSERT_EQUAL(0xFF, test_db->vcu_node->error_group.canbus_error);
}

void test_Database_VCUNode_KeepAliveArray(void) {
    test_db->vcu_node->keep_alive[PEDALNODE] = 1;
    test_db->vcu_node->keep_alive[INV1] = 1;
    test_db->vcu_node->keep_alive[INV2] = 1;
    
    TEST_ASSERT_EQUAL(1, test_db->vcu_node->keep_alive[PEDALNODE]);
    TEST_ASSERT_EQUAL(1, test_db->vcu_node->keep_alive[INV1]);
    TEST_ASSERT_EQUAL(1, test_db->vcu_node->keep_alive[INV2]);
    TEST_ASSERT_EQUAL(0, test_db->vcu_node->keep_alive[INV3]);
}

void test_Database_SubNode_CanStoreData(void) {
    test_db->sub_node->ASMS = 1;
    test_db->sub_node->water_temp = 85;
    test_db->sub_node->pneumatic_pumps.val1 = 100;
    test_db->sub_node->pneumatic_pumps.val2 = 200;
    
    TEST_ASSERT_EQUAL(1, test_db->sub_node->ASMS);
    TEST_ASSERT_EQUAL(85, test_db->sub_node->water_temp);
    TEST_ASSERT_EQUAL(100, test_db->sub_node->pneumatic_pumps.val1);
    TEST_ASSERT_EQUAL(200, test_db->sub_node->pneumatic_pumps.val2);
}

void test_Database_DashboardNode_CanStoreData(void) {
    test_db->dashboard_node->R2D = 1;
    
    TEST_ASSERT_EQUAL(1, test_db->dashboard_node->R2D);
}

// ==================== Main ====================

int main(void) {
    UNITY_BEGIN();
    
    // Allocation tests
    RUN_TEST(test_dbAllocateMemory_Success_ReturnsValidPointer);
    RUN_TEST(test_dbAllocateMemory_AllocatesAllNodes);
    RUN_TEST(test_dbAllocateMemory_InitializesToZero);
    
    // Free tests
    RUN_TEST(test_dbFreeMemory_ValidDatabase_FreesAllNodes);
    
    // Init tests
    RUN_TEST(test_dbInit_ReturnsInitializedDatabase);
    RUN_TEST(test_dbGetDBPointer_AfterInit_ReturnsSamePointer);
    
    // Data access tests
    RUN_TEST(test_Database_PedalNode_CanStoreAndRetrieveData);
    RUN_TEST(test_Database_VCUNode_CanStoreInverterData);
    RUN_TEST(test_Database_VCUNode_MultipleInverters);
    RUN_TEST(test_Database_VCUNode_ErrorGroup);
    RUN_TEST(test_Database_VCUNode_KeepAliveArray);
    RUN_TEST(test_Database_SubNode_CanStoreData);
    RUN_TEST(test_Database_DashboardNode_CanStoreData);
    
    return UNITY_END();
}
