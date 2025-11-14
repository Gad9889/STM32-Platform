
#include "uart.h"

#ifdef HAL_UART_MODULE_ENABLED
// UART Driver: Implementation for UART communication using DMA for transmission only

/* =============================== Global Variables =============================== */
UART_HandleTypeDef* pUart1;         // UART handle pinters
UART_HandleTypeDef* pUart2;         // UART handle pinters
UART_HandleTypeDef* pUart3;         // UART handle pinters
static handler_set_t* pHandlers = NULL;    // Pointer to the handler set from the platform layer
static plt_callbacks_t* pCallbacks = NULL; // Pointer to the callback function pointers from the platform layer
uart_message_t Uart_TxData = {0};  // UART message structure for transmission
debug_message_t Debug_TxData = {0};  // Debug message structure for transmission
__attribute__((aligned(4))) uint8_t Uart_RxData[2][sizeof(uart_message_t)] = {0};  // DMA buffer for UART reception - DMA aligned
void (*Uart_RxCallback)(uart_message_t *) = NULL;  // Callback function for UART reception

static Queue_t uartRxQueue = {0};
static QueueItem_t uartRxMessage = {
    .data = NULL,
    .sizeof_data = sizeof(uart_message_t)
};

static Queue_t uartTxQueue = {0};
static QueueItem_t uartTxMessage = {
    .data = NULL,
    .sizeof_data = sizeof(uart_message_t)
};

static Queue_t debugTxQueue = {0};
static QueueItem_t debugTxMessage = {
    .data = NULL,
    .sizeof_data = sizeof(debug_message_t)
};

/*========================= Function Definitions =========================*/

/**
  * @brief  Initializes the UART module for transmission and sets up the queue and buffer.
  * @param  pUart Pointer to the UART handle to initialize       
  * @param  tx_queue_size Size of the transmission queue
  * @retval None
  *
  * @note   This function sets up the UART handle and initializes the TX queue and buffer.
  *  ! BE CareFull the Rx DMA channel need to be in Circular mode to work properly
  *  ! BE CareFull the Tx DMA channel need to be in Normal mode to work properly
  */
void plt_UartInit(size_t tx_queue_size)
{
    // NULL pointer checks
    pHandlers = plt_GetHandlersPointer();
    if (pHandlers == NULL) {
        Error_Handler();
        return;
    }
    
    pCallbacks = plt_GetCallbacksPointer();
    if (pCallbacks == NULL) {
        Error_Handler();
        return;
    }
    
    // Bounds check for queue size
    if (tx_queue_size == 0 || tx_queue_size > PLT_MAX_QUEUE_SIZE) {
        Error_Handler();
        return;
    }
    
    Uart_RxCallback = pCallbacks->UART_RxCallback;
    Queue_Init(&uartRxQueue,&uartRxMessage,tx_queue_size);

    Queue_Init(&uartTxQueue,&uartTxMessage,tx_queue_size);  // Initialize the TX queue for UART1 transmission

    if(pHandlers->huart1 != NULL)
    {
        pUart1 = pHandlers->huart1;  // Set the UART handle pointer
        
        if (HAL_UART_Receive_DMA(pUart1,Uart_RxData,(uint16_t)sizeof(uart_message_t)) != HAL_OK) {
            Error_Handler();
            return;
        }
    }


    if(pHandlers->huart2 != NULL)
    {
        pUart2 = pHandlers->huart2;  // Set the UART handle pointer
        Queue_Init(&debugTxQueue,&debugTxMessage,tx_queue_size);  // Initialize the debugging queue for UART2  transmission for 
    }

    if(pHandlers->huart3 != NULL)
    {
        pUart3 = pHandlers->huart3;  // Set the UART handle pointer
        if (HAL_UART_Receive_DMA(pUart3,Uart_RxData,(uint16_t)sizeof(uart_message_t)) != HAL_OK) {
            Error_Handler();
            return;
        }
    }



    
}

/**
 * @brief Send stdios printf to UART
 * 
 * @note This function is used to redirect the printf output to UART, _write is a weak function that is called by printf to write the output to the desired stream
 * @TODO: Implement the function to write the data with UART via DMA
 */

 int _write(int file,   //FILE DESCRIPTOR
     char *ptr,        //POINTER TO DATA
      int len)        //DATA LENGTH 
{
    UNUSED(file);

    if(pHandlers->huart2 != NULL)
    {
        plt_DebugSendMSG((uint8_t*)ptr,(uint16_t)len);  // Send the data via UART
    }

    return len;
}

