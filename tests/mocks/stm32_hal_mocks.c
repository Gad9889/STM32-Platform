/**
 * @file stm32_hal_mocks.c
 * @brief Mock implementations of STM32 HAL functions for unit testing
 * 
 * This file provides stub implementations of HAL functions so that
 * platform code can be tested without actual hardware.
 */

#include "stm32_hal_mocks.h"
#include <string.h>

// ==================== Mock State ====================

static HAL_StatusTypeDef mock_hal_status = HAL_OK;
static uint32_t mock_tick = 0;

// Mock CAN state
static uint8_t mock_can_rx_pending = 0;
static CAN_RxHeaderTypeDef mock_can_rx_header;
static uint8_t mock_can_rx_data[8];

// Mock UART state
static uint8_t mock_uart_rx_data[256];
static uint16_t mock_uart_rx_size = 0;

// ==================== Helper Functions ====================

void Mock_HAL_Reset(void) {
    mock_hal_status = HAL_OK;
    mock_tick = 0;
    mock_can_rx_pending = 0;
    mock_uart_rx_size = 0;
    memset(&mock_can_rx_header, 0, sizeof(mock_can_rx_header));
    memset(mock_can_rx_data, 0, sizeof(mock_can_rx_data));
    memset(mock_uart_rx_data, 0, sizeof(mock_uart_rx_data));
}

void Mock_HAL_SetStatus(HAL_StatusTypeDef status) {
    mock_hal_status = status;
}

void Mock_HAL_SetTick(uint32_t tick) {
    mock_tick = tick;
}

void Mock_CAN_SetRxMessage(uint32_t id, uint8_t *data, uint8_t dlc) {
    mock_can_rx_header.StdId = id;
    mock_can_rx_header.DLC = dlc;
    memcpy(mock_can_rx_data, data, dlc);
    mock_can_rx_pending = 1;
}

// ==================== HAL General ====================

void Error_Handler(void) {
    // Mock error handler - do nothing in tests
    // Can be overridden in tests to track errors
}

uint32_t HAL_GetTick(void) {
    return mock_tick++;
}

// ==================== HAL CAN ====================

HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan) {
    (void)hcan;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_CAN_Stop(CAN_HandleTypeDef *hcan) {
    (void)hcan;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_CAN_ConfigFilter(CAN_HandleTypeDef *hcan, CAN_FilterTypeDef *sFilterConfig) {
    (void)hcan;
    (void)sFilterConfig;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t ActiveITs) {
    (void)hcan;
    (void)ActiveITs;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_CAN_DeactivateNotification(CAN_HandleTypeDef *hcan, uint32_t InactiveITs) {
    (void)hcan;
    (void)InactiveITs;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_CAN_AddTxMessage(
    CAN_HandleTypeDef *hcan,
    CAN_TxHeaderTypeDef *pHeader,
    uint8_t aData[],
    uint32_t *pTxMailbox)
{
    (void)hcan;
    (void)pHeader;
    (void)aData;
    if (pTxMailbox != NULL) {
        *pTxMailbox = 0; // Assume mailbox 0
    }
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_CAN_GetRxMessage(
    CAN_HandleTypeDef *hcan,
    uint32_t RxFifo,
    CAN_RxHeaderTypeDef *pHeader,
    uint8_t aData[])
{
    (void)hcan;
    (void)RxFifo;
    
    if (mock_can_rx_pending) {
        if (pHeader != NULL) {
            memcpy(pHeader, &mock_can_rx_header, sizeof(CAN_RxHeaderTypeDef));
        }
        if (aData != NULL) {
            memcpy(aData, mock_can_rx_data, 8);
        }
        mock_can_rx_pending = 0;
        return HAL_OK;
    }
    
    return HAL_ERROR;
}

uint32_t HAL_CAN_GetRxFifoFillLevel(CAN_HandleTypeDef *hcan, uint32_t RxFifo) {
    (void)hcan;
    (void)RxFifo;
    return mock_can_rx_pending ? 1 : 0;
}

uint32_t HAL_CAN_GetError(CAN_HandleTypeDef *hcan) {
    (void)hcan;
    return 0; // No errors
}

HAL_StatusTypeDef HAL_CAN_ResetError(CAN_HandleTypeDef *hcan) {
    (void)hcan;
    return HAL_OK;
}

// ==================== HAL UART ====================

HAL_StatusTypeDef HAL_UART_Transmit(
    UART_HandleTypeDef *huart,
    const uint8_t *pData,
    uint16_t Size,
    uint32_t Timeout)
{
    (void)huart;
    (void)pData;
    (void)Size;
    (void)Timeout;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_UART_Receive(
    UART_HandleTypeDef *huart,
    uint8_t *pData,
    uint16_t Size,
    uint32_t Timeout)
{
    (void)huart;
    (void)Timeout;
    
    if (pData != NULL && mock_uart_rx_size > 0) {
        uint16_t copy_size = (Size < mock_uart_rx_size) ? Size : mock_uart_rx_size;
        memcpy(pData, mock_uart_rx_data, copy_size);
        mock_uart_rx_size = 0;
        return HAL_OK;
    }
    
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size) {
    (void)huart;
    (void)pData;
    (void)Size;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size) {
    (void)huart;
    (void)pData;
    (void)Size;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_UART_Abort(UART_HandleTypeDef *huart) {
    (void)huart;
    return HAL_OK;
}

// ==================== HAL SPI ====================

HAL_StatusTypeDef HAL_SPI_Transmit(
    SPI_HandleTypeDef *hspi,
    const uint8_t *pData,
    uint16_t Size,
    uint32_t Timeout)
{
    (void)hspi;
    (void)pData;
    (void)Size;
    (void)Timeout;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_SPI_Receive(
    SPI_HandleTypeDef *hspi,
    uint8_t *pData,
    uint16_t Size,
    uint32_t Timeout)
{
    (void)hspi;
    (void)pData;
    (void)Size;
    (void)Timeout;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive(
    SPI_HandleTypeDef *hspi,
    const uint8_t *pTxData,
    uint8_t *pRxData,
    uint16_t Size,
    uint32_t Timeout)
{
    (void)hspi;
    (void)pTxData;
    (void)pRxData;
    (void)Size;
    (void)Timeout;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_SPI_Transmit_DMA(SPI_HandleTypeDef *hspi, const uint8_t *pData, uint16_t Size) {
    (void)hspi;
    (void)pData;
    (void)Size;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_SPI_Receive_DMA(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size) {
    (void)hspi;
    (void)pData;
    (void)Size;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(
    SPI_HandleTypeDef *hspi,
    const uint8_t *pTxData,
    uint8_t *pRxData,
    uint16_t Size)
{
    (void)hspi;
    (void)pTxData;
    (void)pRxData;
    (void)Size;
    return mock_hal_status;
}

// ==================== HAL ADC ====================

HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc) {
    (void)hadc;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *hadc) {
    (void)hadc;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef *hadc, uint32_t *pData, uint32_t Length) {
    (void)hadc;
    (void)pData;
    (void)Length;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_ADC_Stop_DMA(ADC_HandleTypeDef *hadc) {
    (void)hadc;
    return mock_hal_status;
}

uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *hadc) {
    (void)hadc;
    return 2048; // Return mid-scale value
}

// ==================== HAL TIM ====================

HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *htim) {
    (void)htim;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_TIM_Base_Stop(TIM_HandleTypeDef *htim) {
    (void)htim;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t Channel) {
    (void)htim;
    (void)Channel;
    return mock_hal_status;
}

HAL_StatusTypeDef HAL_TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t Channel) {
    (void)htim;
    (void)Channel;
    return mock_hal_status;
}

// ==================== HAL RCC ====================

uint32_t HAL_RCC_GetPCLK1Freq(void) {
    return 42000000; // 42 MHz
}

uint32_t HAL_RCC_GetPCLK2Freq(void) {
    return 84000000; // 84 MHz
}
