#include "can.h"
#ifdef HAL_CAN_MODULE_ENABLED
// CAN Driver: Implementation for CAN message handling and processing using DMA

/* =============================== Global Variables =============================== */
CAN_HandleTypeDef* pCan1;         
CAN_HandleTypeDef* pCan2;         
CAN_HandleTypeDef* pCan3;

static handler_set_t* pHandlers = NULL; // Pointer to the handler set form the platform layer
static plt_callbacks_t* pCallbacks = NULL; // Pointer to the callback function pointers from the platform layer
static char CAN_ErrorMsg[256];
CAN_RxHeaderTypeDef RxHeader[2];          // Array for received CAN message headers 
uint8_t Can_RxData[2][8];                 // Buffer for received CAN message data (2 FIFOs, 8 bytes each) 
uint32_t TxMailbox[3];                    // Array for managing CAN transmission mailboxes

// Callback function for processing received CAN messages
void (*Can_RxCallback)(can_message_t *) = NULL;

// Queue for managing CAN received messages in main context
static Queue_t canRxQueue = {0};         // Queue for CAN received messages

static QueueItem_t canRxMessage = {      // Queue item for CAN received messages
    .data = NULL,
    .sizeof_data = sizeof(can_message_t)
};

/* ========================== Function Definitions ============================ */

