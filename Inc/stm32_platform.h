/**
 * @file stm32_platform.h
 * @brief Consumer-grade API for STM32 Platform
 * 
 * Modern, explicit interface with singleton objects for each peripheral.
 * Users have full control over when and what to process.
 * 
 * AUTO-DETECTION: This header automatically detects your STM32 family
 * (F0/F1/F2/F3/F4/F7/H7/L0/L1/L4/L5/G0/G4/WB/WL) and includes the correct
 * HAL header. No manual configuration needed!
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

/* Auto-detect STM32 family and include appropriate HAL */
/* Detection based on actual device defines from CubeMX/STM32CubeMX */
#if defined(STM32F030x6) || defined(STM32F030x8) || defined(STM32F030xC) || \
    defined(STM32F031x6) || defined(STM32F038xx) || defined(STM32F042x6) || \
    defined(STM32F048xx) || defined(STM32F051x8) || defined(STM32F058xx) || \
    defined(STM32F070x6) || defined(STM32F070xB) || defined(STM32F071xB) || \
    defined(STM32F072xB) || defined(STM32F078xx) || defined(STM32F091xC) || \
    defined(STM32F098xx)
    #include "stm32f0xx_hal.h"
#elif defined(STM32F100xB) || defined(STM32F100xE) || defined(STM32F101x6) || \
    defined(STM32F101xB) || defined(STM32F101xE) || defined(STM32F101xG) || \
    defined(STM32F102x6) || defined(STM32F102xB) || defined(STM32F103x6) || \
    defined(STM32F103xB) || defined(STM32F103xE) || defined(STM32F103xG) || \
    defined(STM32F105xC) || defined(STM32F107xC)
    #include "stm32f1xx_hal.h"
#elif defined(STM32F205xx) || defined(STM32F215xx) || defined(STM32F207xx) || \
    defined(STM32F217xx)
    #include "stm32f2xx_hal.h"
#elif defined(STM32F301x8) || defined(STM32F302x8) || defined(STM32F302xC) || \
    defined(STM32F302xE) || defined(STM32F303x8) || defined(STM32F303xC) || \
    defined(STM32F303xE) || defined(STM32F318xx) || defined(STM32F328xx) || \
    defined(STM32F334x8) || defined(STM32F358xx) || defined(STM32F373xC) || \
    defined(STM32F378xx) || defined(STM32F398xx)
    #include "stm32f3xx_hal.h"
#elif defined(STM32F401xC) || defined(STM32F401xE) || defined(STM32F405xx) || \
    defined(STM32F407xx) || defined(STM32F410Cx) || defined(STM32F410Rx) || \
    defined(STM32F410Tx) || defined(STM32F411xE) || defined(STM32F412Cx) || \
    defined(STM32F412Rx) || defined(STM32F412Vx) || defined(STM32F412Zx) || \
    defined(STM32F413xx) || defined(STM32F415xx) || defined(STM32F417xx) || \
    defined(STM32F423xx) || defined(STM32F427xx) || defined(STM32F429xx) || \
    defined(STM32F437xx) || defined(STM32F439xx) || defined(STM32F446xx) || \
    defined(STM32F469xx) || defined(STM32F479xx)
    #include "stm32f4xx_hal.h"
#elif defined(STM32F722xx) || defined(STM32F723xx) || defined(STM32F730xx) || \
    defined(STM32F732xx) || defined(STM32F733xx) || defined(STM32F745xx) || \
    defined(STM32F746xx) || defined(STM32F750xx) || defined(STM32F756xx) || \
    defined(STM32F765xx) || defined(STM32F767xx) || defined(STM32F769xx) || \
    defined(STM32F777xx) || defined(STM32F779xx)
    #include "stm32f7xx_hal.h"
#elif defined(STM32H743xx) || defined(STM32H753xx) || defined(STM32H750xx) || \
    defined(STM32H742xx) || defined(STM32H745xx) || defined(STM32H755xx) || \
    defined(STM32H747xx) || defined(STM32H757xx) || defined(STM32H7A3xx) || \
    defined(STM32H7A3xxQ) || defined(STM32H7B3xx) || defined(STM32H7B3xxQ) || \
    defined(STM32H7B0xx) || defined(STM32H7B0xxQ)
    #include "stm32h7xx_hal.h"
