#ifndef CAN_H
#define CAN_H
/* =============================== Includes ======================================= */
#include "platform.h"

#ifdef HAL_CAN_MODULE_ENABLED

/* ========================== Constants ============================ */
#define CAN_FILTER_BANK_CAN1_FIFO0  0   /*!< CAN1 FIFO0 filter bank */
#define CAN_FILTER_BANK_CAN1_FIFO1  13  /*!< CAN1 FIFO1 filter bank */
#define CAN_FILTER_BANK_CAN2_FIFO0  14  /*!< CAN2 FIFO0 filter bank */
#define CAN_FILTER_BANK_CAN2_FIFO1  27  /*!< CAN2 FIFO1 filter bank */
#define PLT_MAX_QUEUE_SIZE          256 /*!< Maximum queue size for safety */

/* ========================== Function Declarations ============================ */
void plt_CanInit(size_t rx_queue_size);
void plt_CanFilterInit(CAN_HandleTypeDef* pCan);
HAL_StatusTypeDef plt_CanSendMsg(CanChanel_t chanel, can_message_t* pData);
void plt_CanProcessRxMsgs();
Queue_t* plt_GetCanRxQueue();
/** @defgroup CAN_Error_Code CAN Error Code
  * @{
  */


#endif

#endif	// CAN_H