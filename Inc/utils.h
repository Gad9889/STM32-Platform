
#ifndef UTILS_H
#define UTILS_H
/* =============================== Includes ======================================= */
#include "hashtable.h"
#include "platform_status.h"
#include <stdbool.h>

// Include CMSIS core for ARM intrinsics on target platform
#if defined(__ARM_ARCH) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) || defined(__ARMCC_VERSION)
#include "cmsis_gcc.h"
#endif

/*========================= Queue related definitions =========================*/

/**
 * @brief Thread-safe circular queue for ISR-safe message passing
 * 
 * This queue uses atomic operations (interrupt disable/enable) to ensure
 * thread safety between ISRs and main loop.
 */
typedef struct{
    void* buffer;           ///< Contiguous buffer for all items
    size_t item_size;       ///< Size of each item in bytes
    volatile size_t head;   ///< Write index (producer)
    volatile size_t tail;   ///< Read index (consumer)
    size_t capacity;        ///< Maximum number of items
    volatile size_t count;  ///< Current number of items
} Queue_t;


/*========================= Queue related function prototypes =========================*/

/**
 * @brief Initialize a queue with fixed-size items
 * @param queue Pointer to queue structure
 * @param item_size Size of each item in bytes
 * @param capacity Maximum number of items
 * @return PLT_OK on success, error code otherwise
 */
plt_status_t Queue_Init(Queue_t* queue, size_t item_size, size_t capacity);

/**
 * @brief Push an item into the queue (thread-safe)
 * @param queue Pointer to queue structure
 * @param data Pointer to data to push
 * @return PLT_OK on success, PLT_QUEUE_FULL if full
 */
plt_status_t Queue_Push(Queue_t* queue, const void* data);

/**
 * @brief Pop an item from the queue (thread-safe)
 * @param queue Pointer to queue structure  
 * @param data Pointer to buffer for popped data
 * @return PLT_OK on success, PLT_QUEUE_EMPTY if empty
 */
plt_status_t Queue_Pop(Queue_t* queue, void* data);

/**
 * @brief Peek at front item without removing (thread-safe)
 * @param queue Pointer to queue structure
 * @param data Pointer to buffer for peeked data
 * @return PLT_OK on success, PLT_QUEUE_EMPTY if empty
 */
plt_status_t Queue_Peek(Queue_t* queue, void* data);

/**
 * @brief Get current number of items in queue
 * @param queue Pointer to queue structure
 * @return Number of items (thread-safe read)
 */
size_t Queue_Count(Queue_t* queue);

/**
 * @brief Check if queue is empty
 */
bool Queue_IsEmpty(Queue_t* queue);

/**
 * @brief Check if queue is full
 */
bool Queue_IsFull(Queue_t* queue);

/**
 * @brief Free queue memory
 * @param queue Pointer to queue structure
 */
void Queue_Free(Queue_t* queue);




/* =============================== Macros ========================================= */
/**
 * @brief Error handler
 */
#define VALID(x) do{if((x) != HAL_OK){Error_Handler();}}while(0)

/**
 * @brief Delayed action
 * @note This macro is used to execute an action after a certain delay
 */
#define DELAYED(timer, delay, action) do{if(HAL_GetTick() - (timer) > (delay)){(action); (timer) = HAL_GetTick();}}while(0)


#endif // UTILS_H