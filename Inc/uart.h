#ifndef UART_H
#define UART_H
/* =============================== Includes ======================================= */
#include "platform.h"

#ifdef HAL_UART_MODULE_ENABLED

/*========================= Function Declarations =========================*/
void plt_UartInit(size_t tx_queue_size);
void plt_UartSendMsg(UartChanel_t chanel, uart_message_t* pData);
void plt_DebugSendMSG(uint8_t* pData,uint16_t len);
void plt_UartProcessRxMsgs(void);
Queue_t* GetDebugTxQueue(void);

#endif // HAL_UART_MODULE_ENABLED


#endif	// UART_H