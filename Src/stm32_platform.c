/**
 * @file stm32_platform.c
 * @brief Implementation of consumer-grade API
 */

#include "stm32_platform.h"
#include "platform.h"
#include "can.h"
#include "uart.h"
#include "spi.h"
#include "adc.h"
#include "tim.h"
#include "platform_config.h"
#include <stdio.h>
#include <string.h>

/* ==================== Private State ==================== */

static plt_status_t lastError = PLT_OK;
static bool platform_initialized = false;

/* ==================== CAN Implementation ==================== */

static bool CAN_send_impl(uint16_t id, const uint8_t* data, uint8_t length) {
    if (data == NULL || length > 8) {
        lastError = PLT_INVALID_PARAM;
        return false;
    }
    
    can_message_t msg = {0};
    msg.id = id;
    memcpy(msg.data, data, length);
    
    HAL_StatusTypeDef status = plt_CanSendMsg(Can1, &msg);
    lastError = (status == HAL_OK) ? PLT_OK : PLT_HAL_ERROR;
    
    return (status == HAL_OK);
}

static bool CAN_sendMessage_impl(const CANMessage_t* msg) {
    if (msg == NULL) {
        lastError = PLT_NULL_POINTER;
        return false;
    }
    
    return CAN_send_impl(msg->id, msg->data, msg->length);
}

static void CAN_handleRxMessages_impl(void) {
    plt_CanProcessRxMsgs();
}

static uint16_t CAN_availableMessages_impl(void) {
    Queue_t* queue = plt_GetCanRxQueue();
    if (queue == NULL) return 0;
    
    // Calculate messages in queue
    if (queue->status == QUEUE_EMPTY) return 0;
    if (queue->status == QUEUE_FULL) return queue->capacity;
    
    if (queue->head >= queue->tail) {
        return queue->head - queue->tail;
    } else {
        return queue->capacity - (queue->tail - queue->head);
    }
}

static void CAN_route_impl(uint16_t id, void (*handler)(CANMessage_t*)) {
    // Wrap handler to convert can_message_t to CANMessage_t
    // Implementation connects to existing hash table system
    // TODO: Add wrapper layer in can.c
}

static void CAN_routeRange_impl(uint16_t idStart, uint16_t idEnd, void (*handler)(CANMessage_t*)) {
    for (uint16_t id = idStart; id <= idEnd; id++) {
        CAN_route_impl(id, handler);
    }
}

static void CAN_setFilter_impl(uint16_t id, uint16_t mask) {
    // Configure CAN filter
    // Implementation uses plt_CanFilterInit with custom parameters
}

static void CAN_setBaudrate_impl(uint32_t baudrate) {
    // Reconfigure CAN baudrate
    // Would require HAL reconfiguration
}

static bool CAN_isReady_impl(void) {
    handler_set_t* handlers = plt_GetHandlersPointer();
    if (handlers == NULL || handlers->hcan1 == NULL) return false;
    
    return (handlers->hcan1->State == HAL_CAN_STATE_READY ||
            handlers->hcan1->State == HAL_CAN_STATE_LISTENING);
}

static uint32_t CAN_getTxCount_impl(void) {
    // Return TX counter (would need to add to can.c)
    return 0;
}

static uint32_t CAN_getRxCount_impl(void) {
    // Return RX counter (would need to add to can.c)
    return 0;
}

static uint32_t CAN_getErrorCount_impl(void) {
    handler_set_t* handlers = plt_GetHandlersPointer();
    if (handlers == NULL || handlers->hcan1 == NULL) return 0;
    
    return handlers->hcan1->ErrorCode;
}

/* ==================== UART Implementation ==================== */

static void UART_print_impl(const char* str) {
    if (str == NULL) return;
    plt_DebugSendMSG((uint8_t*)str, strlen(str));
}

static void UART_println_impl(const char* str) {
    UART_print_impl(str);
    UART_print_impl("\r\n");
}

static void UART_printf_impl(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    UART_print_impl(buffer);
}

static bool UART_write_impl(const uint8_t* data, uint16_t length) {
    if (data == NULL || length == 0) {
        lastError = PLT_INVALID_PARAM;
        return false;
    }
    
    uart_message_t msg = {0};
    if (length > sizeof(msg.data)) {
        length = sizeof(msg.data);
    }
    
    memcpy(msg.data, data, length);
    HAL_StatusTypeDef status = plt_UartSendMsg(Uart1, &msg);
    lastError = (status == HAL_OK) ? PLT_OK : PLT_HAL_ERROR;
    
    return (status == HAL_OK);
}

