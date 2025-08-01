#include "callbacks.h"

/* =============================== Global Variables =============================== */
static database_t* pMainDB = NULL;
static Queue_t* pUartTxQueue = NULL;
plt_callbacks_t pcallbacks;
Set_Function_t pSet_Function;


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
    //pUartTxQueue = plt_GetUartTxQueue(); // Get the UART transmission queue pointer
    plt_SetHandlers(handlers);
    SetCallbacks();
    plt_SetCallbacks(&pcallbacks);
    //hash_Init(); // Initialize the hash table for storing set functions

    #ifdef HAL_CAN_MODULE_ENABLED
    plt_CanInit(RxQueueSize);
    printf("CAN Initialized \r\n");
    #endif

    #ifdef HAL_UART_MODULE_ENABLED
    plt_UartInit(RxQueueSize);
    pUartTxQueue = plt_GetUartTxQueue(); // Get the UART transmission queue pointer
    printf("UART Initialized \r\n");
    #endif

    #ifdef HAL_SPI_MODULE_ENABLED
    plt_SpiInit(RxQueueSize);
    printf("SPI Initialized \r\n");
    #endif

    #ifdef HAL_ADC_MODULE_ENABLED
    plt_AdcInit();
    printf("ADC Initialized \r\n");
    #endif

    #ifdef HAL_TIM_MODULE_ENABLED
    plt_TimInit();
    printf("Advanced TIM Initialized \r\n");
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
  //printf("Received CAN message with ID: %lu\r\n", msg->id);
 /* pSet_Function = hash_Lookup(msg->id);
  if(pSet_Function != NULL)
  {
    pSet_Function(msg->data); // Call the set function for the received message
  }
    */
  switch (msg->id) // Replace with actual condition
  {
    case PEDAL_ID:
      setPedalParameters(msg->data);
      break;
    case DB_ID:
      setDBParameters(msg->data);
      break;\
    case INV1_AV1_ID:
      msg->data[0] = 1; // Inverter number change before calling this function
      setInv1Av1Parameters(msg->data);
      break;
    case INV2_AV1_ID:
      msg->data[0] = 2; // Inverter number change before calling this function
      setInv2Av1Parameters(msg->data);
      break;
    case INV3_AV1_ID:
      msg->data[0] = 3; // Inverter number change before calling this function
      setInv3Av1Parameters(msg->data);
      break;
    case INV4_AV1_ID:
      msg->data[0] = 4; // Inverter number change before calling this function
      setInv4Av1Parameters(msg->data);
      break;
    case INV1_AV2_ID:
      msg->data[6] = 1; // Inverter number change before calling this function
      setInv1Av2Parameters(msg->data);
      break;
    case INV2_AV2_ID:
      msg->data[6] = 2; // Inverter number change before calling this function
      setInv2Av2Parameters(msg->data);
      break;
    case INV3_AV2_ID:
      msg->data[6] = 3; // Inverter number change before calling this function
      setInv3Av2Parameters(msg->data);
      break;
    case INV4_AV2_ID:
      msg->data[6] = 4; // Inverter number change before calling this function
      setInv4Av1Parameters(msg->data);
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

