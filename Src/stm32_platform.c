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
    #ifdef HAL_CAN_MODULE_ENABLED
    CAN_HandleTypeDef*  hcan[PLT_MAX_CAN_INSTANCES];
    uint8_t             can_count;
    #endif
    #ifdef HAL_UART_MODULE_ENABLED
    UART_HandleTypeDef* huart[PLT_MAX_UART_INSTANCES];
    uint8_t             uart_count;
    #endif
    #ifdef HAL_SPI_MODULE_ENABLED
    SPI_HandleTypeDef*  hspi[PLT_MAX_SPI_INSTANCES];
    uint8_t             spi_count;
    #endif
    #ifdef HAL_ADC_MODULE_ENABLED
    ADC_HandleTypeDef*  hadc[PLT_MAX_ADC_INSTANCES];
    uint8_t             adc_count;
    #endif
    #ifdef HAL_TIM_MODULE_ENABLED
    TIM_HandleTypeDef*  htim[PLT_MAX_TIM_INSTANCES];
    uint8_t             tim_count;
    #endif
} hw_handles = {0};

// CAN state (per instance)
static struct {
    Queue_t rx_queue;
    bool routing_initialized;
    void (*default_handler)(CANMessage_t*);
    volatile uint32_t tx_count;
    volatile uint32_t rx_count;
    volatile uint32_t error_count;
} can_state[PLT_MAX_CAN_INSTANCES] = {0};

// UART state (per instance)
static struct {
    Queue_t rx_queue;
    Queue_t tx_queue;
    uint8_t rx_buffer[256];
    volatile uint16_t rx_index;
    uint16_t timeout_ms;
} uart_state[PLT_MAX_UART_INSTANCES] = {0};

#ifdef HAL_SPI_MODULE_ENABLED
// SPI state (per instance)
static struct {
    Queue_t rx_queue;
    volatile bool busy;
} spi_state[PLT_MAX_SPI_INSTANCES] = {0};
#endif

#ifdef HAL_ADC_MODULE_ENABLED
// ADC state (per instance)
static struct {
    uint16_t* dma_buffer;
    uint16_t buffer_size;
    float vref;
} adc_state[PLT_MAX_ADC_INSTANCES] = {0};
#endif

/* ==================== CAN Implementation ==================== */

static bool CAN_send_impl(uint8_t instance, uint16_t id, const uint8_t* data, uint8_t length) {
    if (instance >= hw_handles.can_count || hw_handles.hcan[instance] == NULL) {
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
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hw_handles.hcan[instance], &tx_header, 
                                                     (uint8_t*)data, &tx_mailbox);
    
    if (status == HAL_OK) {
        can_state[instance].tx_count++;
        lastError = PLT_OK;
        return true;
    } else {
        lastError = PLT_HAL_ERROR;
        can_state[instance].error_count++;
        return false;
    }
}

static bool CAN_sendMessage_impl(uint8_t instance, const CANMessage_t* msg) {
    if (msg == NULL) {
        lastError = PLT_NULL_POINTER;
        return false;
    }
    return CAN_send_impl(instance, msg->id, msg->data, msg->length);
}

static void CAN_handleRxMessages_impl(uint8_t instance) {
    if (instance >= hw_handles.can_count || hw_handles.hcan[instance] == NULL) return;
    
    CANMessage_t msg;
    
    // Process all messages in queue
    while (Queue_Pop(&can_state[instance].rx_queue, &msg) == PLT_OK) {
        // Try hashtable routing first
        Set_Function_t handler = hash_Lookup(msg.id);
        
        if (handler != NULL) {
            // Route to specific handler - pass message data buffer
            handler(msg.data);
        } else if (can_state[instance].default_handler != NULL) {
            // Route to default handler
            can_state[instance].default_handler(&msg);
        }
    }
}

static uint16_t CAN_availableMessages_impl(uint8_t instance) {
    if (instance >= hw_handles.can_count) return 0;
    return (uint16_t)Queue_Count(&can_state[instance].rx_queue);
}

