
#ifndef UTILS_H
#define UTILS_H
/* =============================== Includes ======================================= */
#include "hashtable.h"
/*========================= Queue related definitions =========================*/

/**
 * @brief Queue status
 * @note This enum is used to define the status of the queue
 */
typedef enum{
    QUEUE_OK,
    QUEUE_FULL,
    QUEUE_EMPTY,
    QUEUE_ERROR
} QueueStatus_t;


/**
 * @brief Queue item
 * @note This struct is used to define an item in the queue
 */
typedef struct{
    void* data;
    size_t sizeof_data;    
} QueueItem_t; 

/**
 * @brief Queue
 * @note This struct defines the queue
 */
typedef struct{
    QueueItem_t* buffer;
    size_t head;
    size_t tail;
    size_t capacity;
    QueueStatus_t status;
} Queue_t;


/*========================= Queue related function prototypes =========================*/

void Queue_Init(Queue_t* Q, QueueItem_t* item, size_t size);
void* Queue_Push(Queue_t* Q, void* data);
void Queue_Pop(Queue_t* Q, void* data);
void Queue_free(Queue_t* Q);
void* Queue_Peek(Queue_t* Q);




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