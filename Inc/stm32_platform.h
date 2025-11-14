/**
 * @file stm32_platform.h
 * @brief Consumer-grade API for STM32 Platform
 * 
 * Modern, explicit interface with singleton objects for each peripheral.
 * Users have full control over when and what to process.
 * 
 * @example
 * Platform.begin(&hcan1, &huart2, NULL)->onCAN(myHandler);
 * 
 * while(1) {
 *     P_CAN.handleRxMessages();
 *     P_UART.handleRxData();
 * }
 */

#ifndef STM32_PLATFORM_H
#define STM32_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "stm32f4xx_hal.h"  // Adjust for your MCU family
#include "platform_status.h"

/* ==================== Message Types ==================== */

/**
 * @brief CAN message structure
 */
typedef struct {
    uint16_t id;           /*!< CAN identifier (11-bit standard) */
    uint8_t data[8];       /*!< Message data (up to 8 bytes) */
    uint8_t length;        /*!< Actual data length (0-8) */
    uint32_t timestamp;    /*!< Reception timestamp (HAL_GetTick) */
} CANMessage_t;

/**
 * @brief UART message structure
 */
typedef struct {
    uint8_t data[256];     /*!< Message data */
    uint16_t length;       /*!< Actual data length */
    uint32_t timestamp;    /*!< Reception timestamp */
} UARTMessage_t;

/**
 * @brief SPI message structure
 */
typedef struct {
    uint8_t data[256];     /*!< Message data */
    uint16_t length;       /*!< Actual data length */
} SPIMessage_t;

/* ==================== Forward Declarations ==================== */

typedef struct Platform_t Platform_t;
typedef struct CAN_t CAN_t;
typedef struct UART_t UART_t;
typedef struct SPI_t SPI_t;
typedef struct ADC_t ADC_t;
typedef struct PWM_t PWM_t;

/* ==================== CAN Interface ==================== */

/**
 * @brief CAN peripheral interface
 */
struct CAN_t {
    /**
     * @brief Send CAN message
     * @param id CAN identifier (11-bit standard)
     * @param data Pointer to data buffer
     * @param length Data length (0-8 bytes)
     * @return true if message queued successfully
     */
    bool (*send)(uint16_t id, const uint8_t* data, uint8_t length);
    
    /**
     * @brief Send CAN message structure
     * @param msg Pointer to CAN message
     * @return true if message queued successfully
     */
    bool (*sendMessage)(const CANMessage_t* msg);
    
    /**
     * @brief Handle received CAN messages from queue
     * 
     * Dequeues all pending messages and routes to registered handlers.
     * Call this in your main loop.
     */
    void (*handleRxMessages)(void);
    
    /**
     * @brief Get number of messages waiting in queue
     * @return Number of unprocessed messages
     */
    uint16_t (*availableMessages)(void);
    
    /**
     * @brief Register handler for specific CAN ID
     * @param id CAN identifier to route
     * @param handler Callback function for this ID
     */
    void (*route)(uint16_t id, void (*handler)(CANMessage_t*));
    
    /**
     * @brief Register handler for range of CAN IDs
     * @param idStart Start of ID range (inclusive)
     * @param idEnd End of ID range (inclusive)
     * @param handler Callback function for this range
     */
    void (*routeRange)(uint16_t idStart, uint16_t idEnd, void (*handler)(CANMessage_t*));
    
    /**
     * @brief Configure CAN acceptance filter
     * @param id Filter ID
     * @param mask Filter mask
     */
    void (*setFilter)(uint16_t id, uint16_t mask);
    
    /**
     * @brief Set CAN baudrate
     * @param baudrate Baudrate in bps (e.g., 500000 for 500 kbit/s)
     */
    void (*setBaudrate)(uint32_t baudrate);
    
    /**
     * @brief Check if CAN peripheral is ready
     * @return true if ready for transmission
     */
    bool (*isReady)(void);
    
    /**
     * @brief Get total transmitted messages count
     * @return Number of messages sent since init
     */
    uint32_t (*getTxCount)(void);
    
    /**
     * @brief Get total received messages count
     * @return Number of messages received since init
     */
    uint32_t (*getRxCount)(void);
    