/**
 * @brief Processes received UART messages from the queue.
 * @note This function should be called periodically in the main loop. It dequeues messages and invokes the registered callback for processing.
 */
void plt_UartProcessRxMsgs(void)
{
    uart_message_t data = {0};
    uint16_t iterations = 0;
    const uint16_t MAX_ITERATIONS = PLT_MAX_QUEUE_SIZE + 1;  // Safety limit
    
    while (uartRxQueue.status != QUEUE_EMPTY && iterations < MAX_ITERATIONS)
    {
        Queue_Pop(&uartRxQueue, &data);  // Pop the data from the queue
        if (Uart_RxCallback)  // Check if the callback function is set
        {
            Uart_RxCallback(&data);  // Call the RX processing callback
        }
        iterations++;
    }
}


void plt_UartSyncMCUs(void)
{
    uart_message_t data = {0};
    HAL_StatusTypeDef status = HAL_OK;
    uint16_t iterations = 0;
    const uint16_t MAX_ITERATIONS = PLT_MAX_QUEUE_SIZE + 1;  // Safety limit
    
    while (uartTxQueue.status != QUEUE_EMPTY && iterations < MAX_ITERATIONS)
    {
        if (status == HAL_OK)
        {
            Queue_Pop(&uartTxQueue, &data);  // Pop the data from the queue
        }
        status = plt_UartSendMsg(UART_Between_MCUs, &data);  // Send the data via UART1
        iterations++;
    }
}

/**
 * @brief Sends a standard UART message through the UART DMA.
 * @param pData Pointer to the data buffer to be sent
 * @retval None
 * 
 * @note This function pushes the data for transmission and starts the DMA transfer.
*/ 
HAL_StatusTypeDef plt_UartSendMsg(UartChanel_t chanel, uart_message_t* pData)
{  
    
    #ifdef HAL_UART_MODULE_ENABLED
    // Parameter validation
    if (pData == NULL) {
        return HAL_ERROR;
    }
    
    if (chanel != Uart1 && chanel != Uart3) {
        return HAL_ERROR;
    }
    
    HAL_StatusTypeDef status = HAL_BUSY;
    UART_HandleTypeDef* pUart = (chanel == Uart1) ? pUart1:pUart3;
    
    if (pUart == NULL) {
        return HAL_ERROR;
    }
    if(pUart->gState == HAL_UART_STATE_READY )
    {
    status = HAL_UART_Transmit_DMA(pUart,(uint8_t*)pData,(uint16_t)sizeof(uart_message_t));
    }
    return status;
    #endif

}
/**
 * @brief Sends a debug message through the USART2 DMA.
 * @param pData Pointer to the data buffer to be sent
 * @param len Length of the data to be sent
 * 
 */
void plt_DebugSendMSG(uint8_t* pData,uint16_t len)
{
    if(pUart2)
    {
        Debug_TxData.len = len;
        memcpy(Debug_TxData.data,pData,len);
        Queue_Push(&debugTxQueue,&Debug_TxData);  // Push the data into the queue  
    }
}

/**
 * @brief Gets the UART transmission queue pointer.
 * @retval Pointer to the UART transmission queue.
 */

 Queue_t* GetDebugTxQueue(void)
 {
        return &debugTxQueue;
 }
 
 /**
  * @brief UART RX complete callback function.
  * @param huart Pointer to the UART handle
  * @note This function is called when the UART reception is complete.
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
 {
    if (huart->Instance == USART1)
    {
        if (Queue_Push(&uartRxQueue,Uart_RxData[1]) != QUEUE_OK) {
            // Queue full - UART message lost
            // TODO: Add error counter or logging
        }
        memset(Uart_RxData[1],0,sizeof(uart_message_t));
        HAL_UART_Receive_DMA(huart,Uart_RxData[1],(uint16_t)sizeof(uart_message_t));
    }
    if (huart->Instance == USART3)
    {
        if (Queue_Push(&uartRxQueue,Uart_RxData[2]) != QUEUE_OK) {
            // Queue full - UART message lost
            // TODO: Add error counter or logging
        }
        memset(Uart_RxData[2],0,sizeof(uart_message_t));
        HAL_UART_Receive_DMA(huart,Uart_RxData[2],(uint16_t)sizeof(uart_message_t));
    }
   
 }


 /**
 * @brief  Returns a pointer to the CAN RX queue.
 * @retval Pointer to the CAN RX queue
 * @note   This function is used to get the pointer to the CAN RX queue.
*/
Queue_t* plt_GetUartRxQueue()
{
    return &uartRxQueue;
}
Queue_t* plt_GetUartTxQueue()
{
    return &uartTxQueue;
}
#endif