/**
  * @brief  Initializes the CAN peripherals and enables RX interrupts.
  * @param  handlers     Pointer to the handler set for the platform layer
  * @param  callback     Function pointer to the RX processing callback
  * @param  rx_queue_size Size of the RX message queue
  * @note   This function initializes the CAN peripherals and enables RX interrupts.
*/
void plt_CanInit(size_t rx_queue_size)
{
    pHandlers = plt_GetHandlersPointer();
    pCallbacks = plt_GetCallbacksPointer();
    Can_RxCallback = pCallbacks->CAN_RxCallback;//pCallbacks->CAN_RxCallback; // Register the RX processing callback
    
    // Initialize the CAN1 peripheral
    if (pHandlers->hcan1 != NULL)
    {
        pCan1 = pHandlers->hcan1;
        plt_CanFilterInit(pCan1);
        HAL_CAN_Start(pCan1);
        if(HAL_CAN_ActivateNotification(pCan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
        {
            Error_Handler();
        }
    }

    // Initialize the CAN2 peripheral
    if (pHandlers->hcan2 != NULL)
    {
        pCan2 = pHandlers->hcan2;
        plt_CanFilterInit(pCan2);
        HAL_CAN_Start(pCan2);
        if(HAL_CAN_ActivateNotification(pCan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
        {
            Error_Handler();
        }
    }

    // Initialize the CAN3 peripheral
    if (pHandlers->hcan3 != NULL)
    {
        pCan3 = pHandlers->hcan3;
        plt_CanFilterInit(pCan3);
        HAL_CAN_Start(pCan3);
        if(HAL_CAN_ActivateNotification(pCan3, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
        {
            Error_Handler();
        }
    }

    // Initialize the CAN received message queue
    Queue_Init(&canRxQueue, &canRxMessage, rx_queue_size);
}

/**
 * @brief  Initializes the CAN filter for the specified CAN peripheral.
 * @param  pCan     Pointer to the CAN handle
 * @retval None
 * @note   This function initializes the CAN filter for the specified CAN peripheral.
 *      !  The filter is configured to accept any messages ID.
 */
void plt_CanFilterInit(CAN_HandleTypeDef* pCan)
{
    // Initialize the filter structure of FIFO0
    CAN_FilterTypeDef filter0;
    filter0.FilterActivation = ENABLE;
    filter0.FilterFIFOAssignment = CAN_FILTER_FIFO0;

    // Use 32-bit scale and mask mode
    filter0.FilterScale = CAN_FILTERSCALE_32BIT;
    filter0.FilterMode = CAN_FILTERMODE_IDMASK;

    // Set filter IDs to 0 and mask to 0 to accept any message ID
    filter0.FilterIdHigh = 0x0000;
    filter0.FilterIdLow  = 0x0000;
    filter0.FilterMaskIdHigh = 0x0000;
    filter0.FilterMaskIdLow  = 0x0000;

    if (pCan->Instance == CAN1)
    {
        filter0.FilterBank = 0; // Use bank 0 for CAN1
    }
    else if (pCan->Instance == CAN2)
    {
        filter0.FilterBank = 14; // Use bank 14 for CAN2
    }


    if (HAL_CAN_ConfigFilter(pCan, &filter0)) {
        Error_Handler();
    }

    //Initialize the filter structure of FIFO1
    CAN_FilterTypeDef filter1;
    filter1.FilterActivation = ENABLE;
    filter1.FilterFIFOAssignment = CAN_FILTER_FIFO1;

    // Use 32-bit scale and mask mode
    filter1.FilterScale = CAN_FILTERSCALE_32BIT;
    filter1.FilterMode = CAN_FILTERMODE_IDMASK;

    // Set filter IDs to 0 and mask to 0 to accept any message ID
    filter1.FilterIdHigh = 0x0000;
    filter1.FilterIdLow  = 0x0000;
    filter1.FilterMaskIdHigh = 0x0000;
    filter1.FilterMaskIdLow  = 0x0000;

     if (pCan->Instance == CAN1)
    {
        filter1.FilterBank = 13; // Use bank 0 for CAN1
    }
    else if (pCan->Instance == CAN2)
    {
        filter1.FilterBank = 27; // Use bank 14 for CAN2
    }

    if (HAL_CAN_ConfigFilter(pCan, &filter1)) {
        Error_Handler();
    }

}



/**
  * @brief  Processes received CAN messages from the queue.
  * @note   This function should be called periodically in the main loop. It
  *         dequeues messages and invokes the registered callback for processing.
*/
void plt_CanProcessRxMsgs()
{
    can_message_t data = {0};

    while (canRxQueue.status != QUEUE_EMPTY)
    {
        Queue_Pop(&canRxQueue, &data);
        if (Can_RxCallback)
        {
            Can_RxCallback(&data);
        }
    }
}

/**
 * @brief  General purpose function for sending a CAN message.
 * @param  chanel   CAN channel to send the message on
 * @param  pData    Pointer to the data buffer to be sent
 * @retval HAL status
*/
HAL_StatusTypeDef plt_CanTx(CanChanel_t chanel, CAN_TxHeaderTypeDef* TxHeader, uint8_t* pData)
{
    CAN_HandleTypeDef* pCan = (chanel == Can1) ? pCan1 : (chanel == Can2) ? pCan2 : pCan3; // Select the CAN channel
    if(pCan == NULL) return HAL_ERROR; // Return error if the CAN channel is not initialized
    
    
    if(HAL_CAN_GetTxMailboxesFreeLevel(pCan) > 0)
      {
        return HAL_CAN_AddTxMessage(
            pCan,
            TxHeader, 
            pData, 
            &TxMailbox[chanel - 1]
        );
      }
    
}

/**
 * @brief  Sends a CAN message through the specified CAN channel.
 * @param  chanel   CAN channel to send the message on
 * @param  pData    Pointer to the CAN message to be sent
 * @retval HAL status
*/
HAL_StatusTypeDef 
plt_CanSendMsg(CanChanel_t chanel, can_message_t *pData)
{
    CAN_TxHeaderTypeDef TxHeader = {0};
    TxHeader.StdId = pData->id;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 8;
    return plt_CanTx(chanel, &TxHeader, pData->data);
}

/*========================= Callbacks =========================*/

/**
  * @brief  Callback function for handling CAN messages received in interrupt context.
  * @param  hcan     Pointer to the CAN handle
  * @retval None
  * @note   This function is called when a CAN message is received in interrupt context.
  *         Copies the received message to the CAN message queue for processing in main context.
*/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    //Get the message from FIFO0 and Push it to the queue
    VALID(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader[0], Can_RxData[0]));   
    can_message_t msg;
    msg.id = RxHeader[0].StdId;
    memcpy(msg.data, Can_RxData[0], 8); 
    Queue_Push(&canRxQueue, &msg);
    return;
}

/**
  * @brief  Callback function for handling CAN messages received in interrupt context.
  * @param  hcan     Pointer to the CAN handle
  * @retval None
  * @note   This function is called when a CAN message is received in interrupt context.
  *         Copies the received message to the CAN message queue for processing in main context.
*/
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    //Get the message from FIFO1 and Push it to the queue
    VALID(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader[1], Can_RxData[1]));
    can_message_t msg;
    msg.id = RxHeader[1].StdId;
    memcpy(msg.data, Can_RxData[1], 8);
    Queue_Push(&canRxQueue, &msg);
    return;
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
 uint32_t errorCode = HAL_CAN_GetError(hcan);
    if (errorCode == HAL_CAN_ERROR_NONE)
        return;

    memset(CAN_ErrorMsg, 0, sizeof(CAN_ErrorMsg)); // Clear the error message buffer

    /* Protocol / bus errors ------------------------------------------------ */
    // Initialize the buffer to an empty string before building the message.
    CAN_ErrorMsg[0] = '\0';

    /* Protocol / bus errors ------------------------------------------------ */
    if (errorCode & HAL_CAN_ERROR_ACK) {
        // Use strcat to append messages
        strcat(CAN_ErrorMsg ,
               "CAN Error: ACK Error\r\n"
               " - No node on the bus acknowledged the frame\r\n"
               " - Remote filters rejected this identifier\r\n"
               " - Cable open or disconnected\r\n");
    }

    if (errorCode & HAL_CAN_ERROR_BOF) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: Bus-Off (BOF)\r\n"
               " - Persistent wiring/termination faults\r\n");
    }

    if (errorCode & HAL_CAN_ERROR_STF) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: Stuff Error\r\n"
               " - Six identical bits detected (bit-stuffing violation)\r\n"
               " - Clock mismatch or heavy noise\r\n"
               " - Bad transceiver\r\n");
    }

    if (errorCode & HAL_CAN_ERROR_FOR) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: Form Error\r\n"
               " - Timing disturbance on the line\r\n");
    }

    /* RX FIFO overruns ----------------------------------------------------- */
    if (errorCode & HAL_CAN_ERROR_RX_FOV0) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: RX FIFO0 Overrun\r\n"
               " - Application did not empty FIFO fast enough\r\n");
    }

    if (errorCode & HAL_CAN_ERROR_RX_FOV1) {
        strcat(CAN_ErrorMsg,
               "CAN Error: RX FIFO1 Overrun\r\n"
               " - Application did not empty FIFO fast enough\r\n");
    }

    /* Transmit mailbox problems -------------------------------------------- */
    if (errorCode & (HAL_CAN_ERROR_TX_ALST0 |
                     HAL_CAN_ERROR_TX_ALST1 |
                     HAL_CAN_ERROR_TX_ALST2)) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: Arbitration Lost (TX)\r\n"
               " - Another node won arbitration with lower ID\r\n");
    }

    if (errorCode & (HAL_CAN_ERROR_TX_TERR0 |
                     HAL_CAN_ERROR_TX_TERR1 |
                     HAL_CAN_ERROR_TX_TERR2)) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: Transmit Error (TXERR)\r\n"
               " - Error flag raised during transmission\r\n"
               " - Check wiring/termination/bit-timing\r\n");
    }

    /* HAL / driver-level errors ------------------------------------------- */
    if (errorCode & HAL_CAN_ERROR_TIMEOUT) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: Timeout\r\n"
               " - HAL call exceeded timeout value\r\n");
    }

    if (errorCode & HAL_CAN_ERROR_NOT_INITIALIZED) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: Not Initialized\r\n"
               " - HAL_CAN_Init not called or failed\r\n");
    }

    if (errorCode & HAL_CAN_ERROR_NOT_READY) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: Not Ready\r\n"
               " - Handle state is not HAL_CAN_STATE_READY\r\n");
    }

    if (errorCode & HAL_CAN_ERROR_NOT_STARTED) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: Not Started\r\n"
               " - HAL_CAN_Start missing or failed\r\n");
    }

    if (errorCode & HAL_CAN_ERROR_PARAM) {
        strcat(CAN_ErrorMsg ,
               "CAN Error: Parameter Error\r\n"
               " - Invalid argument supplied to HAL function\r\n");
    }


    HAL_CAN_ResetError(hcan); // Clear the error flags
}
/**
 * @brief  Returns a pointer to the CAN RX queue.
 * @retval Pointer to the CAN RX queue
 * @note   This function is used to get the pointer to the CAN RX queue.
*/
Queue_t* plt_GetCanRxQueue()
{
    return &canRxQueue;
}

#endif