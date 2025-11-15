/**
 * @file stm32_platform.c
 * @brief Consumer-grade API implementation - Direct HAL integration
 * 
 * This implementation talks directly to STM32 HAL without the old plt_* layer.
 * Uses improved thread-safe queues, hashtable routing, and database integration.
 */

#include "stm32_platform.h"
#include "utils.h"
#include "hashtable.h"
#include "database.h"
// Note: callbacks.h is a legacy stub - not required for v2.0.0
#include <stdio.h>
#include <string.h>

/* ==================== Configuration ==================== */

#define CAN_RX_QUEUE_SIZE   32
#define UART_RX_QUEUE_SIZE  16
#define UART_TX_QUEUE_SIZE  16
#define SPI_RX_QUEUE_SIZE   8

/* ==================== Private State ==================== */

// Global state
static plt_status_t lastError = PLT_OK;
static bool platform_initialized = false;

// Hardware handles (set by Platform.begin())
static struct {
    CAN_HandleTypeDef*  hcan;
    UART_HandleTypeDef* huart;
    SPI_HandleTypeDef*  hspi;
    ADC_HandleTypeDef*  hadc;
    TIM_HandleTypeDef*  htim;
} hw_handles = {NULL, NULL, NULL, NULL, NULL};

// CAN state
static struct {
    Queue_t rx_queue;
    hash_table_t* routing_table;
    void (*default_handler)(CANMessage_t*);
    volatile uint32_t tx_count;
    volatile uint32_t rx_count;
    volatile uint32_t error_count;
} can_state = {0};

// UART state  
static struct {
    Queue_t rx_queue;
    Queue_t tx_queue;
    uint8_t rx_buffer[256];
    volatile uint16_t rx_index;
    uint16_t timeout_ms;
} uart_state = {0};

// SPI state
static struct {
    Queue_t rx_queue;
    volatile bool busy;
} spi_state = {0};

// ADC state
static struct {
    uint16_t* dma_buffer;
    uint16_t buffer_size;
    float vref;
} adc_state = {0};

/* ==================== CAN Implementation ==================== */

static bool CAN_send_impl(uint16_t id, const uint8_t* data, uint8_t length) {
    if (hw_handles.hcan == NULL) {
        lastError = PLT_NOT_INITIALIZED;
        return false;
    }
    
    if (data == NULL || length > 8) {
        lastError = PLT_INVALID_PARAM;
        return false;
    }
    
    // Prepare CAN message
    CAN_TxHeaderTypeDef tx_header;
    tx_header.StdId = id;
    tx_header.ExtId = 0;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = length;
    tx_header.TransmitGlobalTime = DISABLE;
    
    uint32_t tx_mailbox;
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hw_handles.hcan, &tx_header, 
                                                     (uint8_t*)data, &tx_mailbox);
    
    if (status == HAL_OK) {
        can_state.tx_count++;
        lastError = PLT_OK;
        return true;
    } else {
        lastError = PLT_HAL_ERROR;
        can_state.error_count++;
        return false;
    }
}

static bool CAN_sendMessage_impl(const CANMessage_t* msg) {
    if (msg == NULL) {
        lastError = PLT_NULL_POINTER;
        return false;
    }
    return CAN_send_impl(msg->id, msg->data, msg->length);
}

static void CAN_handleRxMessages_impl(void) {
    if (hw_handles.hcan == NULL) return;
    
    CANMessage_t msg;
    
    // Process all messages in queue
    while (Queue_Pop(&can_state.rx_queue, &msg) == PLT_OK) {
        // Try hashtable routing first
        hash_member_t* handler = hash_Search(can_state.routing_table, msg.id);
        
        if (handler != NULL && handler->handler != NULL) {
            // Route to specific handler
            handler->handler((can_message_t*)&msg);
        } else if (can_state.default_handler != NULL) {
            // Route to default handler
            can_state.default_handler(&msg);
        }
    }
}

static uint16_t CAN_availableMessages_impl(void) {
    return (uint16_t)Queue_Count(&can_state.rx_queue);
}

static void CAN_route_impl(uint16_t id, void (*handler)(CANMessage_t*)) {
    if (can_state.routing_table == NULL || handler == NULL) {
        return;
    }
    
    // Create hash member for routing
    hash_member_t member;
    member.id = id;
    member.handler = (void (*)(can_message_t*))handler;
    
    hash_InsertMember(can_state.routing_table, &member);
}

