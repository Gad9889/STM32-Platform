
#ifndef CALLBACKS_H
#define CALLBACKS_H

/**
 * @file callbacks.h
 * @brief Application callback stub header
 * 
 * @note This file is a compatibility stub. The old callback system was removed
 *       in Platform v2.0.0. User applications should directly implement callbacks
 *       in their own code using the new P_CAN/P_UART/P_SPI/P_ADC/P_PWM API.
 * 
 * @deprecated Legacy API - kept for backward compatibility during migration
 */

/* =============================== Includes ======================================= */
#include <stdint.h>
#include <stddef.h>

//TODO: check if you can move this two variables to database.h
extern uint8_t KL_Nodes[3];
extern uint8_t FSM_stage;

#endif // CALLBACKS_H