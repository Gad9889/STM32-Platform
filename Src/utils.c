/**
 * @file utils.c
 * @brief Utility functions - Thread-safe queue implementation
 */

#include "utils.h"
#include <stdlib.h>
#include <string.h>

/*================================== Queue implementation ===============================*/

/**
 * @brief Enter critical section (disable interrupts)
 * @note For ARM Cortex-M: disables interrupts and returns PRIMASK state
 *       For testing: no-op stub that returns 0
 */
static inline uint32_t Queue_EnterCritical(void) {
#if defined(__ARM_ARCH) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) || defined(__ARMCC_VERSION)
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
#else
    // Stub for unit testing on x86/x64
    return 0;
#endif
}

/**
 * @brief Exit critical section (restore interrupts)
 * @note For ARM Cortex-M: restores PRIMASK state
 *       For testing: no-op stub
 */
static inline void Queue_ExitCritical(uint32_t primask) {
#if defined(__ARM_ARCH) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) || defined(__ARMCC_VERSION)
    __set_PRIMASK(primask);
#else
    // Stub for unit testing on x86/x64
    (void)primask; // Suppress unused parameter warning
#endif
}

plt_status_t Queue_Init(Queue_t* queue, size_t item_size, size_t capacity) {
    if (queue == NULL) {
        return PLT_NULL_POINTER;
    }
    
    if (item_size == 0 || capacity == 0) {
        return PLT_INVALID_PARAM;
    }
    
    // Limit capacity to reasonable size
    if (capacity > 1024) {
        return PLT_INVALID_PARAM;
    }
    
    // Allocate contiguous buffer for all items
    queue->buffer = calloc(capacity, item_size);
    if (queue->buffer == NULL) {
        return PLT_NO_MEMORY;
    }
    
    queue->item_size = item_size;
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    
    return PLT_OK;
}

plt_status_t Queue_Push(Queue_t* queue, const void* data) {
    if (queue == NULL || data == NULL) {
        return PLT_NULL_POINTER;
    }
    
    if (queue->buffer == NULL) {
        return PLT_NOT_INITIALIZED;
    }
    
    // Enter critical section
    uint32_t primask = Queue_EnterCritical();
    
    // Check if full
    if (queue->count >= queue->capacity) {
        Queue_ExitCritical(primask);
        return PLT_QUEUE_FULL;
    }
    
    // Copy data to buffer
    void* dest = (uint8_t*)queue->buffer + (queue->head * queue->item_size);
    memcpy(dest, data, queue->item_size);
    
    // Update head and count
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count++;
    
    Queue_ExitCritical(primask);
    return PLT_OK;
}

plt_status_t Queue_Pop(Queue_t* queue, void* data) {
    if (queue == NULL) {
        return PLT_NULL_POINTER;
    }
    
    if (queue->buffer == NULL) {
        return PLT_NOT_INITIALIZED;
    }
    
    // Enter critical section
    uint32_t primask = Queue_EnterCritical();
    
    // Check if empty
    if (queue->count == 0) {
        Queue_ExitCritical(primask);
        return PLT_QUEUE_EMPTY;
    }
    
    // Copy data from buffer (if destination provided)
    if (data != NULL) {
        void* src = (uint8_t*)queue->buffer + (queue->tail * queue->item_size);
        memcpy(data, src, queue->item_size);
    }
    
    // Update tail and count
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count--;
    
    Queue_ExitCritical(primask);
    return PLT_OK;
}

plt_status_t Queue_Peek(Queue_t* queue, void* data) {
    if (queue == NULL || data == NULL) {
        return PLT_NULL_POINTER;
    }
    
    if (queue->buffer == NULL) {
        return PLT_NOT_INITIALIZED;
    }
    
    // Enter critical section (quick read)
    uint32_t primask = Queue_EnterCritical();
    
    // Check if empty
    if (queue->count == 0) {
        Queue_ExitCritical(primask);
        return PLT_QUEUE_EMPTY;
    }
    
    // Copy data without removing
    void* src = (uint8_t*)queue->buffer + (queue->tail * queue->item_size);
    memcpy(data, src, queue->item_size);
    
    Queue_ExitCritical(primask);
    return PLT_OK;
}

size_t Queue_Count(Queue_t* queue) {
    if (queue == NULL) {
        return 0;
    }
    
    // Atomic read of volatile variable
    return queue->count;
}

bool Queue_IsEmpty(Queue_t* queue) {
    return Queue_Count(queue) == 0;
}

bool Queue_IsFull(Queue_t* queue) {
    if (queue == NULL) {
        return false;
    }
    return queue->count >= queue->capacity;
}

void Queue_Free(Queue_t* queue) {
    if (queue == NULL) {
        return;
    }
    
    if (queue->buffer != NULL) {
        free(queue->buffer);
        queue->buffer = NULL;
    }
    
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->capacity = 0;
}
