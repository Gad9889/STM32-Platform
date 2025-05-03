
#include "spi.h"


// SPI Driver: Implementation for SPI communication and processing using DMA

/* =============================== Global Variables =============================== */
#ifdef HAL_SPI_MODULE_ENABLED
SPI_HandleTypeDef* pSpi;        //SPI handle pointer
spi_message_t dummy ; // Dummy message for SPI transmission
static handler_set_t* pHandlers = NULL; // Pointer to the handler set form the platform layer
static plt_callbacks_t* pCallbacks = NULL; // Pointer to the callback function pointers from the platform layer

uint8_t Spi_RxData[sizeof(spi_message_t)] = {0};  // DMA buffer for SPI reception
void (*Spi_RxCallback)(spi_message_t *msg) = NULL;  // Callback function for SPI reception

static Queue_t spiRxQueue = {0}; // Queue for SPI received messages
static QueueItem_t spiRxMessage = {
    .data = NULL,
    .sizeof_data = sizeof(spi_message_t)
};

/*========================= Function Definitions =========================*/

/**
  * @brief  Initializes the SPI module for reception and assigns a processing callback.
  * @param  huart      Pointer to the SPI handle to initialize
  * @param  callback   Function pointer to the RX processing callback
  * @retval None
  *
  * @note   This function enables SPI reception in interrupt mode.
  */
void plt_SpiInit(size_t rx_queue_size)
{
    pHandlers = plt_GetHandlersPointer();
    pCallbacks = plt_GetCallbacksPointer();
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
    HAL_SPI_TransmitReceive_DMA(pSpi,(uint8_t*)&dummy,Spi_RxData,(uint16_t)sizeof(spi_message_t)); // Start SPI transmission
   }

   if(pSpi->Init.Mode == SPI_MODE_SLAVE)
   {
    HAL_SPI_Receive_DMA(pSpi, Spi_RxData, sizeof(spi_message_t));  // Start SPI reception
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
    while (spiRxQueue.status != QUEUE_EMPTY)
    {
        Queue_Pop(&spiRxQueue, &data);
        if (Spi_RxCallback)
        {
            Spi_RxCallback(&data);
        }
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
     
     // Push the received data to the queue
     Queue_Push(&spiRxQueue, Spi_RxData);
     memset(Spi_RxData, 0, sizeof(Spi_RxData));
     // Start the next reception in interrupt mode
     HAL_SPI_Receive_DMA(pSpi, Spi_RxData, sizeof(spi_message_t));
     hspi->State = HAL_SPI_STATE_READY;
     
 }
 void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
 {
     // Push the received data to the queue
     Queue_Push(&spiRxQueue, Spi_RxData);
     memset(Spi_RxData, 0, sizeof(Spi_RxData));
     hspi->State = HAL_SPI_STATE_READY; 
 }
 #endif