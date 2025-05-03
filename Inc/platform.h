
#ifndef PLATFORM_H
#define PLATFORM_H

/* =============================== Includes ======================================= */
#include "utils.h"
/* =============================== Global Structs =============================== */
/**
 * @brief CAN Chanels enum
 * @note This enum is used to define the CAN channels
 */
typedef enum{
	Can1 = 1,
	Can2 = 2,
    Can3 = 3
}CanChanel_t;

/**
 * @brief CAN message structure
 * @note This struct is used to store the CAN message data
 */
typedef struct{
    uint32_t id;
    uint8_t data[8];
} can_message_t;

/**
 * @brief UART Chanels enum
 * @note This enum is used to define the UART channels
 */
typedef enum{
	Uart1 = 1,
	Uart2 = 2,
	Uart3 = 3
}UartChanel_t;
/**
 * @brief UART message structure
 * @note This struct is used to store the UART message data
 */
typedef struct{
    uint32_t id;
    uint8_t data[8];

}uart_message_t;

/**
 * @brief Debug message structure
 * @note This struct is used to store the debug message data
 */
typedef struct {
    uint8_t data[64];
    uint16_t len;
} debug_message_t; 

/**
 * @brief SPI message structure
 * @note This struct is used to store the SPI message data
 */
typedef struct{
    uint32_t id;
    uint8_t data[8];
} spi_message_t; 

/**
 * @brief Timer Chanels enum
 * @note This enum is used to define the Timer channels
 */
typedef enum{
	Tim2 = 2,
	Tim3 = 3,
    Tim4 = 4
}TimModule_t;

typedef enum {
    Adc1 = 1,
    Adc2 = 2,
    Adc3 = 3
}Adc_Module_t;
/**
 * @brief Handlers for the platform layer
 * @note This struct is used to store the handler pointers for the platform layer
 */
typedef struct{
    #ifdef HAL_CAN_MODULE_ENABLED
    CAN_HandleTypeDef *hcan1;
    CAN_HandleTypeDef *hcan2;
    CAN_HandleTypeDef *hcan3;
    #endif
    #ifdef HAL_UART_MODULE_ENABLED
    UART_HandleTypeDef *huart1;
    UART_HandleTypeDef *huart2;
    UART_HandleTypeDef *huart3;
    #endif
    #ifdef HAL_SPI_MODULE_ENABLED
    SPI_HandleTypeDef *hspi1;
    SPI_HandleTypeDef *hspi2;
    SPI_HandleTypeDef *hspi3;
    #endif
    #ifdef HAL_ADC_MODULE_ENABLED
    ADC_HandleTypeDef *hadc1;
    ADC_HandleTypeDef *hadc2;
    ADC_HandleTypeDef *hadc3;
    #endif
    #ifdef HAL_TIM_MODULE_ENABLED
    TIM_HandleTypeDef *htim2;
    TIM_HandleTypeDef *htim3;
    TIM_HandleTypeDef *htim4;
    #endif
} handler_set_t;


/** 
 * @brief Callbacks for the platform layer
 * @note This struct is used to store the callback function pointers for the platform layer
*/

typedef struct{
    #ifdef HAL_CAN_MODULE_ENABLED
    void (*CAN_RxCallback)(can_message_t *msg);
    void (*CAN_TxCallback)(can_message_t *msg);
    
    #endif
    #ifdef HAL_UART_MODULE_ENABLED
    void (*UART_RxCallback)(uart_message_t *msg);
    #endif
    #ifdef HAL_SPI_MODULE_ENABLED
    void (*SPI_RxCallback)(spi_message_t *msg);
    void (*SPI_TxCallback)(spi_message_t *msg);
    #endif
} plt_callbacks_t;


/* ========================== Function Declarations ============================ */

void plt_SetHandlers(handler_set_t *handlers);
void plt_SetCallbacks(plt_callbacks_t *pcallbacks);
handler_set_t* plt_GetHandlersPointer();
plt_callbacks_t* plt_GetCallbacksPointer();

#endif // PLATFORM_H