static void CAN_route_impl(uint8_t instance, uint16_t id, void (*handler)(CANMessage_t*)) {
    if (instance >= hw_handles.can_count || !can_state[instance].routing_initialized || handler == NULL) {
        return;
    }
    
    // Create hash member for routing - note: handler signature mismatch
    // hashtable expects Set_Function_t which takes uint8_t*, but we have CANMessage_t*
    // This is a design limitation - for now cast the handler
    hash_member_t member;
    member.id = id;
    member.Set_Function = (Set_Function_t)handler;
    
    hash_InsertMember(&member);
}

static void CAN_routeRange_impl(uint8_t instance, uint16_t idStart, uint16_t idEnd, void (*handler)(CANMessage_t*)) {
    for (uint16_t id = idStart; id <= idEnd; id++) {
        CAN_route_impl(instance, id, handler);
    }
}

static void CAN_setFilter_impl(uint8_t instance, uint16_t id, uint16_t mask) {
    if (instance >= hw_handles.can_count || hw_handles.hcan[instance] == NULL) return;
    
    CAN_FilterTypeDef filter;
    filter.FilterIdHigh = id << 5;
    filter.FilterIdLow = 0;
    filter.FilterMaskIdHigh = mask << 5;
    filter.FilterMaskIdLow = 0;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation = ENABLE;
    
    // Determine filter bank based on CAN instance
    #if defined(CAN2)
    if (hw_handles.hcan[instance]->Instance == CAN1) {
        filter.FilterBank = instance;  // CAN1: banks 0-13
        filter.SlaveStartFilterBank = 14;  // CAN2 starts at bank 14
    } else if (hw_handles.hcan[instance]->Instance == CAN2) {
        filter.FilterBank = 14 + instance;  // CAN2: banks 14-27
    }
    #else
    // Single CAN controller
    filter.FilterBank = instance;
    #endif
    
    HAL_CAN_ConfigFilter(hw_handles.hcan[instance], &filter);
}

static void CAN_setBaudrate_impl(uint8_t instance, uint32_t baudrate) {
    (void)instance;
    (void)baudrate;
    // Would require HAL re-initialization
    lastError = PLT_NOT_SUPPORTED;
}

static bool CAN_isReady_impl(uint8_t instance) {
    if (instance >= hw_handles.can_count || hw_handles.hcan[instance] == NULL) return false;
    
    HAL_CAN_StateTypeDef state = HAL_CAN_GetState(hw_handles.hcan[instance]);
    return (state == HAL_CAN_STATE_READY || state == HAL_CAN_STATE_LISTENING);
}

static uint32_t CAN_getTxCount_impl(uint8_t instance) {
    if (instance >= hw_handles.can_count) return 0;
    return can_state[instance].tx_count;
}

static uint32_t CAN_getRxCount_impl(uint8_t instance) {
    if (instance >= hw_handles.can_count) return 0;
    return can_state[instance].rx_count;
}

static uint32_t CAN_getErrorCount_impl(uint8_t instance) {
    if (instance >= hw_handles.can_count || hw_handles.hcan[instance] == NULL) return 0;
    return hw_handles.hcan[instance]->ErrorCode + can_state[instance].error_count;
}

/* ==================== UART Implementation ==================== */

static void UART_print_impl(uint8_t instance, const char* str) {
    if (instance >= hw_handles.uart_count || hw_handles.huart[instance] == NULL || str == NULL) return;
    
    HAL_UART_Transmit(hw_handles.huart[instance], (uint8_t*)str, strlen(str), uart_state[instance].timeout_ms);
}

static void UART_println_impl(uint8_t instance, const char* str) {
    UART_print_impl(instance, str);
    UART_print_impl(instance, "\r\n");
}

static void UART_printf_impl(uint8_t instance, const char* fmt, ...) {
    if (instance >= hw_handles.uart_count || hw_handles.huart[instance] == NULL) return;
    
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    if (len > 0) {
        HAL_UART_Transmit(hw_handles.huart[instance], (uint8_t*)buffer, len, uart_state[instance].timeout_ms);
    }
}

