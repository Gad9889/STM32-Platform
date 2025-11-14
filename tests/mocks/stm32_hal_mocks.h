/**
 * @file stm32_hal_mocks.h
 * @brief Mock header for STM32 HAL functions
 * 
 * Provides minimal HAL type definitions and function declarations for testing
 */

#ifndef STM32_HAL_MOCKS_H
#define STM32_HAL_MOCKS_H

#include <stdint.h>
#include <stddef.h>

// ==================== HAL Status ====================

typedef enum {
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

// ==================== HAL Peripheral States ====================

typedef enum {
    HAL_CAN_STATE_RESET      = 0x00U,
    HAL_CAN_STATE_READY      = 0x01U,
    HAL_CAN_STATE_LISTENING  = 0x02U,
    HAL_CAN_STATE_SLEEP_PENDING  = 0x03U,
    HAL_CAN_STATE_SLEEP_ACTIVE   = 0x04U,
    HAL_CAN_STATE_ERROR      = 0x05U
} HAL_CAN_StateTypeDef;

typedef enum {
    HAL_UART_STATE_RESET     = 0x00U,
    HAL_UART_STATE_READY     = 0x20U,
    HAL_UART_STATE_BUSY      = 0x24U,
    HAL_UART_STATE_BUSY_TX   = 0x21U,
    HAL_UART_STATE_BUSY_RX   = 0x22U,
    HAL_UART_STATE_BUSY_TX_RX = 0x23U,
    HAL_UART_STATE_TIMEOUT   = 0xA0U,
    HAL_UART_STATE_ERROR     = 0xE0U
} HAL_UART_StateTypeDef;

typedef enum {
    HAL_SPI_STATE_RESET      = 0x00U,
    HAL_SPI_STATE_READY      = 0x01U,
    HAL_SPI_STATE_BUSY       = 0x02U,
    HAL_SPI_STATE_BUSY_TX    = 0x03U,
    HAL_SPI_STATE_BUSY_RX    = 0x04U,
    HAL_SPI_STATE_BUSY_TX_RX = 0x05U,
    HAL_SPI_STATE_ERROR      = 0x06U,
    HAL_SPI_STATE_ABORT      = 0x07U
} HAL_SPI_StateTypeDef;

// ==================== CAN Definitions ====================

#define CAN_IT_RX_FIFO0_MSG_PENDING     0x00000001U
#define CAN_IT_RX_FIFO1_MSG_PENDING     0x00000010U
#define CAN_RX_FIFO0                    0x00000000U
#define CAN_RX_FIFO1                    0x00000001U

// CAN Filter Mode
#define CAN_FILTERMODE_IDMASK       0x00000000U
#define CAN_FILTERMODE_IDLIST       0x00000001U

// CAN Filter Scale
#define CAN_FILTERSCALE_16BIT       0x00000000U
#define CAN_FILTERSCALE_32BIT       0x00000001U

// CAN FIFO
#define CAN_FILTER_FIFO0           0x00000000U
#define CAN_FILTER_FIFO1           0x00000001U

// CAN Error Codes
#define HAL_CAN_ERROR_NONE            0x00000000U
#define HAL_CAN_ERROR_EWG             0x00000001U
#define HAL_CAN_ERROR_EPV             0x00000002U
#define HAL_CAN_ERROR_BOF             0x00000004U
#define HAL_CAN_ERROR_STF             0x00000008U
#define HAL_CAN_ERROR_FOR             0x00000010U
#define HAL_CAN_ERROR_ACK             0x00000020U
#define HAL_CAN_ERROR_BR              0x00000040U
#define HAL_CAN_ERROR_BD              0x00000080U
#define HAL_CAN_ERROR_CRC             0x00000100U
#define HAL_CAN_ERROR_NOT_INITIALIZED 0x00000200U
#define HAL_CAN_ERROR_NOT_READY       0x00000400U
#define HAL_CAN_ERROR_NOT_STARTED     0x00000800U
#define HAL_CAN_ERROR_PARAM           0x00001000U

typedef struct {
    uint32_t StdId;
    uint32_t ExtId;
    uint32_t IDE;
    uint32_t RTR;
    uint32_t DLC;
    uint32_t Timestamp;
    uint32_t FilterMatchIndex;
} CAN_RxHeaderTypeDef;

typedef struct {
    uint32_t StdId;
    uint32_t ExtId;
    uint32_t IDE;
    uint32_t RTR;
    uint32_t DLC;
    uint32_t TransmitGlobalTime;
} CAN_TxHeaderTypeDef;

typedef struct {
    uint32_t FilterIdHigh;
    uint32_t FilterIdLow;
    uint32_t FilterMaskIdHigh;
    uint32_t FilterMaskIdLow;
    uint32_t FilterFIFOAssignment;
    uint32_t FilterBank;
    uint32_t FilterMode;
    uint32_t FilterScale;
    uint32_t FilterActivation;
    uint32_t SlaveStartFilterBank;
} CAN_FilterTypeDef;

typedef struct {
    void *Instance;
    HAL_CAN_StateTypeDef State;
    uint32_t ErrorCode;
} CAN_HandleTypeDef;

// CAN Instances
#define CAN1 ((void*)0x40006400U)
#define CAN2 ((void*)0x40006800U)

// ==================== UART Definitions ====================

typedef struct {
    void *Instance;
    HAL_UART_StateTypeDef State;
} UART_HandleTypeDef;

#define USART1 ((void*)0x40011000U)
#define USART2 ((void*)0x40004400U)
#define USART3 ((void*)0x40004800U)

// ==================== SPI Definitions ====================

#define SPI_MODE_MASTER                 0x00000104U
#define SPI_MODE_SLAVE                  0x00000000U

typedef struct {
    uint32_t Mode;
} SPI_InitTypeDef;

typedef struct {
    void *Instance;
    SPI_InitTypeDef Init;
    HAL_SPI_StateTypeDef State;
    void (*RxCpltCallback)(struct __SPI_HandleTypeDef *hspi);
} SPI_HandleTypeDef;

#define SPI1 ((void*)0x40013000U)
#define SPI2 ((void*)0x40003800U)
#define SPI3 ((void*)0x40003C00U)

// ==================== ADC Definitions ====================

typedef struct {
    void *Instance;
} ADC_HandleTypeDef;

#define ADC1 ((void*)0x40012000U)
#define ADC2 ((void*)0x40012100U)
#define ADC3 ((void*)0x40012200U)

// ==================== TIM Definitions ====================

#define TIM_CHANNEL_1                   0x00000000U
#define TIM_CHANNEL_2                   0x00000004U
#define TIM_CHANNEL_3                   0x00000008U
#define TIM_CHANNEL_4                   0x0000000CU

typedef struct {
    uint32_t Prescaler;
    uint32_t Period;
} TIM_Base_InitTypeDef;

typedef struct {
    void *Instance;
    TIM_Base_InitTypeDef Init;
} TIM_HandleTypeDef;

#define TIM2 ((void*)0x40000000U)
#define TIM3 ((void*)0x40000400U)
#define TIM4 ((void*)0x40000800U)

// ==================== DMA Definitions ====================

#define UNUSED(x) ((void)(x))

// ==================== Macro Helpers ====================

#define __HAL_TIM_SET_PRESCALER(htim, prescaler) do { (htim)->Init.Prescaler = (prescaler); } while(0)
#define __HAL_TIM_SET_AUTORELOAD(htim, period) do { (htim)->Init.Period = (period); } while(0)
#define __HAL_TIM_SET_COMPARE(htim, channel, value) ((void)(htim), (void)(channel), (void)(value))

// ==================== Module Enable Defines ====================

#define HAL_CAN_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED

// ==================== Mock Control Functions ====================

void Mock_HAL_Reset(void);
void Mock_HAL_SetStatus(HAL_StatusTypeDef status);
void Mock_HAL_SetTick(uint32_t tick);
void Mock_CAN_SetRxMessage(uint32_t id, uint8_t *data, uint8_t dlc);

// ==================== HAL Function Declarations ====================

// General
void Error_Handler(void);
uint32_t HAL_GetTick(void);

// CAN
HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_Stop(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_ConfigFilter(CAN_HandleTypeDef *hcan, CAN_FilterTypeDef *sFilterConfig);
HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t ActiveITs);
HAL_StatusTypeDef HAL_CAN_DeactivateNotification(CAN_HandleTypeDef *hcan, uint32_t InactiveITs);
HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox);
HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t RxFifo, CAN_RxHeaderTypeDef *pHeader, uint8_t aData[]);
uint32_t HAL_CAN_GetRxFifoFillLevel(CAN_HandleTypeDef *hcan, uint32_t RxFifo);
uint32_t HAL_CAN_GetError(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_ResetError(CAN_HandleTypeDef *hcan);

// UART
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_UART_Abort(UART_HandleTypeDef *huart);

// SPI
HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, const uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_Transmit_DMA(SPI_HandleTypeDef *hspi, const uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_SPI_Receive_DMA(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(SPI_HandleTypeDef *hspi, const uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);

// ADC
HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef *hadc, uint32_t *pData, uint32_t Length);
HAL_StatusTypeDef HAL_ADC_Stop_DMA(ADC_HandleTypeDef *hadc);
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *hadc);

// TIM
HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_Stop(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t Channel);

// RCC
uint32_t HAL_RCC_GetPCLK1Freq(void);
uint32_t HAL_RCC_GetPCLK2Freq(void);

// Add main.h compatibility
#ifndef MAIN_H
#define MAIN_H
#include "stm32_hal_mocks.h"
#endif

#endif // STM32_HAL_MOCKS_H
