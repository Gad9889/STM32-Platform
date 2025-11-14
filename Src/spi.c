
#include "spi.h"


// SPI Driver: Implementation for SPI communication and processing using DMA

/* =============================== Global Variables =============================== */
#ifdef HAL_SPI_MODULE_ENABLED
SPI_HandleTypeDef* pSpi;        //SPI handle pointer
spi_message_t dummy ; // Dummy message for SPI transmission
static handler_set_t* pHandlers = NULL; // Pointer to the handler set form the platform layer
static plt_callbacks_t* pCallbacks = NULL; // Pointer to the callback function pointers from the platform layer

__attribute__((aligned(4))) uint8_t Spi_RxData[sizeof(spi_message_t)] = {0};  // DMA buffer for SPI reception - DMA aligned
void (*Spi_RxCallback)(spi_message_t *msg) = NULL;  // Callback function for SPI reception

static Queue_t spiRxQueue = {0}; // Queue for SPI received messages
static QueueItem_t spiRxMessage = {
    .data = NULL,
    .sizeof_data = sizeof(spi_message_t)
};

/*========================= Function Definitions =========================*/

/**
 * @brief Initialize SPI peripheral for DMA-based full-duplex communication
 * 
 * Configures SPI1, SPI2, or SPI3 for message exchange via DMA. Supports both
 * master and slave modes with automatic mode detection from HAL configuration.
 * Sets up circular RX buffer and message queue for ISR-to-main data transfer.
 * 
 * @param[in] rx_queue_size Size of the RX message queue (1-256)
 * 
 * @note SPI Configuration Requirements:
 *       - DMA enabled for both TX and RX (if master)
 *       - DMA RX only (if slave)
 *       - SPI configured in CubeMX with correct CPOL/CPHA for your device
 *       - NSS (chip select) managed by user code
 * 
 * @note Mode Detection:
 *       - Master: Uses TransmitReceive DMA (full duplex)
 *       - Slave: Uses Receive DMA only
 * 
 * @warning Calls Error_Handler() on:
 *          - NULL handler pointers
 *          - Invalid queue size (0 or >256)
 *          - DMA start failure
 * 
 * @see plt_SpiProcessRxMsgs() to process received messages
 * @see plt_SpiSendMsg() to transmit messages (non-blocking)
 */
void plt_SpiInit(size_t rx_queue_size)
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
    if (rx_queue_size == 0 || rx_queue_size > PLT_MAX_QUEUE_SIZE) {
        Error_Handler();
        return;
    }
    
    if (pHandlers->hspi1 != NULL)
    {
        pSpi = pHandlers->hspi1;
        pSpi->RxCpltCallback = HAL_SPI_RxCpltCallback;
    }
    if (pHandlers->hspi2 != NULL)
    {
        pSpi = pHandlers->hspi2;
        pSpi->RxCpltCallback = HAL_SPI_RxCpltCallback;
    }
    if (pHandlers->hspi3 != NULL)
    {
        pSpi = pHandlers->hspi3;
        pSpi->RxCpltCallback = HAL_SPI_RxCpltCallback;
    }

    Spi_RxCallback = pCallbacks->SPI_RxCallback;        // Register the RX processing callback
    Queue_Init(&spiRxQueue,&spiRxMessage,rx_queue_size);  // Initialize the RX queue
  
   if(pSpi->Init.Mode == SPI_MODE_MASTER)
   {
    if (HAL_SPI_TransmitReceive_DMA(pSpi,(uint8_t*)&dummy,Spi_RxData,(uint16_t)sizeof(spi_message_t)) != HAL_OK) {
        Error_Handler();
        return;
    }
   }

   if(pSpi->Init.Mode == SPI_MODE_SLAVE)
   {
    if (HAL_SPI_Receive_DMA(pSpi, Spi_RxData, sizeof(spi_message_t)) != HAL_OK) {
        Error_Handler();
        return;
    }
    }
}


/**
  * @brief  Processes received SPI messages from the queue.
  * @note   This function should be called periodically in the main loop. It
  *         dequeues messages and invokes the registered callback for processing.
  */
void plt_SpiProcessRxMsgs(void)
{
    spi_message_t data = {0};
    uint16_t iterations = 0;
    const uint16_t MAX_ITERATIONS = PLT_MAX_QUEUE_SIZE + 1;  // Safety limit
    
    while (spiRxQueue.status != QUEUE_EMPTY && iterations < MAX_ITERATIONS)
    {
        Queue_Pop(&spiRxQueue, &data);
        if (Spi_RxCallback)
        {
            Spi_RxCallback(&data);
        }
        iterations++;
    }

}
/**
 * @brief Sends a standard SPI message through the SPI DMA.
 * @param pData Pointer to the data buffer to be sent
 * @retval None
 * 
 * @note This function pushes the data for transmission and starts the DMA transfer.
*/ 
void plt_SpiSendMsg(spi_message_t* pData)
{  
    if (pData == NULL || pSpi == NULL) {
        return;
    }
    
    if(pSpi->State != HAL_SPI_STATE_READY) return;

    if(pSpi->Init.Mode == SPI_MODE_MASTER)
    {
        HAL_SPI_TransmitReceive_DMA(pSpi,(uint8_t *)pData,Spi_RxData,(uint16_t)sizeof(spi_message_t));
    }
 
    if(pSpi->Init.Mode == SPI_MODE_SLAVE)
    {
        HAL_SPI_Transmit_DMA(pSpi,(uint8_t *)pData, sizeof(spi_message_t));
     }
    return;
}



/* ============================ Interupt Callbacks ============================ */

/**
  * @brief  SPI receive complete callback triggered in interrupt context.
  * @param  hspi Pointer to the SPI handle
  * @retval None
  *
  * @note   This function is called when the SPI reception is complete. It pushes the  received data to the queue.
  */
 void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
 {   
     if (hspi == NULL || pSpi == NULL) {
         return;
     }
     
     // Push the received data to the queue
     if (Queue_Push(&spiRxQueue, Spi_RxData) != QUEUE_OK) {
         // Queue full - data lost
         // TODO: Add error logging or counter
     }
     memset(Spi_RxData, 0, sizeof(Spi_RxData));
     // Start the next reception in interrupt mode
     HAL_SPI_Receive_DMA(pSpi, Spi_RxData, sizeof(spi_message_t));
     
 }
 void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
 {
     // Push the received data to the queue
     if (Queue_Push(&spiRxQueue, Spi_RxData) != QUEUE_OK) {
         // Queue full - data lost
         // TODO: Add error logging or counter
     }
     memset(Spi_RxData, 0, sizeof(Spi_RxData));
 }
 #endif