static bool UART_write_impl(uint8_t instance, const uint8_t* data, uint16_t length) {
    if (instance >= hw_handles.uart_count || hw_handles.huart[instance] == NULL || data == NULL || length == 0) {
        lastError = PLT_INVALID_PARAM;
        return false;
    }
    
    HAL_StatusTypeDef status = HAL_UART_Transmit(hw_handles.huart[instance], (uint8_t*)data, 
                                                  length, uart_state[instance].timeout_ms);
    
    lastError = (status == HAL_OK) ? PLT_OK : PLT_HAL_ERROR;
    return (status == HAL_OK);
}

static void UART_handleRxData_impl(uint8_t instance) {
    (void)instance;
    // Process received data from queue
    // Implementation depends on user callback design
}

static uint16_t UART_availableBytes_impl(uint8_t instance) {
    if (instance >= hw_handles.uart_count) return 0;
    return (uint16_t)Queue_Count(&uart_state[instance].rx_queue);
}

static uint8_t UART_read_impl(uint8_t instance) {
    if (instance >= hw_handles.uart_count) return 0;
    uint8_t byte = 0;
    Queue_Pop(&uart_state[instance].rx_queue, &byte);
    return byte;
}

static uint16_t UART_readBytes_impl(uint8_t instance, uint8_t* buffer, uint16_t length) {
    if (instance >= hw_handles.uart_count || buffer == NULL || length == 0) return 0;
    
    uint16_t count = 0;
    while (count < length && Queue_Pop(&uart_state[instance].rx_queue, &buffer[count]) == PLT_OK) {
        count++;
    }
    return count;
}

static void UART_setBaudrate_impl(uint8_t instance, uint32_t baudrate) {
    if (instance >= hw_handles.uart_count || hw_handles.huart[instance] == NULL) return;
    
    hw_handles.huart[instance]->Init.BaudRate = baudrate;
    HAL_UART_Init(hw_handles.huart[instance]);
}

static void UART_setTimeout_impl(uint8_t instance, uint16_t ms) {
    if (instance >= hw_handles.uart_count) return;
    uart_state[instance].timeout_ms = ms;
}

static bool UART_isReady_impl(uint8_t instance) {
    if (instance >= hw_handles.uart_count || hw_handles.huart[instance] == NULL) return false;
    return (HAL_UART_GetState(hw_handles.huart[instance]) == HAL_UART_STATE_READY);
}

/* ==================== SPI Implementation ==================== */
#ifdef HAL_SPI_MODULE_ENABLED

static void SPI_transfer_impl(uint8_t instance, uint8_t* txData, uint8_t* rxData, uint16_t length) {
    if (instance >= hw_handles.spi_count || hw_handles.hspi[instance] == NULL || txData == NULL || rxData == NULL || length == 0) {
        return;
    }
    
    HAL_SPI_TransmitReceive(hw_handles.hspi[instance], txData, rxData, length, 1000);
}

static uint8_t SPI_transferByte_impl(uint8_t instance, uint8_t data) {
    uint8_t rx = 0;
    SPI_transfer_impl(instance, &data, &rx, 1);
    return rx;
}

static void SPI_handleRxData_impl(uint8_t instance) {
    (void)instance;
    // SPI is synchronous, no background handling needed
}

static uint16_t SPI_availableBytes_impl(uint8_t instance) {
    if (instance >= hw_handles.spi_count) return 0;
    return (uint16_t)Queue_Count(&spi_state[instance].rx_queue);
}

static void SPI_setClockSpeed_impl(uint8_t instance, uint32_t hz) {
    (void)instance;
    (void)hz;
    // Would require HAL re-initialization
    lastError = PLT_NOT_SUPPORTED;
}

static void SPI_setMode_impl(uint8_t instance, uint8_t mode) {
    (void)instance;
    (void)mode;
    // Would require HAL re-initialization
    lastError = PLT_NOT_SUPPORTED;
}