static void CAN_routeRange_impl(uint16_t idStart, uint16_t idEnd, void (*handler)(CANMessage_t*)) {
    for (uint16_t id = idStart; id <= idEnd; id++) {
        CAN_route_impl(id, handler);
    }
}

static void CAN_setFilter_impl(uint16_t id, uint16_t mask) {
    if (hw_handles.hcan == NULL) return;
    
    CAN_FilterTypeDef filter;
    filter.FilterIdHigh = id << 5;
    filter.FilterIdLow = 0;
    filter.FilterMaskIdHigh = mask << 5;
    filter.FilterMaskIdLow = 0;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterBank = 0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation = ENABLE;
    
    HAL_CAN_ConfigFilter(hw_handles.hcan, &filter);
}

static void CAN_setBaudrate_impl(uint32_t baudrate) {
    // Would require HAL re-initialization
    lastError = PLT_NOT_SUPPORTED;
}

static bool CAN_isReady_impl(void) {
    if (hw_handles.hcan == NULL) return false;
    
    HAL_CAN_StateTypeDef state = HAL_CAN_GetState(hw_handles.hcan);
    return (state == HAL_CAN_STATE_READY || state == HAL_CAN_STATE_LISTENING);
}

static uint32_t CAN_getTxCount_impl(void) {
    return can_state.tx_count;
}

static uint32_t CAN_getRxCount_impl(void) {
    return can_state.rx_count;
}

static uint32_t CAN_getErrorCount_impl(void) {
    if (hw_handles.hcan == NULL) return 0;
    return hw_handles.hcan->ErrorCode + can_state.error_count;
}

/* ==================== UART Implementation ==================== */

static void UART_print_impl(const char* str) {
    if (hw_handles.huart == NULL || str == NULL) return;
    
    HAL_UART_Transmit(hw_handles.huart, (uint8_t*)str, strlen(str), uart_state.timeout_ms);
}

static void UART_println_impl(const char* str) {
    UART_print_impl(str);
    UART_print_impl("\r\n");
}

static void UART_printf_impl(const char* fmt, ...) {
    if (hw_handles.huart == NULL) return;
    
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    if (len > 0) {
        HAL_UART_Transmit(hw_handles.huart, (uint8_t*)buffer, len, uart_state.timeout_ms);
    }
}

static bool UART_write_impl(const uint8_t* data, uint16_t length) {
    if (hw_handles.huart == NULL || data == NULL || length == 0) {
        lastError = PLT_INVALID_PARAM;
        return false;
    }
    
    HAL_StatusTypeDef status = HAL_UART_Transmit(hw_handles.huart, (uint8_t*)data, 
                                                  length, uart_state.timeout_ms);
    
    lastError = (status == HAL_OK) ? PLT_OK : PLT_HAL_ERROR;
    return (status == HAL_OK);
}

static void UART_handleRxData_impl(void) {
    // Process received data from queue
    // Implementation depends on user callback design
}

static uint16_t UART_availableBytes_impl(void) {
    return (uint16_t)Queue_Count(&uart_state.rx_queue);
}

static uint8_t UART_read_impl(void) {
    uint8_t byte = 0;
    Queue_Pop(&uart_state.rx_queue, &byte);
    return byte;
}

static uint16_t UART_readBytes_impl(uint8_t* buffer, uint16_t length) {
    if (buffer == NULL || length == 0) return 0;
    
    uint16_t count = 0;
    while (count < length && Queue_Pop(&uart_state.rx_queue, &buffer[count]) == PLT_OK) {
        count++;
    }
    return count;
}

static void UART_setBaudrate_impl(uint32_t baudrate) {
    if (hw_handles.huart == NULL) return;
    
    hw_handles.huart->Init.BaudRate = baudrate;
    HAL_UART_Init(hw_handles.huart);
}

static void UART_setTimeout_impl(uint16_t ms) {
    uart_state.timeout_ms = ms;
}

static bool UART_isReady_impl(void) {
    if (hw_handles.huart == NULL) return false;
    return (HAL_UART_GetState(hw_handles.huart) == HAL_UART_STATE_READY);
}

/* ==================== SPI Implementation ==================== */

