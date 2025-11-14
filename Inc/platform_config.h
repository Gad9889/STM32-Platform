/**
 * @file platform_config.h
 * @brief Platform compile-time and runtime configuration
 * 
 * This file provides configuration options for buffer sizes, feature flags,
 * and runtime parameters for the STM32 Platform.
 */

#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "platform_status.h"

/* ==================== Compile-Time Configuration ==================== */

/**
 * @defgroup Platform_Version Platform Version Information
 * @{
 */
#define PLATFORM_VERSION_MAJOR    1
#define PLATFORM_VERSION_MINOR    0
#define PLATFORM_VERSION_PATCH    0
#define PLATFORM_VERSION_STRING   "1.0.0"
/** @} */

/**
 * @defgroup Queue_Sizes Default Queue Sizes
 * @brief Configure default queue buffer sizes for each peripheral
 * @{
 */
#ifndef PLT_CAN_RX_QUEUE_SIZE
#define PLT_CAN_RX_QUEUE_SIZE     64    /*!< CAN RX message queue size */
#endif

#ifndef PLT_UART_RX_QUEUE_SIZE
#define PLT_UART_RX_QUEUE_SIZE    32    /*!< UART RX message queue size */
#endif

#ifndef PLT_UART_TX_QUEUE_SIZE
#define PLT_UART_TX_QUEUE_SIZE    32    /*!< UART TX message queue size */
#endif

#ifndef PLT_SPI_RX_QUEUE_SIZE
#define PLT_SPI_RX_QUEUE_SIZE     16    /*!< SPI RX message queue size */
#endif

#ifndef PLT_DEBUG_TX_QUEUE_SIZE
#define PLT_DEBUG_TX_QUEUE_SIZE   64    /*!< Debug UART TX queue size */
#endif
/** @} */

/**
 * @defgroup Buffer_Limits Buffer Size Limits
 * @brief Maximum and minimum values for buffer sizes
 * @{
 */
#define PLT_QUEUE_SIZE_MIN        1     /*!< Minimum queue size */
#define PLT_QUEUE_SIZE_MAX        256   /*!< Maximum queue size */
#define PLT_ITEM_SIZE_MIN         1     /*!< Minimum item size (bytes) */
#define PLT_ITEM_SIZE_MAX         1024  /*!< Maximum item size (bytes) */
#define PLT_ADC_BUFFER_MAX        1024  /*!< Maximum ADC buffer size */
/** @} */

/**
 * @defgroup Feature_Flags Feature Enable/Disable
 * @brief Compile-time feature flags
 * @{
 */
#ifndef PLT_ENABLE_CAN
#define PLT_ENABLE_CAN            1     /*!< Enable CAN module */
#endif

#ifndef PLT_ENABLE_UART
#define PLT_ENABLE_UART           1     /*!< Enable UART module */
#endif

#ifndef PLT_ENABLE_SPI
#define PLT_ENABLE_SPI            1     /*!< Enable SPI module */
#endif

#ifndef PLT_ENABLE_ADC
#define PLT_ENABLE_ADC            1     /*!< Enable ADC module */
#endif

#ifndef PLT_ENABLE_TIM
#define PLT_ENABLE_TIM            1     /*!< Enable Timer/PWM module */
#endif

#ifndef PLT_ENABLE_DATABASE
#define PLT_ENABLE_DATABASE       1     /*!< Enable database system */
#endif

#ifndef PLT_ENABLE_HASHTABLE
#define PLT_ENABLE_HASHTABLE      1     /*!< Enable hash table routing */
#endif
/** @} */

/**
 * @defgroup Safety_Checks Safety Validation
 * @brief Enable/disable runtime safety checks
 * @{
 */
#ifndef PLT_ENABLE_PARAM_CHECK
#define PLT_ENABLE_PARAM_CHECK    1     /*!< Enable parameter validation */
#endif

#ifndef PLT_ENABLE_BOUNDS_CHECK
#define PLT_ENABLE_BOUNDS_CHECK   1     /*!< Enable bounds checking */
#endif

#ifndef PLT_ENABLE_NULL_CHECK
#define PLT_ENABLE_NULL_CHECK     1     /*!< Enable NULL pointer checks */
#endif
/** @} */

/**
 * @defgroup PWM_Limits PWM Configuration Limits
 * @brief Valid ranges for PWM parameters
 * @{
 */
#define PLT_PWM_FREQ_MIN          1         /*!< Minimum PWM frequency (Hz) */
#define PLT_PWM_FREQ_MAX          1000000   /*!< Maximum PWM frequency (Hz) */
#define PLT_PWM_DUTY_MIN          0.0f      /*!< Minimum duty cycle (%) */
#define PLT_PWM_DUTY_MAX          100.0f    /*!< Maximum duty cycle (%) */
/** @} */

