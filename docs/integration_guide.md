# STM32 Platform Integration Guide

Complete step-by-step guide for integrating the STM32 Platform into your project.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installation](#installation)
3. [STM32CubeMX Configuration](#stm32cubemx-configuration)
4. [Project Setup](#project-setup)
5. [Basic Usage](#basic-usage)
6. [Advanced Configuration](#advanced-configuration)
7. [Troubleshooting](#troubleshooting)

## Prerequisites

### Hardware

- STM32 microcontroller (F0, F1, F4, F7, H7 families supported)
- Development board (NUCLEO, Discovery, or custom)
- Debugger (ST-LINK V2/V3)
- Peripherals (CAN transceiver, UART-USB adapter, etc.)

### Software

- **VS Code** with C/C++ extension
- **STM32CubeMX** for peripheral configuration
- **ARM GCC toolchain**
- **ST-LINK utility** for programming
- **Serial terminal** (screen, minicom, PuTTY) for UART

### Knowledge

- C programming
- STM32 HAL library
- Target peripheral specifications

## Installation

### Method 1: Direct Copy (Recommended)

1. **Clone or download** this repository:

   ```bash
   git clone https://github.com/Ben-Gurion-Racing/STM32_Platform.git
   ```

2. **Copy platform files** to your STM32 project:

   ```
   YourProject/
   ├── Core/
   │   ├── Inc/
   │   │   └── (your includes)
   │   └── Src/
   │       └── (your sources)
   ├── Platform/          ← Create this directory
   │   ├── Inc/           ← Copy STM32_Platform/Inc here
   │   └── Src/           ← Copy STM32_Platform/Src here
   ├── Drivers/
   └── ...
   ```

3. **Configure build system** (Makefile or CMakeLists.txt):

   - Add include path: `Platform/Inc`
   - Add source files: `Platform/Src/*.c` (select required modules only)

4. **VS Code configuration**:
   - Update `.vscode/c_cpp_properties.json` with include paths
   - Configure build tasks in `.vscode/tasks.json`

### Method 2: Git Submodule

```bash
cd your_project
git submodule add https://github.com/Ben-Gurion-Racing/STM32_Platform.git Platform
git submodule update --init --recursive
```

Update include paths as in Method 1.

## STM32CubeMX Configuration

### 1. System Configuration

**RCC (Reset and Clock Control)**:

- Enable HSE (external crystal) or HSI (internal oscillator)
- Configure PLL for desired system clock (e.g., 168 MHz for STM32F4)

**SYS**:

- Debug: Serial Wire
- Timebase Source: SysTick

### 2. CAN Configuration

**CAN1 Settings**:

- Mode: Master
- Prescaler: Calculate for desired baud rate
  - 500 kbit/s @ 42MHz APB1: Prescaler=4, BS1=13TQ, BS2=2TQ
  - 250 kbit/s @ 42MHz APB1: Prescaler=8, BS1=13TQ, BS2=2TQ
- Auto Bus-Off: Enable
- Auto Retransmission: Enable

**GPIO Configuration**:

- PA11: CAN1_RX
- PA12: CAN1_TX
- Alternate function mode

**NVIC Settings**:

- Enable CAN1 RX0 interrupt (optional, platform handles DMA)

### 3. UART Configuration

**UART1/2/3 Settings**:

- Mode: Asynchronous
- Baud Rate: 115200 (or as needed)
- Word Length: 8 bits
- Stop Bits: 1
- Parity: None
- Hardware Flow Control: None

**DMA Configuration** (CRITICAL for platform):

- UART TX: DMA Request, Mode=Normal, Priority=Medium
- UART RX: DMA Request, Mode=Circular, Priority=High

**GPIO Configuration**:

- UART TX: Alternate function, Pull-up
- UART RX: Alternate function, Pull-up

### 4. SPI Configuration (if needed)

**SPI1/2/3 Settings**:

- Mode: Master or Slave
- Data Size: 8 bits
- Clock Polarity: Low (or as per device)
- Clock Phase: 1 Edge
- NSS: Software or Hardware
- Baud Rate Prescaler: Based on peripheral requirements

**DMA Configuration**:

- SPI TX: DMA Request, Mode=Normal
- SPI RX: DMA Request, Mode=Normal

### 5. ADC Configuration (if needed)

**ADC1/2/3 Settings**:

- Mode: Independent or Dual
- Resolution: 12 bits
- Data Alignment: Right
- Scan Conversion: Enable
- Continuous Conversion: Enable
- DMA Continuous Requests: Enable

**Channel Configuration**:

- Add channels (e.g., IN0, IN1, IN2)
- Sampling Time: 84 cycles (or higher for accuracy)

**DMA Configuration**:

- ADC: DMA Request, Mode=Circular, Data Width=Half Word

### 6. TIM Configuration (for PWM)

**TIM2/3/4 Settings**:

- Clock Source: Internal Clock
- Channel: PWM Generation CH1/CH2/CH3/CH4
- Mode: PWM Mode 1
- Pulse: 0 (will be set at runtime)
- Prescaler: 0 (will be calculated by platform)
- Counter Period: 1000 (will be calculated by platform)

### 7. Code Generation

**Project Settings**:

- Toolchain: Makefile
- Application Structure: Basic

**Code Generator Options**:

- Generate peripheral initialization as .c/.h pairs
- Keep user code on re-generation

Generate code.

## Project Setup

### 1. Include Platform Headers

In your `main.c`:

```c
/* USER CODE BEGIN Includes */
#include "callbacks.h"
#include "platform_status.h"
/* USER CODE END Includes */
```

### 2. Configure Platform Handlers

After peripheral initialization in `main()`:

```c
/* USER CODE BEGIN 2 */

// Configure platform handlers
handler_set_t handlers = {
    .hcan1 = &hcan1,
    .huart2 = &huart2,  // For printf debugging
    .hspi1 = &hspi1,
    .hadc1 = &hadc1,
    .htim2 = &htim2,
    // Set unused peripherals to NULL
    .hcan2 = NULL,
    .hcan3 = NULL,
    .huart1 = NULL,
    .huart3 = NULL,
    .hspi2 = NULL,
    .hspi3 = NULL,
    .hadc2 = NULL,
    .hadc3 = NULL,
    .htim3 = NULL,
    .htim4 = NULL
};

// Initialize platform
PlatformInit(&handlers, 64);  // 64 = queue size

printf("Platform initialized\r\n");

/* USER CODE END 2 */
```

### 3. Main Loop Processing

```c
/* USER CODE BEGIN WHILE */
while (1)
{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // Process peripheral messages
    plt_CanProcessRxMsgs();
    plt_UartProcessRxMsgs();
    plt_SpiProcessRxMsgs();

    // Your application code

    HAL_Delay(10);  // 10ms loop time
}
/* USER CODE END 3 */
```

### 4. Implement Callbacks

Edit `callbacks.c` to define your message handlers:

```c
void CanRxCallback(can_message_t *msg) {
    switch (msg->id) {
        case 0x100:
            // Handle motor controller
            break;
        case 0x200:
            // Handle battery management
            break;
        default:
            break;
    }
}

void UartRxCallback(uart_message_t *msg) {
    // Handle UART messages
}

void SpiRxCallback(spi_message_t *msg) {
    // Handle SPI messages
}
```

## Basic Usage

### Sending CAN Messages

```c
can_message_t msg = {
    .id = 0x123,
    .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
};

plt_status_t status = plt_CanSendMsg(Can1, &msg);
if (status != PLT_OK) {
    printf("CAN send error: %s\n", plt_StatusToString(status));
}
```

### Sending UART Messages

```c
uart_message_t msg = {
    .id = 0x01,
    .data = {0xAA, 0xBB, 0xCC, 0xDD, 0, 0, 0, 0}
};

plt_UartSendMsg(Uart1, &msg);
```

### Using printf (UART2)

```c
printf("Temperature: %d°C\r\n", temp);
```

The platform automatically redirects printf to UART2.

### Generating PWM

```c
// 1kHz PWM at 50% duty cycle
plt_StartPWM(Tim2, TIM_CHANNEL_1, 1000, 50.0f);

// Change duty cycle
plt_StartPWM(Tim2, TIM_CHANNEL_1, 1000, 75.0f);

// Stop PWM
plt_StopPWM(Tim2, TIM_CHANNEL_1);
```

### Reading ADC Values

ADC values are automatically read and published to the CAN queue:

```c
void CanRxCallback(can_message_t *msg) {
    if (msg->id == Internal_ADC) {
        uint16_t adc_ch0 = (msg->data[0] << 8) | msg->data[1];
        uint16_t adc_ch1 = (msg->data[2] << 8) | msg->data[3];
        uint16_t adc_ch2 = (msg->data[4] << 8) | msg->data[5];

        printf("ADC: %u, %u, %u\r\n", adc_ch0, adc_ch1, adc_ch2);
    }
}
```

## Advanced Configuration

### Custom Queue Sizes

Adjust queue sizes based on message rate:

```c
// High CAN traffic
#define CAN_QUEUE_SIZE 128

// Low UART traffic
#define UART_QUEUE_SIZE 16

PlatformInit(&handlers, CAN_QUEUE_SIZE);
```

### Custom ADC Configuration

In `adc.h`, adjust for your channel count:

```c
#define ADC1_NUM_SENSORS 3           // Number of ADC channels
#define ADC1_SAMPLES_PER_SENSOR 50   // Averaging factor
```

### Error Handling

```c
plt_status_t status = plt_CanInit(64);
if (status != PLT_OK) {
    switch (status) {
        case PLT_INVALID_PARAM:
            printf("Invalid queue size\r\n");
            break;
        case PLT_NO_MEMORY:
            printf("Queue allocation failed\r\n");
            break;
        case PLT_HAL_ERROR:
            printf("HAL initialization failed\r\n");
            break;
        default:
            printf("Unknown error: %s\r\n", plt_StatusToString(status));
            break;
    }
    Error_Handler();
}
```

### RTOS Integration

When using FreeRTOS:

1. **Protect queue operations**:

   ```c
   osMutexWait(canMutex, osWaitForever);
   plt_CanProcessRxMsgs();
   osMutexRelease(canMutex);
   ```

2. **Create dedicated tasks**:
   ```c
   void CanRxTask(void *argument) {
       while (1) {
           plt_CanProcessRxMsgs();
           osDelay(10);
       }
   }
   ```

## Troubleshooting

### Issue: Compilation Errors - "HAL_CAN_MODULE_ENABLED not defined"

**Solution**: The platform uses conditional compilation. Ensure modules are enabled in `stm32fxxx_hal_conf.h`:

```c
#define HAL_CAN_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
// etc.
```

### Issue: Linker Error - "undefined reference to `plt_CanInit`"

**Solution**: Add platform source files to your project:

- Check that `can.c` is in your source folder
- Verify include paths are correct
- Rebuild project (Clean → Build)

### Issue: CAN messages not received

**Solution**:

1. Check DMA is enabled for CAN RX
2. Verify `plt_CanProcessRxMsgs()` is called in main loop
3. Check CAN filter configuration allows your message IDs
4. Verify hardware (transceiver, termination, wiring)

### Issue: printf doesn't work

**Solution**:

1. Ensure UART2 is initialized and passed to platform
2. Check UART TX pin is connected
3. Verify baud rate in terminal matches (115200)
4. Add `setvbuf(stdout, NULL, _IONBF, 0);` after initialization

### Issue: Queue overflow - messages lost

**Solution**:

1. Increase queue size in `PlatformInit()`
2. Call `plt_XxxProcessRxMsgs()` more frequently
3. Optimize callback processing time
4. Consider using RTOS for dedicated tasks

### Issue: Hard fault / crash

**Solution**:

1. Enable UsageFault, BusFault, MemManage handlers
2. Use debugger to check stack trace
3. Verify all pointers are valid before use
4. Check for stack overflow (increase stack size)
5. Ensure DMA buffers are not in CCM RAM (some STM32 models)

## Performance Tips

1. **Process messages efficiently**: Keep callbacks short
2. **Adjust loop timing**: Match message rate (10ms typical)
3. **Use DMA**: Already configured by platform
4. **Optimize printf**: Use sparingly in production
5. **Monitor queue depth**: Check for overflows

## Next Steps

- Review [examples/](../examples/) for working code
- Read [API Reference](api_reference.md) for detailed function docs
- Check [Architecture Guide](architecture.md) for system design
- Join discussions on GitHub for support

## Support

- **Issues**: [GitHub Issues](https://github.com/Ben-Gurion-Racing/STM32_Platform/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Ben-Gurion-Racing/STM32_Platform/discussions)
- **Contributions**: See [CONTRIBUTING.md](../CONTRIBUTING.md)

---

**Happy Coding! 🚀**
