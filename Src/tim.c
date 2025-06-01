#include "tim.h"

#ifdef HAL_TIM_MODULE_ENABLED
TIM_HandleTypeDef *pTim2;
TIM_HandleTypeDef *pTim3;
TIM_HandleTypeDef *pTim4;


static handler_set_t* pHandlers = NULL; // Pointer to the handler set form the platform layer
static plt_callbacks_t* pCallbacks = NULL; // Pointer to the callback function pointers from the platform layer

void plt_TimInit(void) 
{
    pHandlers = plt_GetHandlersPointer(); // Get the platform layer handlers pointer
    pCallbacks = plt_GetCallbacksPointer(); // Get the platform layer Callbacks pointer

    // Initialize the TIM2 peripheral
    if (pHandlers->htim2 != NULL) 
    {
        pTim2 = pHandlers->htim2;
    }

    // Initialize the TIM3 peripheral
    if (pHandlers->htim3 != NULL) 
    {
        pTim3 = pHandlers->htim3;

    }

    // Initialize the TIM4 peripheral
    if (pHandlers->htim4 != NULL) 
    {
        pTim4 = pHandlers->htim4;
    }
}

void plt_StartPWM(TimModule_t timer, uint32_t Channel, uint32_t frequency, float dutyCycle)
{
    TIM_HandleTypeDef *pTim = (timer == Tim2) ? pTim2 : (timer == Tim3) ? pTim3 : pTim4; 

    /* Clamp duty cycle between 0 and 100% */
    if (dutyCycle < 0.0f)  dutyCycle = 0.0f;
    if (dutyCycle > 100.0f) dutyCycle = 100.0f;

    /* Determine timer input clock (APB1 timer clocks run at 2× PCLK1 if prescaler >1) */
    uint32_t timerClock = 2U * HAL_RCC_GetPCLK1Freq();


    /* Set prescaler for a 1 MHz counter clock */

    uint32_t prescaler = 0U;
    __HAL_TIM_SET_PRESCALER(pTim, prescaler);

    /* Compute and set auto-reload for desired PWM frequency */
    uint32_t period = (timerClock / frequency) - 1U;
    __HAL_TIM_SET_AUTORELOAD(pTim, period);

    /* Compute and set capture-compare pulse for requested duty cycle */
    uint32_t pulse = (uint32_t)(((float)(period + 1U) * dutyCycle) / 100.0f);
    __HAL_TIM_SET_COMPARE(pTim, Channel, pulse);

    /* Force update generation to apply new prescaler and period immediately */
    pTim->Instance->EGR = TIM_EGR_UG;

    /* (Re)start PWM on the channel */
    return HAL_TIM_PWM_Start(pTim, Channel);
}

void plt_StopPWM(TimModule_t timer, uint32_t Channel)
{
    TIM_HandleTypeDef *pTim = (timer == Tim2) ? pTim2 : (timer == Tim3) ? pTim3 : pTim4; 

    /* Stop PWM on the channel */
    return HAL_TIM_PWM_Stop(pTim, Channel);
}
#endif