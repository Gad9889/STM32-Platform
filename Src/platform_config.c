/**
 * @file platform_config.c
 * @brief Platform configuration implementation
 */

#include "platform_config.h"
#include <string.h>

/* ==================== Private Variables ==================== */

/** @brief Current platform configuration */
static platform_config_t current_config = {0};

/** @brief Configuration initialized flag */
static bool config_initialized = false;

/* ==================== Public Functions ==================== */

/**
 * @brief Get default platform configuration
 */
platform_config_t plt_GetDefaultConfig(void)
{
    platform_config_t config = {0};
    
    // CAN defaults
    config.can.rx_queue_size = PLT_CAN_RX_QUEUE_SIZE;
    config.can.baudrate = 500000;  // 500 kbit/s
    config.can.enable_auto_retransmit = true;
    config.can.tx_mailbox_priority = 0;
    
    // UART defaults
    config.uart.rx_queue_size = PLT_UART_RX_QUEUE_SIZE;
    config.uart.tx_queue_size = PLT_UART_TX_QUEUE_SIZE;
    config.uart.baudrate = 115200;
    config.uart.timeout_ms = 1000;
    
    // SPI defaults
    config.spi.rx_queue_size = PLT_SPI_RX_QUEUE_SIZE;
    config.spi.timeout_ms = 1000;
    
    // ADC defaults
    config.adc.samples_per_sensor = 10;
    config.adc.num_sensors = 3;
    config.adc.enable_averaging = true;
    
    // Timer defaults
    config.tim.default_frequency = 1000;  // 1 kHz
    config.tim.default_duty_cycle = 50.0f;  // 50%
    
    // System defaults
    config.system.enable_error_handler = true;
    config.system.enable_debug_output = true;
    config.system.system_clock_hz = 168000000;  // 168 MHz (typical F4)
    
    return config;
}

/**
 * @brief Validate platform configuration
 */
plt_status_t plt_ConfigValidate(const platform_config_t* config)
{
    if (config == NULL) {
        return PLT_NULL_POINTER;
    }
    
    // Validate CAN configuration
    if (config->can.rx_queue_size < PLT_QUEUE_SIZE_MIN || 
        config->can.rx_queue_size > PLT_QUEUE_SIZE_MAX) {
        return PLT_INVALID_PARAM;
    }
    
    if (config->can.baudrate == 0 || config->can.baudrate > 1000000) {
        return PLT_INVALID_PARAM;
    }
    
    if (config->can.tx_mailbox_priority > 3) {
        return PLT_INVALID_PARAM;
    }
    
    // Validate UART configuration
    if (config->uart.rx_queue_size < PLT_QUEUE_SIZE_MIN || 
        config->uart.rx_queue_size > PLT_QUEUE_SIZE_MAX) {
        return PLT_INVALID_PARAM;
    }
    
    if (config->uart.tx_queue_size < PLT_QUEUE_SIZE_MIN || 
        config->uart.tx_queue_size > PLT_QUEUE_SIZE_MAX) {
        return PLT_INVALID_PARAM;
    }
    
    if (config->uart.baudrate == 0 || config->uart.baudrate > 10000000) {
        return PLT_INVALID_PARAM;
    }
    
    if (config->uart.timeout_ms == 0 || config->uart.timeout_ms > 60000) {
        return PLT_INVALID_PARAM;
    }
    
    // Validate SPI configuration
    if (config->spi.rx_queue_size < PLT_QUEUE_SIZE_MIN || 
        config->spi.rx_queue_size > PLT_QUEUE_SIZE_MAX) {
        return PLT_INVALID_PARAM;
    }
    
    if (config->spi.timeout_ms == 0 || config->spi.timeout_ms > 60000) {
        return PLT_INVALID_PARAM;
    }
    
    // Validate ADC configuration
    if (config->adc.samples_per_sensor == 0 || config->adc.samples_per_sensor > 100) {
        return PLT_INVALID_PARAM;
    }
    
    if (config->adc.num_sensors == 0 || config->adc.num_sensors > 16) {
        return PLT_INVALID_PARAM;
    }
    
    // Validate Timer configuration
    if (config->tim.default_frequency < PLT_PWM_FREQ_MIN || 
        config->tim.default_frequency > PLT_PWM_FREQ_MAX) {
        return PLT_INVALID_PARAM;
    }
    
    if (config->tim.default_duty_cycle < PLT_PWM_DUTY_MIN || 
        config->tim.default_duty_cycle > PLT_PWM_DUTY_MAX) {
        return PLT_INVALID_PARAM;
    }
    
    // Validate System configuration
    if (config->system.system_clock_hz == 0 || config->system.system_clock_hz > 480000000) {
        return PLT_INVALID_PARAM;
    }
    
    return PLT_OK;
}

/**
 * @brief Initialize platform with custom configuration
 */
plt_status_t plt_ConfigInit(const platform_config_t* config)
{
    platform_config_t temp_config;
    
    // Use default config if NULL provided
    if (config == NULL) {
        temp_config = plt_GetDefaultConfig();
        config = &temp_config;
    }
    
    // Validate configuration
    plt_status_t status = plt_ConfigValidate(config);
    if (status != PLT_OK) {
        return status;
    }
    
    // Copy configuration
    memcpy(&current_config, config, sizeof(platform_config_t));
    config_initialized = true;
    
    return PLT_OK;
}

/**
 * @brief Get current platform configuration
 */
const platform_config_t* plt_GetCurrentConfig(void)
{
    if (!config_initialized) {
        // Initialize with defaults if not already done
        plt_ConfigInit(NULL);
    }
    
    return &current_config;
}

/**
 * @brief Update specific configuration parameter at runtime
 */
plt_status_t plt_ConfigUpdate(uint32_t param, uint32_t value)
{
    if (!config_initialized) {
        return PLT_NOT_INITIALIZED;
    }
    
    // Parameter update logic would go here
    // This is a simplified implementation
    // In practice, you'd have enum for param identifiers
    
    return PLT_NOT_SUPPORTED;
}

/**
 * @brief Get platform version string
 */
const char* plt_GetVersion(void)
{
    return PLATFORM_VERSION_STRING;
}

/**
 * @brief Get platform version components
 */
plt_status_t plt_GetVersionInfo(uint8_t* major, uint8_t* minor, uint8_t* patch)
{
    if (major == NULL || minor == NULL || patch == NULL) {
        return PLT_NULL_POINTER;
    }
    
    *major = PLATFORM_VERSION_MAJOR;
    *minor = PLATFORM_VERSION_MINOR;
    *patch = PLATFORM_VERSION_PATCH;
    
    return PLT_OK;
}