    /**
     * @brief Get CAN bus error count
     * @return Number of errors detected
     */
    uint32_t (*getErrorCount)(void);
};

/* ==================== UART Interface ==================== */

/**
 * @brief UART peripheral interface
 */
struct UART_t {
    /**
     * @brief Print string without newline
     * @param str Null-terminated string
     */
    void (*print)(const char* str);
    
    /**
     * @brief Print string with newline
     * @param str Null-terminated string
     */
    void (*println)(const char* str);
    
    /**
     * @brief Print formatted string (printf-style)
     * @param fmt Format string
     * @param ... Variable arguments
     */
    void (*printf)(const char* fmt, ...);
    
    /**
     * @brief Write raw bytes
     * @param data Pointer to data buffer
     * @param length Number of bytes to write
     * @return true if write successful
     */
    bool (*write)(const uint8_t* data, uint16_t length);
    
    /**
     * @brief Handle received UART data from queue
     * 
     * Dequeues all pending data and calls registered handler.
     * Call this in your main loop.
     */
    void (*handleRxData)(void);
    
    /**
     * @brief Get number of bytes waiting in RX queue
     * @return Number of unread bytes
     */
    uint16_t (*availableBytes)(void);
    
    /**
     * @brief Read single byte from RX buffer
     * @return Byte read (0 if buffer empty)
     */
    uint8_t (*read)(void);
    
    /**
     * @brief Read multiple bytes from RX buffer
     * @param buffer Pointer to destination buffer
     * @param length Maximum bytes to read
     * @return Number of bytes actually read
     */
    uint16_t (*readBytes)(uint8_t* buffer, uint16_t length);
    
    /**
     * @brief Set UART baudrate
     * @param baudrate Baudrate in bps (e.g., 115200)
     */
    void (*setBaudrate)(uint32_t baudrate);
    
    /**
     * @brief Set RX/TX timeout
     * @param ms Timeout in milliseconds
     */
    void (*setTimeout)(uint16_t ms);
    
    /**
     * @brief Check if UART peripheral is ready
     * @return true if ready for transmission
     */
    bool (*isReady)(void);
};

/* ==================== SPI Interface ==================== */

/**
 * @brief SPI peripheral interface
 */
struct SPI_t {
    /**
     * @brief Full-duplex SPI transfer
     * @param txData Pointer to transmit buffer
     * @param rxData Pointer to receive buffer
     * @param length Number of bytes to transfer
     */
    void (*transfer)(uint8_t* txData, uint8_t* rxData, uint16_t length);
    
    /**
     * @brief Transfer single byte
     * @param data Byte to transmit
     * @return Byte received
     */
    uint8_t (*transferByte)(uint8_t data);
    
    /**
     * @brief Handle received SPI data from queue
     * 
     * Dequeues all pending data and calls registered handler.
     * Call this in your main loop.
     */
    void (*handleRxData)(void);
    
    /**
     * @brief Get number of bytes waiting in RX queue
     * @return Number of unprocessed bytes
     */
    uint16_t (*availableBytes)(void);
    
    /**
     * @brief Set SPI clock speed
     * @param hz Clock frequency in Hz
     */
    void (*setClockSpeed)(uint32_t hz);
    
    /**
     * @brief Set SPI mode (0-3)
     * @param mode SPI mode (CPOL/CPHA combination)
     */
    void (*setMode)(uint8_t mode);
    
    /**
     * @brief Assert chip select (pull low)
     * @param port GPIO port
     * @param pin GPIO pin
     */
    void (*select)(GPIO_TypeDef* port, uint16_t pin);
    
    /**
     * @brief Deassert chip select (pull high)
     * @param port GPIO port
     * @param pin GPIO pin
     */
    void (*deselect)(GPIO_TypeDef* port, uint16_t pin);
};

/* ==================== ADC Interface ==================== */

/**
 * @brief ADC peripheral interface
 */
struct ADC_t {
    /**
     * @brief Read raw ADC value
     * @param channel ADC channel number
     * @return Raw ADC value (12-bit: 0-4095)
     */
    uint16_t (*readRaw)(uint8_t channel);
    
    /**
     * @brief Read ADC value converted to voltage
     * @param channel ADC channel number
     * @return Voltage in volts
     */
    float (*readVoltage)(uint8_t channel);
    
