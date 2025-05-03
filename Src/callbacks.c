#include "callbacks.h"

/* =============================== Global Variables =============================== */
static database_t* pMainDB = NULL;
uint8_t FSM_stage = Stage1;
uint8_t KL_Nodes[3] = {0};
plt_callbacks_t pcallbacks;


/* ========================== Function Definitions ============================ */

 /**
  * @brief Initialize the platform layer with the provided handlers and RxQueueSize
  * @param handlers Pointer to the handler set for the platform layer
  * @param RxQueueSize Size of the RX message queue
  * @note This function initializes the platform layer with the provided handlers and RxQueueSize
  */
 void PlatformInit(handler_set_t *handlers,size_t RxQueueSize)
 {
    // Initialize the platform layer with the provided handlers and RxQueueSize
    
    pMainDB = db_Init();
    plt_SetHandlers(handlers);
    SetCallbacks();
    plt_SetCallbacks(&pcallbacks);

    #ifdef HAL_CAN_MODULE_ENABLED
    plt_CanInit(RxQueueSize);
    #endif

    #ifdef HAL_UART_MODULE_ENABLED
    plt_UartInit(RxQueueSize);
    #endif

    #ifdef HAL_SPI_MODULE_ENABLED
    plt_SpiInit(RxQueueSize);
    #endif

    #ifdef HAL_ADC_MODULE_ENABLED
      plt_AdcInit();
    #endif

    #ifdef HAL_TIM_MODULE_ENABLED
      plt_TimInit();
    #endif
 }

/**
 * @brief Callback function for handling CAN messages from the CAN-RxQueue and store the data in the DB.
 * @param msg Pointer to the received CAN message
 * @note This function is called in the plt_CanProcessRxMsgs function
 * @link plt_CanProcessRxMsgs
 * 
 */
void CanRxCallback(can_message_t *msg) 
{
  switch (msg->id) // Replace with actual condition
  {
  case PEDAL_ID:
    setPedalParameters(msg->data);
    break;
  
  default:
    break;
  }
}

/**
 * @brief Callback function for handling SPI messages from the SPI-RxQueue and store the data in the DB.
 * @param msg Pointer to the received SPI message
 * @note This function is called in the plt_SpiProcessRxMsgs function
 * @link plt_SpiProcessRxMsgs
 * 
 */
void SpiRxCallback(spi_message_t *msg) {

}

/**
 * @brief Callback function for handling UART messages from the UART-RXQueue and store the data in the DB.
 * @param msg Pointer to the received UART message
 * @note This function is called in the plt_UartProcessRxMsgs function
 * @link plt_UartProcessRxMsgs
 * 
 */
void UartRxCallback(uart_message_t *msg) {

}

/**
 * @brief Set the callback functions for the platform layer
 * @note This function sets the callback functions for the platform layer
 * @link SetCallbacks
 * 
 */
void SetCallbacks() {
   
    
    #ifdef HAL_CAN_MODULE_ENABLED
    pcallbacks.CAN_RxCallback = CanRxCallback;
    pcallbacks.CAN_TxCallback = NULL;
    #endif
    #ifdef HAL_UART_MODULE_ENABLED
    pcallbacks.UART_RxCallback = UartRxCallback;
    #endif
    #ifdef HAL_SPI_MODULE_ENABLED
    pcallbacks.SPI_RxCallback = SpiRxCallback;
    pcallbacks.SPI_TxCallback = NULL;
    #endif
 }

