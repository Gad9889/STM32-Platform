#include "platform.h"

/* =============================== Global Variables =============================== */
static plt_callbacks_t callbacks = {0}; // Callback function pointers for the platform layer
static handler_set_t *plt_handlers; // Pointer to the handler set for the platform layer


/* ========================== Function Definitions ============================ */

/**
 * @brief Initialize the platform layer
 * @param callbacks Callback function pointers for the platform layer
 */
void plt_SetCallbacks(plt_callbacks_t *pcallbacks){

    
    #ifdef HAL_CAN_MODULE_ENABLED
    callbacks.CAN_RxCallback = pcallbacks->CAN_RxCallback;
    callbacks.CAN_TxCallback = pcallbacks->CAN_TxCallback;
    #endif

    #ifdef HAL_UART_MODULE_ENABLED
    callbacks.UART_RxCallback = pcallbacks->UART_RxCallback;
    #endif

    #ifdef HAL_SPI_MODULE_ENABLED
    callbacks.SPI_RxCallback = pcallbacks->SPI_RxCallback;
    callbacks.SPI_TxCallback = pcallbacks->SPI_TxCallback;
    #endif
    
}

/**
 * @brief Configure the platform layer
 * @param handlers Pointer to the handler set for the platform layer
**/
void plt_SetHandlers(handler_set_t * handlers){
    plt_handlers = handlers;
}

/**
 * @brief Get function for the callbacks pointer
 * @retval Pointer to the callbacks struct
 * @note This function is used to get the pointer to the callbacks struct
 */
plt_callbacks_t* plt_GetCallbacksPointer()
{
    return &callbacks;
}

/**
 * @brief Get function for the handlers pointer
 * @retval Pointer to the handlers struct
 * @note This function is used to get the pointer to the handlers struct
 */
handler_set_t* plt_GetHandlersPointer()
{
    return plt_handlers;
}