static void SPI_transfer_impl(uint8_t* txData, uint8_t* rxData, uint16_t length) {
    if (hw_handles.hspi == NULL || txData == NULL || rxData == NULL || length == 0) {
        return;
    }
    
    HAL_SPI_TransmitReceive(hw_handles.hspi, txData, rxData, length, 1000);
}

static uint8_t SPI_transferByte_impl(uint8_t data) {
    uint8_t rx = 0;
    SPI_transfer_impl(&data, &rx, 1);
    return rx;
}

static void SPI_handleRxData_impl(void) {
    // SPI is synchronous, no background handling needed
}

static uint16_t SPI_availableBytes_impl(void) {
    return (uint16_t)Queue_Count(&spi_state.rx_queue);
}

static void SPI_setClockSpeed_impl(uint32_t hz) {
    // Would require HAL re-initialization
    lastError = PLT_NOT_SUPPORTED;
}

static void SPI_setMode_impl(uint8_t mode) {
    // Would require HAL re-initialization
    lastError = PLT_NOT_SUPPORTED;
}

static void SPI_select_impl(GPIO_TypeDef* port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

static void SPI_deselect_impl(GPIO_TypeDef* port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

/* ==================== ADC Implementation ==================== */

static uint16_t ADC_readRaw_impl(uint8_t channel) {
    if (hw_handles.hadc == NULL) return 0;
    
    // For DMA mode, read from buffer
    if (adc_state.dma_buffer != NULL && channel < adc_state.buffer_size) {
        return adc_state.dma_buffer[channel];
    }
    
    // For polling mode, start conversion
    HAL_ADC_Start(hw_handles.hadc);
    HAL_ADC_PollForConversion(hw_handles.hadc, 100);
    uint16_t value = HAL_ADC_GetValue(hw_handles.hadc);
    HAL_ADC_Stop(hw_handles.hadc);
    
    return value;
}

static float ADC_readVoltage_impl(uint8_t channel) {
    uint16_t raw = ADC_readRaw_impl(channel);
    
    // Assume 12-bit ADC
    float max_value = 4095.0f;
    return (raw / max_value) * adc_state.vref;
}

static void ADC_handleConversions_impl(void) {
    // DMA handles conversions automatically
}

static void ADC_setResolution_impl(uint8_t bits) {
    if (hw_handles.hadc == NULL) return;
    
    uint32_t resolution;
    switch (bits) {
        case 12: resolution = ADC_RESOLUTION_12B; break;
        case 10: resolution = ADC_RESOLUTION_10B; break;
        case 8:  resolution = ADC_RESOLUTION_8B; break;
        case 6:  resolution = ADC_RESOLUTION_6B; break;
        default: return;
    }
    
    hw_handles.hadc->Init.Resolution = resolution;
    HAL_ADC_Init(hw_handles.hadc);
}

static void ADC_setReference_impl(float voltage) {
    adc_state.vref = voltage;
}

static void ADC_calibrate_impl(void) {
    if (hw_handles.hadc == NULL) return;
    
#ifdef HAL_ADCEx_Calibration_Start
    HAL_ADCEx_Calibration_Start(hw_handles.hadc);
#endif
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
    
    // Calculate prescaler and period for desired frequency
    // This assumes timer clock = SystemCoreClock
    uint32_t timer_clock = SystemCoreClock;
    uint32_t prescaler = 1;
    uint32_t period = timer_clock / hz;
    
    // Adjust if period too large
    while (period > 65535 && prescaler < 65535) {
        prescaler++;
        period = timer_clock / (hz * prescaler);
    }
    
    htim->Instance->PSC = prescaler - 1;
    htim->Instance->ARR = period - 1;
    HAL_TIM_GenerateEvent(htim, TIM_EVENTSOURCE_UPDATE);
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

static Platform_t* Platform_begin_impl(PlatformHandles_t* handles) {
    if (handles == NULL) {
        lastError = PLT_NULL_POINTER;
        return &Platform;
    }
    
    lastError = PLT_OK;
    
    // Store hardware handles (cast void* back to proper types internally)
    hw_handles.hcan = (CAN_HandleTypeDef*)handles->hcan;
    hw_handles.huart = (UART_HandleTypeDef*)handles->huart;
    hw_handles.hspi = (SPI_HandleTypeDef*)handles->hspi;
    hw_handles.hadc = (ADC_HandleTypeDef*)handles->hadc;
    hw_handles.htim = (TIM_HandleTypeDef*)handles->htim;
    
    // Initialize CAN if enabled
    if (hcan != NULL) {
        // Initialize RX queue
        if (Queue_Init(&can_state.rx_queue, sizeof(CANMessage_t), CAN_RX_QUEUE_SIZE) != PLT_OK) {
            lastError = PLT_NO_MEMORY;
            return &Platform;
        }
        
        // Initialize routing hashtable
        can_state.routing_table = hash_Init();
        if (can_state.routing_table == NULL) {
            lastError = PLT_NO_MEMORY;
            return &Platform;
        }
        
        // Configure CAN filter to accept all messages
        CAN_FilterTypeDef filter;
        filter.FilterIdHigh = 0;
        filter.FilterIdLow = 0;
        filter.FilterMaskIdHigh = 0;
        filter.FilterMaskIdLow = 0;
        filter.FilterFIFOAssignment = CAN_RX_FIFO0;
        filter.FilterBank = 0;
        filter.FilterMode = CAN_FILTERMODE_IDMASK;
        filter.FilterScale = CAN_FILTERSCALE_32BIT;
        filter.FilterActivation = ENABLE;
        HAL_CAN_ConfigFilter(hcan, &filter);
        
        // Start CAN
        HAL_CAN_Start(hcan);
        HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
        
        can_state.tx_count = 0;
        can_state.rx_count = 0;
        can_state.error_count = 0;
    }
    
    // Initialize UART if enabled
    if (huart != NULL) {
        // Initialize queues
        Queue_Init(&uart_state.rx_queue, sizeof(uint8_t), UART_RX_QUEUE_SIZE);
        Queue_Init(&uart_state.tx_queue, sizeof(uint8_t), UART_TX_QUEUE_SIZE);
        
        uart_state.rx_index = 0;
        uart_state.timeout_ms = 1000;
        
        // Start UART RX in interrupt mode
        HAL_UART_Receive_IT(huart, &uart_state.rx_buffer[0], 1);
    }
    
    // Initialize SPI if enabled
    if (hspi != NULL) {
        Queue_Init(&spi_state.rx_queue, sizeof(uint8_t), SPI_RX_QUEUE_SIZE);
        spi_state.busy = false;
    }
    
    // Initialize ADC if enabled
    if (hadc != NULL) {
        adc_state.vref = 3.3f; // Default VREF
        adc_state.dma_buffer = NULL;
        adc_state.buffer_size = 0;
        
        // Calibrate ADC
        ADC_calibrate_impl();
    }
    
    platform_initialized = true;
    return &Platform;
}

static Platform_t* Platform_onCAN_impl(void (*callback)(CANMessage_t*)) {
    can_state.default_handler = callback;
    return &Platform;
}

static Platform_t* Platform_onUART_impl(void (*callback)(UARTMessage_t*)) {
    // UART callback not yet implemented
    return &Platform;
}

static Platform_t* Platform_onSPI_impl(void (*callback)(SPIMessage_t*)) {
    // SPI callback not yet implemented
    return &Platform;
}

static const char* Platform_version_impl(void) {
    return "2.0.0";
}

static plt_status_t Platform_getLastError_impl(void) {
    return lastError;
}

static const char* Platform_getErrorString_impl(plt_status_t err) {
    return plt_GetStatusString(err);
}

static bool Platform_isHealthy_impl(void) {
    return platform_initialized && (lastError == PLT_OK);
}

/* ==================== HAL Callbacks ==================== */

/**
 * @brief CAN RX FIFO0 callback - called by HAL when message received
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    if (hcan != hw_handles.hcan) return;
    
    CAN_RxHeaderTypeDef rx_header;
    CANMessage_t msg;
    
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, msg.data) == HAL_OK) {
        msg.id = (uint16_t)rx_header.StdId;
        msg.length = rx_header.DLC;
        msg.timestamp = HAL_GetTick();
        
        // Push to queue (ISR-safe)
        if (Queue_Push(&can_state.rx_queue, &msg) == PLT_OK) {
            can_state.rx_count++;
        }
    }
}

/**
 * @brief UART RX complete callback - called when byte received
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart != hw_handles.huart) return;
    
    // Push byte to queue
    Queue_Push(&uart_state.rx_queue, &uart_state.rx_buffer[uart_state.rx_index]);
    
    // Continue receiving
    HAL_UART_Receive_IT(huart, &uart_state.rx_buffer[uart_state.rx_index], 1);
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