static void SPI_select_impl(GPIO_TypeDef* port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

static void SPI_deselect_impl(GPIO_TypeDef* port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

#else // !HAL_SPI_MODULE_ENABLED

// Stub implementations when SPI not enabled
static void SPI_transfer_impl(uint8_t instance, uint8_t* txData, uint8_t* rxData, uint16_t length) { (void)instance; (void)txData; (void)rxData; (void)length; lastError = PLT_NOT_SUPPORTED; }
static uint8_t SPI_transferByte_impl(uint8_t instance, uint8_t data) { (void)instance; (void)data; lastError = PLT_NOT_SUPPORTED; return 0; }
static void SPI_handleRxData_impl(uint8_t instance) { (void)instance; }
static uint16_t SPI_availableBytes_impl(uint8_t instance) { (void)instance; return 0; }
static void SPI_setClockSpeed_impl(uint8_t instance, uint32_t hz) { (void)instance; (void)hz; lastError = PLT_NOT_SUPPORTED; }
static void SPI_setMode_impl(uint8_t instance, uint8_t mode) { (void)instance; (void)mode; lastError = PLT_NOT_SUPPORTED; }
static void SPI_select_impl(GPIO_TypeDef* port, uint16_t pin) { (void)port; (void)pin; }
static void SPI_deselect_impl(GPIO_TypeDef* port, uint16_t pin) { (void)port; (void)pin; }

#endif // HAL_SPI_MODULE_ENABLED

/* ==================== ADC Implementation ==================== */
#ifdef HAL_ADC_MODULE_ENABLED

static uint16_t ADC_readRaw_impl(uint8_t instance, uint8_t channel) {
    if (instance >= hw_handles.adc_count || hw_handles.hadc[instance] == NULL) return 0;
    
    // For DMA mode, read from buffer
    if (adc_state[instance].dma_buffer != NULL && channel < adc_state[instance].buffer_size) {
        return adc_state[instance].dma_buffer[channel];
    }
    
    // For polling mode, start conversion
    HAL_ADC_Start(hw_handles.hadc[instance]);
    HAL_ADC_PollForConversion(hw_handles.hadc[instance], 100);
    uint16_t value = HAL_ADC_GetValue(hw_handles.hadc[instance]);
    HAL_ADC_Stop(hw_handles.hadc[instance]);
    
    return value;
}

static float ADC_readVoltage_impl(uint8_t instance, uint8_t channel) {
    uint16_t raw = ADC_readRaw_impl(instance, channel);
    
    // Assume 12-bit ADC
    float max_value = 4095.0f;
    return (raw / max_value) * adc_state[instance].vref;
}

static void ADC_handleConversions_impl(uint8_t instance) {
    (void)instance;
    // DMA handles conversions automatically
}

static void ADC_setResolution_impl(uint8_t instance, uint8_t bits) {
    if (instance >= hw_handles.adc_count || hw_handles.hadc[instance] == NULL) return;
    
    uint32_t resolution;
    switch (bits) {
        // Different STM32 families use different constant naming
        #if defined(ADC_RESOLUTION_12B)
        case 12: resolution = ADC_RESOLUTION_12B; break;
        case 10: resolution = ADC_RESOLUTION_10B; break;
        case 8:  resolution = ADC_RESOLUTION_8B; break;
        case 6:  resolution = ADC_RESOLUTION_6B; break;
        #elif defined(ADC_RESOLUTION12b)
        case 12: resolution = ADC_RESOLUTION12b; break;
        case 10: resolution = ADC_RESOLUTION10b; break;
        case 8:  resolution = ADC_RESOLUTION8b; break;
        case 6:  resolution = ADC_RESOLUTION6b; break;
        #endif
        default: return;
    }
    
    hw_handles.hadc[instance]->Init.Resolution = resolution;
    HAL_ADC_Init(hw_handles.hadc[instance]);
}

static void ADC_setReference_impl(uint8_t instance, float voltage) {
    if (instance >= hw_handles.adc_count) return;
    adc_state[instance].vref = voltage;
}

static void ADC_calibrate_impl(uint8_t instance) {
    if (instance >= hw_handles.adc_count || hw_handles.hadc[instance] == NULL) return;
    
#ifdef HAL_ADCEx_Calibration_Start
    HAL_ADCEx_Calibration_Start(hw_handles.hadc[instance]);
#endif
}

#else // !HAL_ADC_MODULE_ENABLED

// Stub implementations when ADC not enabled
static uint16_t ADC_readRaw_impl(uint8_t instance, uint8_t channel) { (void)instance; (void)channel; lastError = PLT_NOT_SUPPORTED; return 0; }
static float ADC_readVoltage_impl(uint8_t instance, uint8_t channel) { (void)instance; (void)channel; lastError = PLT_NOT_SUPPORTED; return 0.0f; }
static void ADC_handleConversions_impl(uint8_t instance) { (void)instance; }
static void ADC_setResolution_impl(uint8_t instance, uint8_t bits) { (void)instance; (void)bits; lastError = PLT_NOT_SUPPORTED; }
static void ADC_setReference_impl(uint8_t instance, float voltage) { (void)instance; (void)voltage; }
static void ADC_calibrate_impl(uint8_t instance) { (void)instance; lastError = PLT_NOT_SUPPORTED; }

#endif // HAL_ADC_MODULE_ENABLED

/* ==================== PWM Implementation ==================== */
#ifdef HAL_TIM_MODULE_ENABLED

static void PWM_start_impl(uint8_t instance, uint32_t channel) {
    if (instance >= hw_handles.tim_count || hw_handles.htim[instance] == NULL) return;
    HAL_TIM_PWM_Start(hw_handles.htim[instance], channel);
}

static void PWM_stop_impl(uint8_t instance, uint32_t channel) {
    if (instance >= hw_handles.tim_count || hw_handles.htim[instance] == NULL) return;
    HAL_TIM_PWM_Stop(hw_handles.htim[instance], channel);
}

static void PWM_setFrequency_impl(uint8_t instance, uint32_t hz) {
    if (instance >= hw_handles.tim_count || hw_handles.htim[instance] == NULL || hz == 0) return;
    
    TIM_HandleTypeDef* htim = hw_handles.htim[instance];
    
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

static void PWM_setDutyCycle_impl(uint8_t instance, uint32_t channel, float percent) {
    if (instance >= hw_handles.tim_count || hw_handles.htim[instance] == NULL) return;
    
    TIM_HandleTypeDef* htim = hw_handles.htim[instance];
    
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(htim);
    uint32_t pulse = (uint32_t)(((float)(period + 1) * percent) / 100.0f);
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
}

static void PWM_setPulseWidth_impl(uint8_t instance, uint32_t channel, uint32_t us) {
    if (instance >= hw_handles.tim_count || hw_handles.htim[instance] == NULL) return;
    __HAL_TIM_SET_COMPARE(hw_handles.htim[instance], channel, us);
}

#else // !HAL_TIM_MODULE_ENABLED

// Stub implementations when TIM not enabled
static void PWM_start_impl(uint8_t instance, uint32_t channel) { (void)instance; (void)channel; lastError = PLT_NOT_SUPPORTED; }
static void PWM_stop_impl(uint8_t instance, uint32_t channel) { (void)instance; (void)channel; lastError = PLT_NOT_SUPPORTED; }
static void PWM_setFrequency_impl(uint8_t instance, uint32_t hz) { (void)instance; (void)hz; lastError = PLT_NOT_SUPPORTED; }
static void PWM_setDutyCycle_impl(uint8_t instance, uint32_t channel, float percent) { (void)instance; (void)channel; (void)percent; lastError = PLT_NOT_SUPPORTED; }
static void PWM_setPulseWidth_impl(uint8_t instance, uint32_t channel, uint32_t us) { (void)instance; (void)channel; (void)us; lastError = PLT_NOT_SUPPORTED; }

#endif // HAL_TIM_MODULE_ENABLED

/* ==================== Platform Implementation ==================== */

static Platform_t* Platform_begin_impl(PlatformHandles_t* handles) {
    if (handles == NULL) {
        lastError = PLT_NULL_POINTER;
        return &Platform;
    }
    
    lastError = PLT_OK;
    
    // Store hardware handles counts and arrays
    #ifdef HAL_CAN_MODULE_ENABLED
    hw_handles.can_count = (handles->can_count > PLT_MAX_CAN_INSTANCES) ? PLT_MAX_CAN_INSTANCES : handles->can_count;
    for (uint8_t i = 0; i < hw_handles.can_count; i++) {
        hw_handles.hcan[i] = (CAN_HandleTypeDef*)handles->hcan[i];
    }
    #endif
    
    #ifdef HAL_UART_MODULE_ENABLED
    hw_handles.uart_count = (handles->uart_count > PLT_MAX_UART_INSTANCES) ? PLT_MAX_UART_INSTANCES : handles->uart_count;
    for (uint8_t i = 0; i < hw_handles.uart_count; i++) {
        hw_handles.huart[i] = (UART_HandleTypeDef*)handles->huart[i];
    }
    #endif
    
    #ifdef HAL_SPI_MODULE_ENABLED
    hw_handles.spi_count = (handles->spi_count > PLT_MAX_SPI_INSTANCES) ? PLT_MAX_SPI_INSTANCES : handles->spi_count;
    for (uint8_t i = 0; i < hw_handles.spi_count; i++) {
        hw_handles.hspi[i] = (SPI_HandleTypeDef*)handles->hspi[i];
    }
    #endif
    
    #ifdef HAL_ADC_MODULE_ENABLED
    hw_handles.adc_count = (handles->adc_count > PLT_MAX_ADC_INSTANCES) ? PLT_MAX_ADC_INSTANCES : handles->adc_count;
    for (uint8_t i = 0; i < hw_handles.adc_count; i++) {
        hw_handles.hadc[i] = (ADC_HandleTypeDef*)handles->hadc[i];
    }
    #endif
    
    #ifdef HAL_TIM_MODULE_ENABLED
    hw_handles.tim_count = (handles->tim_count > PLT_MAX_TIM_INSTANCES) ? PLT_MAX_TIM_INSTANCES : handles->tim_count;
    for (uint8_t i = 0; i < hw_handles.tim_count; i++) {
        hw_handles.htim[i] = (TIM_HandleTypeDef*)handles->htim[i];
    }
    #endif
    
    #ifdef HAL_CAN_MODULE_ENABLED
    // Initialize all CAN instances
    for (uint8_t i = 0; i < hw_handles.can_count; i++) {
        if (hw_handles.hcan[i] == NULL) continue;
        
        // Initialize RX queue
        if (Queue_Init(&can_state[i].rx_queue, sizeof(CANMessage_t), CAN_RX_QUEUE_SIZE) != PLT_OK) {
            lastError = PLT_NO_MEMORY;
            return &Platform;
        }
        
        // Initialize routing hashtable (shared across instances for now)
        if (i == 0 && hash_Init() != HASH_OK) {
            lastError = PLT_HAL_ERROR;
            return &Platform;
        }
        can_state[i].routing_initialized = true;
        
        // Configure CAN filter to accept all messages
        CAN_FilterTypeDef filter;
        filter.FilterIdHigh = 0;
        filter.FilterIdLow = 0;
        filter.FilterMaskIdHigh = 0;
        filter.FilterMaskIdLow = 0;
        filter.FilterFIFOAssignment = CAN_RX_FIFO0;
        filter.FilterMode = CAN_FILTERMODE_IDMASK;
        filter.FilterScale = CAN_FILTERSCALE_32BIT;
        filter.FilterActivation = ENABLE;
        
        // Determine filter bank based on CAN instance
        // CAN1 uses banks 0-13, CAN2 uses banks 14-27 (on dual CAN MCUs)
        #if defined(CAN2)
        if (hw_handles.hcan[i]->Instance == CAN1) {
            filter.FilterBank = i;  // CAN1: banks 0-13
            filter.SlaveStartFilterBank = 14;  // CAN2 starts at bank 14
        } else if (hw_handles.hcan[i]->Instance == CAN2) {
            filter.FilterBank = 14 + i;  // CAN2: banks 14-27
        }
        #else
        // Single CAN controller
        filter.FilterBank = i;
        #endif
        
        HAL_CAN_ConfigFilter(hw_handles.hcan[i], &filter);
        
        // Start CAN
        HAL_CAN_Start(hw_handles.hcan[i]);
        HAL_CAN_ActivateNotification(hw_handles.hcan[i], CAN_IT_RX_FIFO0_MSG_PENDING);
        
        can_state[i].tx_count = 0;
        can_state[i].rx_count = 0;
        can_state[i].error_count = 0;
    }
    #endif
    
    #ifdef HAL_UART_MODULE_ENABLED
    // Initialize all UART instances
    for (uint8_t i = 0; i < hw_handles.uart_count; i++) {
        if (hw_handles.huart[i] == NULL) continue;
        
        // Initialize queues
        Queue_Init(&uart_state[i].rx_queue, sizeof(uint8_t), UART_RX_QUEUE_SIZE);
        Queue_Init(&uart_state[i].tx_queue, sizeof(uint8_t), UART_TX_QUEUE_SIZE);
        
        uart_state[i].rx_index = 0;
        uart_state[i].timeout_ms = 1000;
        
        // Start UART RX in interrupt mode
        HAL_UART_Receive_IT(hw_handles.huart[i], &uart_state[i].rx_buffer[0], 1);
    }
    #endif
    
    #ifdef HAL_SPI_MODULE_ENABLED
    // Initialize all SPI instances
    for (uint8_t i = 0; i < hw_handles.spi_count; i++) {
        if (hw_handles.hspi[i] == NULL) continue;
        
        Queue_Init(&spi_state[i].rx_queue, sizeof(uint8_t), SPI_RX_QUEUE_SIZE);
        spi_state[i].busy = false;
    }
    #endif
    
    #ifdef HAL_ADC_MODULE_ENABLED
    // Initialize all ADC instances
    for (uint8_t i = 0; i < hw_handles.adc_count; i++) {
        if (hw_handles.hadc[i] == NULL) continue;
        
        adc_state[i].vref = 3.3f; // Default VREF
        adc_state[i].dma_buffer = NULL;
        adc_state[i].buffer_size = 0;
        
        // Calibrate ADC
        ADC_calibrate_impl(i);
    }
    #endif
    
    platform_initialized = true;
    return &Platform;
}

static Platform_t* Platform_onCAN_impl(void (*callback)(CANMessage_t*)) {
    #ifdef HAL_CAN_MODULE_ENABLED
    // Set default handler for all CAN instances
    for (uint8_t i = 0; i < hw_handles.can_count; i++) {
        can_state[i].default_handler = callback;
    }
    #else
    (void)callback;
    #endif
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
    return "2.1.0";  // Multi-instance support
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
    #ifdef HAL_CAN_MODULE_ENABLED
    // Find which instance triggered the callback
    uint8_t instance = 0xFF;
    for (uint8_t i = 0; i < hw_handles.can_count; i++) {
        if (hcan == hw_handles.hcan[i]) {
            instance = i;
            break;
        }
    }
    
    if (instance == 0xFF) return;  // Not our instance
    
    CAN_RxHeaderTypeDef rx_header;
    CANMessage_t msg;
    
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, msg.data) == HAL_OK) {
        msg.id = (uint16_t)rx_header.StdId;
        msg.length = rx_header.DLC;
        msg.timestamp = HAL_GetTick();
        
        // Push to queue (ISR-safe)
        if (Queue_Push(&can_state[instance].rx_queue, &msg) == PLT_OK) {
            can_state[instance].rx_count++;
        }
    }
    #else
    (void)hcan;
    #endif
}

/**
 * @brief UART RX complete callback - called when byte received
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    #ifdef HAL_UART_MODULE_ENABLED
    // Find which instance triggered the callback
    uint8_t instance = 0xFF;
    for (uint8_t i = 0; i < hw_handles.uart_count; i++) {
        if (huart == hw_handles.huart[i]) {
            instance = i;
            break;
        }
    }
    
    if (instance == 0xFF) return;  // Not our instance
    
    // Push byte to queue
    Queue_Push(&uart_state[instance].rx_queue, &uart_state[instance].rx_buffer[uart_state[instance].rx_index]);
    
    // Continue receiving
    HAL_UART_Receive_IT(huart, &uart_state[instance].rx_buffer[uart_state[instance].rx_index], 1);
    #else
    (void)huart;
    #endif
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
