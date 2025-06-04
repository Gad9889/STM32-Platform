#ifndef CAN_H
#define CAN_H
/* =============================== Includes ======================================= */
#include "platform.h"

#ifdef HAL_CAN_MODULE_ENABLED
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