static void UART_handleRxData_impl(void) {
    plt_UartProcessRxMsgs();
}

static uint16_t UART_availableBytes_impl(void) {
    // Would need to query UART RX queue
    return 0;
}

static uint8_t UART_read_impl(void) {
    // Would need to pop from UART RX queue
    return 0;
}

static uint16_t UART_readBytes_impl(uint8_t* buffer, uint16_t length) {
    // Would need to pop multiple bytes from UART RX queue
    return 0;
}

static void UART_setBaudrate_impl(uint32_t baudrate) {
    // Reconfigure UART baudrate
}

static void UART_setTimeout_impl(uint16_t ms) {
    // Set UART timeout
}

static bool UART_isReady_impl(void) {
    handler_set_t* handlers = plt_GetHandlersPointer();
    if (handlers == NULL || handlers->huart2 == NULL) return false;
    
    return (handlers->huart2->gState == HAL_UART_STATE_READY);
}

/* ==================== SPI Implementation ==================== */

static void SPI_transfer_impl(uint8_t* txData, uint8_t* rxData, uint16_t length) {
    if (txData == NULL || rxData == NULL || length == 0) return;
    
    // Use existing SPI functions
    spi_message_t msg = {0};
    memcpy(msg.data, txData, length < sizeof(msg.data) ? length : sizeof(msg.data));
    plt_SpiSendMsg(&msg);
}

static uint8_t SPI_transferByte_impl(uint8_t data) {
    uint8_t rx = 0;
    SPI_transfer_impl(&data, &rx, 1);
    return rx;
}

static void SPI_handleRxData_impl(void) {
    plt_SpiProcessRxMsgs();
}

static uint16_t SPI_availableBytes_impl(void) {
    return 0;
}

static void SPI_setClockSpeed_impl(uint32_t hz) {
    // Reconfigure SPI clock
}

static void SPI_setMode_impl(uint8_t mode) {
    // Set SPI mode
}

static void SPI_select_impl(GPIO_TypeDef* port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

static void SPI_deselect_impl(GPIO_TypeDef* port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

/* ==================== ADC Implementation ==================== */

static uint16_t ADC_readRaw_impl(uint8_t channel) {
    // Read from ADC averaging buffers
    if (channel < ADC1_NUM_SENSORS) {
        return ADC1_AVG_Samples[channel];
    }
    return 0;
}

static float ADC_readVoltage_impl(uint8_t channel) {
    const platform_config_t* config = plt_GetCurrentConfig();
    uint16_t raw = ADC_readRaw_impl(channel);
    
    float maxValue = (1 << config->adc.samples_per_sensor) - 1;
    return (raw / maxValue) * config->system.system_clock_hz;
}

static void ADC_handleConversions_impl(void) {
    // Process ADC conversions
    // Already handled by DMA callbacks
}

static void ADC_setResolution_impl(uint8_t bits) {
    // Set ADC resolution
}

static void ADC_setReference_impl(float voltage) {
    // Set reference voltage (for conversion calculations)
}

static void ADC_calibrate_impl(void) {
    handler_set_t* handlers = plt_GetHandlersPointer();
    if (handlers && handlers->hadc1) {
        HAL_ADCEx_Calibration_Start(handlers->hadc1);
    }
}

/* ==================== PWM Implementation ==================== */

static void PWM_start_impl(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) return;
    HAL_TIM_PWM_Start(htim, channel);
}

static void PWM_stop_impl(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) return;
    HAL_TIM_PWM_Stop(htim, channel);
}

static void PWM_setFrequency_impl(TIM_HandleTypeDef* htim, uint32_t hz) {
    if (htim == NULL || hz == 0) return;
    plt_StartPWM(Tim2, TIM_CHANNEL_1, hz, 50.0);  // Wrapper around existing function
}

static void PWM_setDutyCycle_impl(TIM_HandleTypeDef* htim, uint32_t channel, float percent) {
    if (htim == NULL) return;
    
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(htim);
    uint32_t pulse = (uint32_t)(((float)(period + 1) * percent) / 100.0f);
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
}

static void PWM_setPulseWidth_impl(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t us) {
    if (htim == NULL) return;
    __HAL_TIM_SET_COMPARE(htim, channel, us);
}

/* ==================== Platform Implementation ==================== */

// Static handler set to avoid dangling pointer (Issue #10)
static handler_set_t platform_handlers = {0};

