/**
 * @file test_example.c
 * @brief Example unit test structure for your application logic
 * 
 * This is a template showing how to write tests for your own code.
 * Copy and modify this file for each module you want to test.
 */

#include "unity.h"
// #include "your_module.h"  // Include your module header

void setUp(void) {
    // Setup before each test
    // Initialize variables, reset state, etc.
}

void tearDown(void) {
    // Cleanup after each test
    // Free memory, close files, etc.
}

/* ==================== Example Tests ==================== */

// Example: Testing a calculation function
void test_CalculateSpeed_WithValidInput_ReturnsCorrectValue(void) {
    // Arrange
    // int wheel_rpm = 1000;
    // float wheel_diameter = 0.5f;
    
    // Act
    // float speed = CalculateSpeed(wheel_rpm, wheel_diameter);
    
    // Assert
    // TEST_ASSERT_FLOAT_WITHIN(0.01f, 26.18f, speed);
    
    TEST_IGNORE_MESSAGE("Replace with your actual test");
}

// Example: Testing state machine transitions
void test_StateMachine_OnStartEvent_TransitionsToRunning(void) {
    // Arrange
    // InitStateMachine();
    // TEST_ASSERT_EQUAL(STATE_IDLE, GetCurrentState());
    
    // Act
    // ProcessEvent(EVENT_START);
    
    // Assert
    // TEST_ASSERT_EQUAL(STATE_RUNNING, GetCurrentState());
    
    TEST_IGNORE_MESSAGE("Replace with your actual test");
}

// Example: Testing boundary conditions
void test_SensorFilter_WithOutlier_FiltersCorrectly(void) {
    // Arrange
    // float readings[] = {10.0f, 10.1f, 9.9f, 100.0f, 10.2f};
    
    // Act
    // float filtered = ApplySensorFilter(readings, 5);
    
    // Assert
    // TEST_ASSERT_FLOAT_WITHIN(0.5f, 10.0f, filtered);
    
    TEST_IGNORE_MESSAGE("Replace with your actual test");
}

// Example: Testing error handling
void test_Parser_WithInvalidData_ReturnsError(void) {
    // Arrange
    // uint8_t invalid_data[] = {0xFF, 0xFF, 0xFF};
    
    // Act
    // int result = ParseMessage(invalid_data, sizeof(invalid_data));
    
    // Assert
    // TEST_ASSERT_EQUAL(-1, result);
    
    TEST_IGNORE_MESSAGE("Replace with your actual test");
}

/* ==================== Main Test Runner ==================== */

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_CalculateSpeed_WithValidInput_ReturnsCorrectValue);
    RUN_TEST(test_StateMachine_OnStartEvent_TransitionsToRunning);
    RUN_TEST(test_SensorFilter_WithOutlier_FiltersCorrectly);
    RUN_TEST(test_Parser_WithInvalidData_ReturnsError);
    
    return UNITY_END();
}