#elif defined(STM32L010x4) || defined(STM32L010x6) || defined(STM32L010x8) || \
    defined(STM32L010xB) || defined(STM32L011xx) || defined(STM32L021xx) || \
    defined(STM32L031xx) || defined(STM32L041xx) || defined(STM32L051xx) || \
    defined(STM32L052xx) || defined(STM32L053xx) || defined(STM32L062xx) || \
    defined(STM32L063xx) || defined(STM32L071xx) || defined(STM32L072xx) || \
    defined(STM32L073xx) || defined(STM32L081xx) || defined(STM32L082xx) || \
    defined(STM32L083xx)
    #include "stm32l0xx_hal.h"
#elif defined(STM32L100xB) || defined(STM32L100xBA) || defined(STM32L100xC) || \
    defined(STM32L151xB) || defined(STM32L151xBA) || defined(STM32L151xC) || \
    defined(STM32L151xCA) || defined(STM32L151xD) || defined(STM32L151xDX) || \
    defined(STM32L151xE) || defined(STM32L152xB) || defined(STM32L152xBA) || \
    defined(STM32L152xC) || defined(STM32L152xCA) || defined(STM32L152xD) || \
    defined(STM32L152xDX) || defined(STM32L152xE) || defined(STM32L162xC) || \
    defined(STM32L162xCA) || defined(STM32L162xD) || defined(STM32L162xDX) || \
    defined(STM32L162xE)
    #include "stm32l1xx_hal.h"
#elif defined(STM32L412xx) || defined(STM32L422xx) || defined(STM32L431xx) || \
    defined(STM32L432xx) || defined(STM32L433xx) || defined(STM32L442xx) || \
    defined(STM32L443xx) || defined(STM32L451xx) || defined(STM32L452xx) || \
    defined(STM32L462xx) || defined(STM32L471xx) || defined(STM32L475xx) || \
    defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx) || \
    defined(STM32L496xx) || defined(STM32L4A6xx) || defined(STM32L4P5xx) || \
    defined(STM32L4Q5xx) || defined(STM32L4R5xx) || defined(STM32L4R7xx) || \
    defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || \
    defined(STM32L4S9xx)
    #include "stm32l4xx_hal.h"
#elif defined(STM32L552xx) || defined(STM32L562xx)
    #include "stm32l5xx_hal.h"
#elif defined(STM32G030xx) || defined(STM32G031xx) || defined(STM32G041xx) || \
    defined(STM32G050xx) || defined(STM32G051xx) || defined(STM32G061xx) || \
    defined(STM32G070xx) || defined(STM32G071xx) || defined(STM32G081xx) || \
    defined(STM32G0B0xx) || defined(STM32G0B1xx) || defined(STM32G0C1xx)
    #include "stm32g0xx_hal.h"
#elif defined(STM32G431xx) || defined(STM32G441xx) || defined(STM32G471xx) || \
    defined(STM32G473xx) || defined(STM32G474xx) || defined(STM32G483xx) || \
    defined(STM32G484xx) || defined(STM32G491xx) || defined(STM32G4A1xx)
    #include "stm32g4xx_hal.h"
#elif defined(STM32WB10xx) || defined(STM32WB15xx) || defined(STM32WB1Mxx) || \
    defined(STM32WB30xx) || defined(STM32WB35xx) || defined(STM32WB50xx) || \
    defined(STM32WB55xx) || defined(STM32WB5Mxx)
    #include "stm32wbxx_hal.h"
#elif defined(STM32WL54xx) || defined(STM32WL55xx) || defined(STM32WLE4xx) || \
    defined(STM32WLE5xx)
    #include "stm32wlxx_hal.h"
#else
    #error "STM32 device not detected. Ensure your STM32CubeMX project defines the device (e.g., STM32F303xC, STM32H743xx)"
#endif

#include "platform_status.h"

/* ==================== HAL Type Forward Declarations ==================== */
/* These ensure the types are available even if the peripheral isn't configured in CubeMX */

#ifndef CAN_HandleTypeDef
typedef struct __CAN_HandleTypeDef CAN_HandleTypeDef;
#endif

#ifndef UART_HandleTypeDef
typedef struct __UART_HandleTypeDef UART_HandleTypeDef;
#endif

#ifndef SPI_HandleTypeDef
typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
#endif

#ifndef ADC_HandleTypeDef
typedef struct __ADC_HandleTypeDef ADC_HandleTypeDef;
#endif

#ifndef TIM_HandleTypeDef
typedef struct __TIM_HandleTypeDef TIM_HandleTypeDef;
#endif

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