    /**
     * @brief Handle completed ADC conversions
     * 
     * Processes DMA conversion complete callbacks.
     * Call this in your main loop.
     */
    void (*handleConversions)(void);
    
    /**
     * @brief Set ADC resolution
     * @param bits Resolution in bits (8, 10, or 12)
     */
    void (*setResolution)(uint8_t bits);
    
    /**
     * @brief Set voltage reference
     * @param voltage Reference voltage in volts
     */
    void (*setReference)(float voltage);
    
    /**
     * @brief Calibrate ADC
     */
    void (*calibrate)(void);
};

/* ==================== PWM Interface ==================== */

/**
 * @brief PWM/Timer peripheral interface
 */
struct PWM_t {
    /**
     * @brief Start PWM output
     * @param htim Pointer to timer handle
     * @param channel Timer channel
     */
    void (*start)(TIM_HandleTypeDef* htim, uint32_t channel);
    
    /**
     * @brief Stop PWM output
     * @param htim Pointer to timer handle
     * @param channel Timer channel
     */
    void (*stop)(TIM_HandleTypeDef* htim, uint32_t channel);
    
    /**
     * @brief Set PWM frequency
     * @param htim Pointer to timer handle
     * @param hz Frequency in Hz
     */
    void (*setFrequency)(TIM_HandleTypeDef* htim, uint32_t hz);
    
    /**
     * @brief Set PWM duty cycle
     * @param htim Pointer to timer handle
     * @param channel Timer channel
     * @param percent Duty cycle (0.0 - 100.0)
     */
    void (*setDutyCycle)(TIM_HandleTypeDef* htim, uint32_t channel, float percent);
    
    /**
     * @brief Set PWM pulse width
     * @param htim Pointer to timer handle
     * @param channel Timer channel
     * @param us Pulse width in microseconds
     */
    void (*setPulseWidth)(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t us);
};

/* ==================== Platform Interface ==================== */

/**
 * @brief Platform initialization and utilities
 */
struct Platform_t {
    /**
     * @brief Initialize platform with peripheral handles
     * @param hcan CAN handle (NULL if not used)
     * @param huart UART handle (NULL if not used)
     * @param hspi SPI handle (NULL if not used)
     * @param hadc ADC handle (NULL if not used)
     * @param htim Timer handle (NULL if not used)
     * @return Pointer to Platform for method chaining
     */
    Platform_t* (*begin)(CAN_HandleTypeDef* hcan,
                         UART_HandleTypeDef* huart,
                         SPI_HandleTypeDef* hspi,
                         ADC_HandleTypeDef* hadc,
                         TIM_HandleTypeDef* htim);
    
    /**
     * @brief Register default CAN message handler
     * @param callback Function called for unrouted CAN messages
     * @return Pointer to Platform for method chaining
     */
    Platform_t* (*onCAN)(void (*callback)(CANMessage_t*));
    
    /**
     * @brief Register UART data handler
     * @param callback Function called for received UART data
     * @return Pointer to Platform for method chaining
     */
    Platform_t* (*onUART)(void (*callback)(UARTMessage_t*));
    
    /**
     * @brief Register SPI data handler
     * @param callback Function called for received SPI data
     * @return Pointer to Platform for method chaining
     */
    Platform_t* (*onSPI)(void (*callback)(SPIMessage_t*));
    
    /**
     * @brief Get platform version string
     * @return Version string (e.g., "1.0.0")
     */
    const char* (*version)(void);
    
    /**
     * @brief Get last error code
     * @return Error code from last operation
     */
    plt_status_t (*getLastError)(void);
    
    /**
     * @brief Get human-readable error string
     * @param err Error code
     * @return Error description string
     */
    const char* (*getErrorString)(plt_status_t err);
    
    /**
     * @brief Check system health status
     * @return true if all peripherals healthy
     */
    bool (*isHealthy)(void);
};

/* ==================== Global Singleton Objects ==================== */

extern CAN_t P_CAN;
extern UART_t P_UART;
extern SPI_t P_SPI;
extern ADC_t P_ADC;
extern PWM_t P_PWM;
extern Platform_t Platform;

#endif /* STM32_PLATFORM_H */
