#include "unity.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

// Test data structure
typedef struct {
    uint32_t value;
} test_data_t;

// Global variables for tests
static Queue_t test_queue;
static QueueItem_t queue_item;
static test_data_t test_buffer[10];

void setUp(void) {
    // Reset queue before each test
    memset(&test_queue, 0, sizeof(Queue_t));
    memset(&queue_item, 0, sizeof(QueueItem_t));
    memset(test_buffer, 0, sizeof(test_buffer));
}

void tearDown(void) {
    // Free queue memory after each test
    if (test_queue.buffer != NULL) {
        Queue_free(&test_queue);
    }
}

// ==================== Queue_Init Tests ====================

void test_QueueInit_ValidParams_InitializesCorrectly(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    
    Queue_Init(&test_queue, &queue_item, 5);
    
    TEST_ASSERT_NOT_NULL(test_queue.buffer);
    TEST_ASSERT_EQUAL(0, test_queue.head);
    TEST_ASSERT_EQUAL(0, test_queue.tail);
    TEST_ASSERT_EQUAL(5, test_queue.capacity);
    TEST_ASSERT_EQUAL(QUEUE_EMPTY, test_queue.status);
}

void test_QueueInit_CreatesCorrectCapacity(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    
    Queue_Init(&test_queue, &queue_item, 10);
    
    TEST_ASSERT_EQUAL(10, test_queue.capacity);
}

// ==================== Queue_Push Tests ====================

void test_QueuePush_SingleItem_ReturnsPointer(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 5);
    
    test_data_t data = {.value = 42};
    void* result = Queue_Push(&test_queue, &data);
    
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL(1, test_queue.head);
    TEST_ASSERT_EQUAL(0, test_queue.tail);
    TEST_ASSERT_EQUAL(QUEUE_OK, test_queue.status);
}

void test_QueuePush_MultipleItems_WrapsAround(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 3);
    
    test_data_t data1 = {.value = 1};
    test_data_t data2 = {.value = 2};
    test_data_t data3 = {.value = 3};
    
    Queue_Push(&test_queue, &data1);
    Queue_Push(&test_queue, &data2);
    
    // Pop one to make space
    test_data_t popped;
    Queue_Pop(&test_queue, &popped);
    
    // Push third - should wrap around
    Queue_Push(&test_queue, &data3);
    
    TEST_ASSERT_EQUAL(QUEUE_OK, test_queue.status);
}

void test_QueuePush_WhenFull_SetsFullStatus(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 2);
    
    test_data_t data = {.value = 100};
    
    Queue_Push(&test_queue, &data);
    Queue_Push(&test_queue, &data);
    Queue_Push(&test_queue, &data); // This should cause full
    
    TEST_ASSERT_EQUAL(QUEUE_FULL, test_queue.status);
}

void test_QueuePush_NullQueue_ReturnsNull(void) {
    test_data_t data = {.value = 42};
    void* result = Queue_Push(NULL, &data);
    
    TEST_ASSERT_NULL(result);
}

// ==================== Queue_Pop Tests ====================

void test_QueuePop_SingleItem_RetrievesCorrectly(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 5);
    
    test_data_t push_data = {.value = 99};
    Queue_Push(&test_queue, &push_data);
    
    test_data_t pop_data;
    Queue_Pop(&test_queue, &pop_data);
    
    TEST_ASSERT_EQUAL(99, pop_data.value);
    TEST_ASSERT_EQUAL(QUEUE_EMPTY, test_queue.status);
    TEST_ASSERT_EQUAL(1, test_queue.tail);
}

void test_QueuePop_FIFO_Order_Maintained(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 5);
    
    test_data_t data1 = {.value = 1};
    test_data_t data2 = {.value = 2};
    test_data_t data3 = {.value = 3};
    
    Queue_Push(&test_queue, &data1);
    Queue_Push(&test_queue, &data2);
    Queue_Push(&test_queue, &data3);
    
    test_data_t popped;
    Queue_Pop(&test_queue, &popped);
    TEST_ASSERT_EQUAL(1, popped.value);
    
    Queue_Pop(&test_queue, &popped);
    TEST_ASSERT_EQUAL(2, popped.value);
    
    Queue_Pop(&test_queue, &popped);
    TEST_ASSERT_EQUAL(3, popped.value);
}

