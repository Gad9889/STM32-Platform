#include "utils.h"
/*================================== Queue implementation ===============================*/
/**
  * @brief  Initializes the queue with the specified item size and capacity.
  * @param  Q    Pointer to the queue structure
  * @param  item Pointer to the queue item structure
  * @param  size Capacity of the queue
  *
  * @note   Allocates memory for the queue items and initializes the head, tail,
  *         capacity, and status of the queue.
  * 
  */
void Queue_Init(Queue_t* Q, QueueItem_t* item, size_t size){
    Q->buffer = (QueueItem_t *)malloc(size * sizeof(QueueItem_t));
    for (size_t i = 0; i < size; i++) {
        Q->buffer[i].data = calloc(1,item->sizeof_data);
        Q->buffer[i].sizeof_data = item->sizeof_data;
    }
    Q->head = 0;
    Q->tail = 0;
    Q->capacity = size;
    Q->status = QUEUE_EMPTY;
    return;
}

/**
  * @brief  Pushes data into the queue.
  * @param  Q    Pointer to the queue structure
  * @param  data Pointer to the data to be pushed
  * @retval Pointer to the data pushed
  * 
  * @note   Copies the data into the queue at the head index and updates the
  *         head index. If the queue is full, the tail index is updated to maintain
  *         a circular buffer behavior. Updates the status of the queue.
  */
void* Queue_Push(Queue_t* Q, void* data){
    
    void* pointer = Q->buffer[Q->head].data;
    memcpy(pointer, data, Q->buffer->sizeof_data);
    Q->head = (Q->head + 1) % Q->capacity;
    
    if(Q->status == QUEUE_FULL){
        Q->tail = (Q->tail + 1) % Q->capacity;
    }

   /**/
   if(Q->head == Q->tail)
    {
     Q->status = QUEUE_FULL ;
    }
    else
    {
        Q->status = QUEUE_OK ;
    }

    
    
    return pointer;
}


/**
  * @brief  Pops data from the queue.
  * @param  Q    Pointer to the queue structure
  * @param  data Pointer to the data where popped data will be stored
  *
  * @note   Copies the data from the queue at the tail index and updates the
  *         tail index. Updates the status of the queue.
  */
void Queue_Pop(Queue_t* Q, void* data){
    if(Q->status == QUEUE_EMPTY){
        return;
    }
    if(data != NULL){
        memcpy(data, Q->buffer[Q->tail].data, Q->buffer->sizeof_data);
    }
    Q->tail = (Q->tail + 1) % Q->capacity;
    Q->status = (Q->head == Q->tail) ? QUEUE_EMPTY : QUEUE_OK;
    return;
}

/**
  * @brief  Peeks at the data in the queue.
  * @param  Q Pointer to the queue structure
  * @retval Pointer to the data at the tail index
  * 
  * @note   Returns the data pointer at the tail index without popping it.
  */
void* Queue_Peek(Queue_t* Q){
    return Q->buffer[Q->tail].data;
}


/**
 * @brief  Frees the memory allocated for the queue items.
 * @param  Q Pointer to the queue structure
 * 
 * @note   Frees the memory allocated for each queue item and the buffer itself.
 */
void Queue_free(Queue_t* Q){
    for (size_t i = 0; i < Q->capacity; i++) {
        free(Q->buffer[i].data);
    }
    free(Q->buffer);
    return;
}
