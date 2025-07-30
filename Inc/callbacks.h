
#ifndef CALLBACKS_H
#define CALLBACKS_H

/* =============================== Includes ======================================= */
#include "uart.h"
#include "spi.h"
#include "can.h"
#include "adc.h"
#include "tim.h"
//TODO: check if you can move this two verables to database.h
extern uint8_t KL_Nodes[3];
extern uint8_t FSM_stage;

/* ========================== Function Declarations ============================ */
void SpiRxCallback(spi_message_t *msg);
void CanRxCallback(can_message_t *msg);
void UartRxCallback(uart_message_t *msg);
void SetCallbacks();
void PlatformInit(handler_set_t *handlers,size_t RxQueueSize);

#endif // CALLBACKS_H