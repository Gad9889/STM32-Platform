/**
 * @file main.c
 * @brief CAN Communication Example
 * @author Ben Gurion Racing Team
 * 
 * This example demonstrates:
 * - Platform initialization
 * - CAN message transmission
 * - CAN message reception with callbacks
 * - Basic error handling
 */

#include "main.h"
#include "callbacks.h"
#include "platform_status.h"
#include <stdio.h>

/* Private variables */
CAN_HandleTypeDef hcan1;
UART_HandleTypeDef huart2;  // For printf debugging

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_USART2_UART_Init(void);

/**
 * @brief Main application entry point
 */
int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    
    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_CAN1_Init();
    MX_USART2_UART_Init();
    
    /* Configure platform handlers */
    handler_set_t handlers = {
        .hcan1 = &hcan1,
        .huart2 = &huart2,  // For printf
        .hcan2 = NULL,
        .hcan3 = NULL,
        .huart1 = NULL,
        .huart3 = NULL,
        .hspi1 = NULL,
        .hspi2 = NULL,
        .hspi3 = NULL,
        .hadc1 = NULL,
        .hadc2 = NULL,
        .hadc3 = NULL,
        .htim2 = NULL,
        .htim3 = NULL,
        .htim4 = NULL
    };
    
    /* Initialize platform */
    printf("\r\n=== STM32 Platform CAN Example ===\r\n");
    printf("Initializing platform...\r\n");
    
    PlatformInit(&handlers, 64);  // 64-message queue
    
    printf("Platform initialized successfully!\r\n");
    printf("Waiting for CAN messages...\r\n\r\n");
    
    /* Example: Send a CAN message */
    can_message_t test_msg = {
        .id = 0x123,
        .data = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04}
    };
    
    plt_status_t status = plt_CanSendMsg(Can1, &test_msg);
    if (status == PLT_OK) {
        printf("✓ Test message sent (ID: 0x123)\r\n");
    } else {
        printf("✗ Failed to send message: %s\r\n", plt_StatusToString(status));
    }
    
    /* Main loop */
    uint32_t last_heartbeat = 0;
    uint32_t message_count = 0;
    
    while (1)
    {
        /* Process received CAN messages */
        plt_CanProcessRxMsgs();
        
        /* Heartbeat every 1 second */
        if (HAL_GetTick() - last_heartbeat > 1000) {
            last_heartbeat = HAL_GetTick();
            
            /* Send heartbeat message */
            can_message_t heartbeat = {
                .id = 0x100,
                .data = {
                    (message_count >> 24) & 0xFF,
                    (message_count >> 16) & 0xFF,
                    (message_count >> 8) & 0xFF,
                    message_count & 0xFF,
                    0, 0, 0, 0
                }
            };
            
            plt_CanSendMsg(Can1, &heartbeat);
            printf("Heartbeat sent. Messages received: %lu\r\n", message_count);
        }
        
        HAL_Delay(10);  // 10ms loop time
    }
}

/**
 * @brief System Clock Configuration
 * @note Configure for 168 MHz system clock (STM32F4)
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* Configure main internal regulator output voltage */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    /* Initializes the RCC Oscillators */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    
    /* Initializes the CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  // 42 MHz for CAN
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

/**
 * @brief CAN1 Initialization Function
 */
static void MX_CAN1_Init(void)
{
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 4;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = ENABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    
    if (HAL_CAN_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief USART2 Initialization Function (for printf)
 */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief GPIO Initialization Function
 */
static void MX_GPIO_Init(void)
{
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

/**
 * @brief Error Handler
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {
        // Stay here on error
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    printf("Assert failed: file %s line %lu\r\n", file, line);
}
#endif