void test_QueuePop_EmptyQueue_DoesNotCrash(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 5);
    
    test_data_t data;
    Queue_Pop(&test_queue, &data); // Should not crash
    
    TEST_ASSERT_EQUAL(QUEUE_EMPTY, test_queue.status);
}

void test_QueuePop_NullData_DoesNotCrash(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 5);
    
    test_data_t push_data = {.value = 42};
    Queue_Push(&test_queue, &push_data);
    
    Queue_Pop(&test_queue, NULL); // Should not crash
    
    TEST_ASSERT_EQUAL(QUEUE_EMPTY, test_queue.status);
}

// ==================== Queue_Peek Tests ====================

void test_QueuePeek_ReturnsHeadWithoutRemoving(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 5);
    
    test_data_t data = {.value = 77};
    Queue_Push(&test_queue, &data);
    
    size_t original_tail = test_queue.tail;
    void* peeked = Queue_Peek(&test_queue);
    
    TEST_ASSERT_NOT_NULL(peeked);
    TEST_ASSERT_EQUAL(original_tail, test_queue.tail); // Tail unchanged
    TEST_ASSERT_EQUAL(QUEUE_OK, test_queue.status);
}

void test_QueuePeek_EmptyQueue_ReturnsNull(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 5);
    
    void* peeked = Queue_Peek(&test_queue);
    
    TEST_ASSERT_NULL(peeked);
}

// ==================== Queue_free Tests ====================

void test_QueueFree_ValidQueue_FreesMemory(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 5);
    
    TEST_ASSERT_NOT_NULL(test_queue.buffer);
    
    Queue_free(&test_queue);
    
    TEST_ASSERT_NULL(test_queue.buffer);
}

// ==================== Integration Tests ====================

void test_QueueIntegration_PushPopCycle_WorksCorrectly(void) {
    queue_item.data = NULL;
    queue_item.sizeof_data = sizeof(test_data_t);
    Queue_Init(&test_queue, &queue_item, 3);
    
    // Fill queue
    for (uint32_t i = 0; i < 3; i++) {
        test_data_t data = {.value = i};
        Queue_Push(&test_queue, &data);
    }
    
    // Empty queue
    for (uint32_t i = 0; i < 3; i++) {
        test_data_t data;
        Queue_Pop(&test_queue, &data);
        TEST_ASSERT_EQUAL(i, data.value);
    }
    
    TEST_ASSERT_EQUAL(QUEUE_EMPTY, test_queue.status);
    
    // Refill queue
    for (uint32_t i = 10; i < 13; i++) {
        test_data_t data = {.value = i};
        Queue_Push(&test_queue, &data);
    }
    
    // Verify
    test_data_t data;
    Queue_Pop(&test_queue, &data);
    TEST_ASSERT_EQUAL(10, data.value);
}

// ==================== Main ====================

int main(void) {
    UNITY_BEGIN();
    
    // Init tests
    RUN_TEST(test_QueueInit_ValidParams_InitializesCorrectly);
    RUN_TEST(test_QueueInit_CreatesCorrectCapacity);
    
    // Push tests
    RUN_TEST(test_QueuePush_SingleItem_ReturnsPointer);
    RUN_TEST(test_QueuePush_MultipleItems_WrapsAround);
    RUN_TEST(test_QueuePush_WhenFull_SetsFullStatus);
    RUN_TEST(test_QueuePush_NullQueue_ReturnsNull);
    
    // Pop tests
    RUN_TEST(test_QueuePop_SingleItem_RetrievesCorrectly);
    RUN_TEST(test_QueuePop_FIFO_Order_Maintained);
    RUN_TEST(test_QueuePop_EmptyQueue_DoesNotCrash);
    RUN_TEST(test_QueuePop_NullData_DoesNotCrash);
    
    // Peek tests
    RUN_TEST(test_QueuePeek_ReturnsHeadWithoutRemoving);
    RUN_TEST(test_QueuePeek_EmptyQueue_ReturnsNull);
    
    // Free tests
    RUN_TEST(test_QueueFree_ValidQueue_FreesMemory);
    
    // Integration tests
    RUN_TEST(test_QueueIntegration_PushPopCycle_WorksCorrectly);
    
    return UNITY_END();
}