/* ==================== Runtime Configuration ==================== */

/**
 * @brief Runtime platform configuration structure
 * 
 * This structure holds runtime-configurable parameters that can be
 * modified during platform initialization or operation.
 */
typedef struct {
    /** @brief CAN Configuration */
    struct {
        uint16_t rx_queue_size;         /*!< CAN RX queue size */
        uint32_t baudrate;              /*!< CAN baudrate (bps) */
        bool enable_auto_retransmit;    /*!< Enable automatic retransmission */
        uint8_t tx_mailbox_priority;    /*!< TX mailbox priority (0-3) */
    } can;
    
    /** @brief UART Configuration */
    struct {
        uint16_t rx_queue_size;         /*!< UART RX queue size */
        uint16_t tx_queue_size;         /*!< UART TX queue size */
        uint32_t baudrate;              /*!< UART baudrate (bps) */
        uint16_t timeout_ms;            /*!< TX/RX timeout (milliseconds) */
    } uart;
    
    /** @brief SPI Configuration */
    struct {
        uint16_t rx_queue_size;         /*!< SPI RX queue size */
        uint32_t timeout_ms;            /*!< TX/RX timeout (milliseconds) */
    } spi;
    
    /** @brief ADC Configuration */
    struct {
        uint16_t samples_per_sensor;    /*!< Number of samples to average */
        uint16_t num_sensors;           /*!< Number of ADC channels */
        bool enable_averaging;          /*!< Enable software averaging */
    } adc;
    
    /** @brief Timer/PWM Configuration */
    struct {
        uint32_t default_frequency;     /*!< Default PWM frequency (Hz) */
        float default_duty_cycle;       /*!< Default duty cycle (%) */
    } tim;
    
    /** @brief System Configuration */
    struct {
        bool enable_error_handler;      /*!< Enable Error_Handler() calls */
        bool enable_debug_output;       /*!< Enable debug printf */
        uint32_t system_clock_hz;       /*!< System clock frequency */
    } system;
    
} platform_config_t;

/* ==================== Function Prototypes ==================== */

/**
 * @brief Get default platform configuration
 * @return Default configuration structure
 * 
 * Returns a platform_config_t struct populated with default values.
 * User can modify and pass to plt_ConfigInit().
 */
platform_config_t plt_GetDefaultConfig(void);

/**
 * @brief Initialize platform with custom configuration
 * @param config Pointer to configuration structure
 * @return Platform status code
 * 
 * @note If config is NULL, default configuration is used
 */
plt_status_t plt_ConfigInit(const platform_config_t* config);

/**
 * @brief Validate platform configuration
 * @param config Pointer to configuration structure to validate
 * @return PLT_OK if valid, error code otherwise
 * 
 * Checks all configuration parameters against valid ranges.
 */
plt_status_t plt_ConfigValidate(const platform_config_t* config);

/**
 * @brief Get current platform configuration
 * @return Pointer to current configuration (read-only)
 */
const platform_config_t* plt_GetCurrentConfig(void);

/**
 * @brief Update specific configuration parameter at runtime
 * @param param Configuration parameter identifier
 * @param value New value for parameter
 * @return Platform status code
 * 
 * Allows updating individual parameters without full re-initialization.
 */
plt_status_t plt_ConfigUpdate(uint32_t param, uint32_t value);

/**
 * @brief Get platform version string
 * @return Version string (e.g., "1.0.0")
 */
const char* plt_GetVersion(void);

/**
 * @brief Get platform version components
 * @param major Pointer to store major version
 * @param minor Pointer to store minor version
 * @param patch Pointer to store patch version
 * @return Platform status code
 */
plt_status_t plt_GetVersionInfo(uint8_t* major, uint8_t* minor, uint8_t* patch);

/* ==================== Configuration Macros ==================== */

/**
 * @brief Check if feature is enabled
 * @param feature Feature name (without PLT_ENABLE_ prefix)
 * @return 1 if enabled, 0 if disabled
 */
#define PLT_IS_FEATURE_ENABLED(feature)  (PLT_ENABLE_##feature)

/**
 * @brief Validate queue size at compile time
 */
#define PLT_VALIDATE_QUEUE_SIZE(size) \
    ((size) >= PLT_QUEUE_SIZE_MIN && (size) <= PLT_QUEUE_SIZE_MAX)

/**
 * @brief Validate PWM frequency at compile time
 */
#define PLT_VALIDATE_PWM_FREQ(freq) \
    ((freq) >= PLT_PWM_FREQ_MIN && (freq) <= PLT_PWM_FREQ_MAX)

#endif /* PLATFORM_CONFIG_H */
