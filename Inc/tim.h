#ifndef TIM_H
#define TIM_H
/* =============================== Includes ======================================= */
#include "platform.h"
#ifdef HAL_TIM_MODULE_ENABLED
/* ========================== Function Declarations ============================ */
void plt_TimInit(void);
HAL_StatusTypeDef plt_StartPWM(TimModule_t timer, uint32_t Channel, uint32_t frequency, float dutyCycle);
HAL_StatusTypeDef plt_StopPWM(TimModule_t timer, uint32_t Channel);

#endif
#endif // TIM_H