static Platform_t* Platform_begin_impl(CAN_HandleTypeDef* hcan,
                                       UART_HandleTypeDef* huart,
                                       SPI_HandleTypeDef* hspi,
                                       ADC_HandleTypeDef* hadc,
                                       TIM_HandleTypeDef* htim) {
    // Set up handler set (persistent storage)
    platform_handlers.hcan1 = hcan;
    platform_handlers.huart2 = huart;
    platform_handlers.hspi1 = hspi;
    platform_handlers.hadc1 = hadc;
    platform_handlers.htim2 = htim;
    
    plt_SetHandlers(&platform_handlers);
    
    // Initialize enabled peripherals
    if (hcan != NULL) {
        plt_CanInit(PLT_CAN_RX_QUEUE_SIZE);
    }
    
    if (huart != NULL) {
        plt_UartInit(PLT_UART_TX_QUEUE_SIZE);
    }
    
    if (hspi != NULL) {
        plt_SpiInit(PLT_SPI_RX_QUEUE_SIZE);
    }
    
    if (hadc != NULL) {
        plt_AdcInit();
    }
    
    if (htim != NULL) {
        plt_TimInit();
    }
    
    platform_initialized = true;
    return &Platform;
}

static Platform_t* Platform_onCAN_impl(void (*callback)(CANMessage_t*)) {
    // Register default CAN callback
    // Would need wrapper to convert can_message_t to CANMessage_t
    return &Platform;
}

static Platform_t* Platform_onUART_impl(void (*callback)(UARTMessage_t*)) {
    // Register UART callback
    return &Platform;
}

static Platform_t* Platform_onSPI_impl(void (*callback)(SPIMessage_t*)) {
    // Register SPI callback
    return &Platform;
}

static const char* Platform_version_impl(void) {
    return plt_GetVersion();
}

static plt_status_t Platform_getLastError_impl(void) {
    return lastError;
}

static const char* Platform_getErrorString_impl(plt_status_t err) {
    return plt_GetStatusString(err);
}

static bool Platform_isHealthy_impl(void) {
    return platform_initialized && (lastError == PLT_OK || lastError == PLT_WARN);
}

/* ==================== Global Singleton Definitions ==================== */

CAN_t P_CAN = {
    .send = CAN_send_impl,
    .sendMessage = CAN_sendMessage_impl,
    .handleRxMessages = CAN_handleRxMessages_impl,
    .availableMessages = CAN_availableMessages_impl,
    .route = CAN_route_impl,
    .routeRange = CAN_routeRange_impl,
    .setFilter = CAN_setFilter_impl,
    .setBaudrate = CAN_setBaudrate_impl,
    .isReady = CAN_isReady_impl,
    .getTxCount = CAN_getTxCount_impl,
    .getRxCount = CAN_getRxCount_impl,
    .getErrorCount = CAN_getErrorCount_impl,
};

UART_t P_UART = {
    .print = UART_print_impl,
    .println = UART_println_impl,
    .printf = UART_printf_impl,
    .write = UART_write_impl,
    .handleRxData = UART_handleRxData_impl,
    .availableBytes = UART_availableBytes_impl,
    .read = UART_read_impl,
    .readBytes = UART_readBytes_impl,
    .setBaudrate = UART_setBaudrate_impl,
    .setTimeout = UART_setTimeout_impl,
    .isReady = UART_isReady_impl,
};

SPI_t P_SPI = {
    .transfer = SPI_transfer_impl,
    .transferByte = SPI_transferByte_impl,
    .handleRxData = SPI_handleRxData_impl,
    .availableBytes = SPI_availableBytes_impl,
    .setClockSpeed = SPI_setClockSpeed_impl,
    .setMode = SPI_setMode_impl,
    .select = SPI_select_impl,
    .deselect = SPI_deselect_impl,
};

ADC_t P_ADC = {
    .readRaw = ADC_readRaw_impl,
    .readVoltage = ADC_readVoltage_impl,
    .handleConversions = ADC_handleConversions_impl,
    .setResolution = ADC_setResolution_impl,
    .setReference = ADC_setReference_impl,
    .calibrate = ADC_calibrate_impl,
};

PWM_t P_PWM = {
    .start = PWM_start_impl,
    .stop = PWM_stop_impl,
    .setFrequency = PWM_setFrequency_impl,
    .setDutyCycle = PWM_setDutyCycle_impl,
    .setPulseWidth = PWM_setPulseWidth_impl
};

Platform_t Platform = {
    .begin = Platform_begin_impl,
    .onCAN = Platform_onCAN_impl,
    .onUART = Platform_onUART_impl,
    .onSPI = Platform_onSPI_impl,
    .version = Platform_version_impl,
    .getLastError = Platform_getLastError_impl,
    .getErrorString = Platform_getErrorString_impl,
    .isHealthy = Platform_isHealthy_impl
};
