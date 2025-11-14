#include "tim.h"

#ifdef HAL_TIM_MODULE_ENABLED
TIM_HandleTypeDef *pTim2;
TIM_HandleTypeDef *pTim3;
TIM_HandleTypeDef *pTim4;


static handler_set_t* pHandlers = NULL; // Pointer to the handler set form the platform layer
static plt_callbacks_t* pCallbacks = NULL; // Pointer to the callback function pointers from the platform layer

/**
 * @brief Initialize timer peripherals
 * 
 * @return plt_status_t PLT_OK on success, PLT_NULL_POINTER if handler/callback pointers are NULL
 */
plt_status_t plt_TimInit(void) 
{
    // NULL pointer checks
    pHandlers = plt_GetHandlersPointer();
    if (pHandlers == NULL) {
        return PLT_NULL_POINTER;
    }
    
    pCallbacks = plt_GetCallbacksPointer();
    if (pCallbacks == NULL) {
        return PLT_NULL_POINTER;
    }

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
    
    return PLT_OK;
}

/**
 * @brief Start PWM output on specified timer channel
 * 
 * Configures and starts PWM generation with the requested frequency and duty cycle.
 * Automatically calculates optimal prescaler and period values to achieve the
 * target frequency while maintaining good resolution (period > 100 counts).
 * 
 * @param[in] timer      Timer peripheral to use (Tim2, Tim3, or Tim4)
 * @param[in] Channel    Timer channel (TIM_CHANNEL_1, TIM_CHANNEL_2, etc.)
 * @param[in] frequency  PWM frequency in Hz (1 Hz - 1 MHz)
 * @param[in] dutyCycle  Duty cycle percentage (0.0 - 100.0)
 *                       - 0.0% = always LOW
 *                       - 50.0% = 50% duty
 *                       - 100.0% = always HIGH
 * 
 * @return HAL_StatusTypeDef
 * @retval HAL_OK     PWM started successfully
 * @retval HAL_ERROR  Invalid parameters
 * 
 * @note Timer Configuration Requirements:
 *       - Timer configured in PWM mode in CubeMX
 *       - Channel enabled for PWM output
 *       - GPIO configured as alternate function for timer
 * 
 * @note Prescaler Calculation:
 *       Automatically finds prescaler to give period between 100-65535 for
 *       best resolution. If no valid combination exists, uses minimum resolution.
 * 
 * @warning Parameter validation:
 *          - Frequency clamped to 1 Hz - 1 MHz
 *          - Duty cycle clamped to 0.0 - 100.0%
 *          - Invalid timer returns HAL_ERROR without action
 * 
 * @par Example:
 * @code
 * // 1 kHz PWM at 75% duty cycle on TIM2 CH1
 * plt_StartPWM(Tim2, TIM_CHANNEL_1, 1000, 75.0f);
 * @endcode
 * 
 * @see plt_StopPWM() to stop PWM generation
 */
void plt_StartPWM(TimModule_t timer, uint32_t Channel, uint32_t frequency, float dutyCycle)
{
    // Parameter validation
    if (timer != Tim2 && timer != Tim3 && timer != Tim4) {
        return;
    }
    
    if (frequency == 0 || frequency > 1000000) {
        return; // Invalid frequency range
    }
    
    TIM_HandleTypeDef *pTim = (timer == Tim2) ? pTim2 : (timer == Tim3) ? pTim3 : pTim4;
    
    if (pTim == NULL || pTim->Instance == NULL) {
        return;
    }

    /* Clamp duty cycle between 0 and 100% */
    if (dutyCycle < 0.0f)  dutyCycle = 0.0f;
    if (dutyCycle > 100.0f) dutyCycle = 100.0f;

    /* Determine timer input clock (APB1 timer clocks run at 2× PCLK1 if prescaler >1) */
    uint32_t timerClock = 2U * HAL_RCC_GetPCLK1Freq();

    /* Calculate prescaler and period to achieve desired frequency */
    /* Target: reasonable resolution (period > 100) and achievable frequency */
    uint32_t prescaler = 0U;
    uint32_t period = 0U;
    
    /* Try to find prescaler that gives period between 100 and 65535 */
    for (prescaler = 0; prescaler < 65536; prescaler++) {
        period = (timerClock / ((prescaler + 1) * frequency)) - 1U;
        if (period >= 100 && period <= 65535) {
            break;
        }
    }
    
    /* If no valid combination found, use minimum resolution */
    if (prescaler >= 65536) {
        prescaler = (timerClock / (frequency * 100)) - 1;
        period = 99;
    }

    __HAL_TIM_SET_PRESCALER(pTim, prescaler);
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
    // Parameter validation
    if (timer != Tim2 && timer != Tim3 && timer != Tim4) {
        return;
    }
    
    TIM_HandleTypeDef *pTim = (timer == Tim2) ? pTim2 : (timer == Tim3) ? pTim3 : pTim4;
    
    if (pTim == NULL) {
        return;
    }

    /* Stop PWM on the channel */
    return HAL_TIM_PWM_Stop(pTim, Channel);
}
#endif