# STM32 Platform - CubeMX Integration Guide

**Platform Version:** v2.0.0  
**Last Updated:** November 15, 2025

This guide shows how to integrate the STM32 Platform into an STM32CubeMX project.

---

## Quick Start

### 1. Add Platform Files to Your Project

Copy the following files to your STM32CubeMX project:

\`\`\`
YourProject/
├── Core/
│   ├── Inc/
│   │   ├── stm32_platform.h       ← Copy
│   │   ├── platform_status.h      ← Copy
│   │   ├── hashtable.h            ← Copy
│   │   ├── database.h             ← Copy
│   │   ├── DbSetFunctions.h       ← Copy
│   │   └── utils.h                ← Copy
│   └── Src/
│       ├── stm32_platform.c       ← Copy
│       ├── platform_status.c      ← Copy
│       ├── hashtable.c            ← Copy
│       ├── database.c             ← Copy
│       ├── DbSetFunctions.c       ← Copy
│       └── utils.c                ← Copy
\`\`\`

**⚠️ Important:** Do NOT copy \`callbacks.c\` or \`callbacks.h\` - they are legacy stubs not needed in v2.0.0.

---

### 2. Configure STM32CubeMX Peripherals

Enable only the peripherals your project needs:

#### Required Configuration:
- **System Core → SYS:** Enable at minimum (for HAL_GetTick)
- **System Core → GPIO:** Enable if using SPI chip select

#### Optional Peripherals (Enable as needed):
- **Connectivity → CAN:** For CAN communication
- **Connectivity → USART/UART:** For UART/debug output
- **Connectivity → SPI:** For SPI devices
- **Analog → ADC:** For analog readings
- **Timers → TIM:** For PWM outputs

**✅ Key Feature:** The platform only compiles code for peripherals you enable. If you don't enable SPI/ADC, you can still use CAN/UART/PWM without compilation errors.

---

### 3. Initialize Platform in main.c

Add platform initialization in your \`main.c\`:

\`\`\`c
/* USER CODE BEGIN Includes */
#include "stm32_platform.h"
/* USER CODE END Includes */

int main(void)
{
  /* MCU Configuration */
  HAL_Init();
  SystemClock_Config();
  
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_USART2_UART_Init();
  // ... other peripheral inits
  
  /* USER CODE BEGIN 2 */
  
  // Initialize platform with your peripheral handles
  PlatformHandles_t handles = {
      .hcan = &hcan,        // If CAN enabled
      .huart = &huart2,     // If UART enabled
      .hspi = NULL,         // Set to &hspi if SPI enabled, else NULL
      .hadc = NULL,         // Set to &hadc if ADC enabled, else NULL
      .htim = &htim1        // If PWM/Timer enabled
  };
  
  Platform.begin(&handles);
  
  /* USER CODE END 2 */
  
  /* Infinite loop */
  while (1)
  {
    /* USER CODE BEGIN 3 */
    
    // Process incoming CAN messages
    P_CAN.handleRxMessages();
    
    // Your application code here
    
    /* USER CODE END 3 */
  }
}
\`\`\`

---

## Using the Platform API

### CAN Communication

\`\`\`c
// Send CAN message
uint8_t data[8] = {0x01, 0x02, 0x03, 0x04};
P_CAN.send(0x123, data, 4);

// Register handler for specific CAN ID
void myCANHandler(CANMessage_t* msg) {
    P_UART.printf("Received CAN ID: 0x%X\\n", msg->id);
}

P_CAN.route(0x123, myCANHandler);

// In main loop - process received messages
while (1) {
    P_CAN.handleRxMessages();
}
\`\`\`

### UART Communication

\`\`\`c
// Print debug messages
P_UART.printf("System initialized\\r\\n");
P_UART.println("Debug info");

// Send raw data
uint8_t buffer[10];
P_UART.write(buffer, 10);
\`\`\`

### SPI Communication

\`\`\`c
// SPI transfer
uint8_t tx_data[4] = {0x01, 0x02, 0x03, 0x04};
uint8_t rx_data[4];
P_SPI.transfer(tx_data, rx_data, 4);
\`\`\`

### ADC Reading

\`\`\`c
// Read ADC voltage
float voltage = P_ADC.readVoltage(ADC_CHANNEL_1);
P_UART.printf("ADC voltage: %.2fV\\r\\n", voltage);
\`\`\`

### PWM Control

\`\`\`c
// Start PWM
P_PWM.start(&htim1, TIM_CHANNEL_1);

// Set frequency and duty cycle
P_PWM.setFrequency(&htim1, TIM_CHANNEL_1, 1000); // 1kHz
P_PWM.setDutyCycle(&htim1, TIM_CHANNEL_1, 50.0f); // 50%
\`\`\`

---

## Build Configuration

### STM32CubeIDE

Platform files are auto-discovered. No special configuration required.

### CMakeLists.txt

If using CMake:

\`\`\`cmake
# Add platform sources
target_sources(\${PROJECT_NAME} PRIVATE
    Core/Src/stm32_platform.c
    Core/Src/platform_status.c
    Core/Src/utils.c
    Core/Src/hashtable.c
    Core/Src/database.c
    Core/Src/DbSetFunctions.c
)

# Add include directories
target_include_directories(\${PROJECT_NAME} PRIVATE
    Core/Inc
)
\`\`\`

### Makefile

If using Makefile:

\`\`\`makefile
C_SOURCES += \\
Core/Src/stm32_platform.c \\
Core/Src/platform_status.c \\
Core/Src/utils.c \\
Core/Src/hashtable.c \\
Core/Src/database.c \\
Core/Src/DbSetFunctions.c

C_INCLUDES += \\
-ICore/Inc
\`\`\`

---

## Common Issues & Solutions

### Issue: \`'PlatformHandles_t' undeclared\`
**Solution:** Make sure \`#include "stm32_platform.h"\` is in your main.c

### Issue: \`unknown type name 'SPI_HandleTypeDef'\`
**Solution:** Either enable SPI in CubeMX, or set \`.hspi = NULL\` in PlatformHandles_t

### Issue: \`undefined reference to 'Platform'\`
**Solution:** Make sure \`stm32_platform.c\` is being compiled (check your build system)

### Issue: CAN messages not received
**Solution:** Make sure to call \`P_CAN.handleRxMessages()\` in your main loop

### Issue: UART printf not working
**Solution:** Implement \`_write()\` to redirect stdio to UART:

\`\`\`c
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
\`\`\`

### Issue: PWM not starting
**Solution:** Call \`P_PWM.start()\` before setting duty cycle:

\`\`\`c
P_PWM.start(&htim2, TIM_CHANNEL_1);
P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 50.0f);
\`\`\`

---

## HAL Callback Integration

The platform handles HAL callbacks internally. You don't need to modify \`stm32f3xx_it.c\`.

**Callbacks Implemented:**
- \`HAL_CAN_RxFifo0MsgPendingCallback()\` - CAN message reception
- \`HAL_UART_RxCpltCallback()\` - UART byte reception

---

## Migration from Old plt_* API

| Old API (plt_*)               | New API (v2.0.0)          |
|-------------------------------|---------------------------|
| \`plt_CanInit(&hcan, 32)\`      | \`Platform.begin(&handles)\` |
| \`plt_CanSendMessage()\`        | \`P_CAN.send()\`            |
| \`plt_UartPrintln()\`           | \`P_UART.println()\`        |
| \`plt_SetHandlers(&handlers)\`  | Not needed                |
| \`plt_CanProcessRxMsgs()\`      | \`P_CAN.handleRxMessages()\`|
| \`plt_SpiTransfer()\`           | \`P_SPI.transfer()\`        |
| \`plt_AdcReadVoltage()\`        | \`P_ADC.readVoltage()\`     |
| \`plt_PwmSetFrequency()\`       | \`P_PWM.setFrequency()\`    |

---

## Platform Architecture

\`\`\`
┌─────────────────────────────────────────┐
│         Your Application (main.c)       │
│   Platform.begin() / P_CAN.send()      │
└──────────────────┬──────────────────────┘
                   │
┌──────────────────▼──────────────────────┐
│    STM32 Platform (stm32_platform.c)   │
│  • Thread-safe queues (ISR → main)     │
│  • Hashtable routing (CAN messages)    │
│  • Direct HAL calls                    │
└──────────────────┬──────────────────────┘
                   │
┌──────────────────▼──────────────────────┐
│         STM32 HAL (CubeMX Generated)    │
│  HAL_CAN_*() / HAL_UART_*() / etc.     │
└──────────────────┬──────────────────────┘
                   │
┌──────────────────▼──────────────────────┐
│           Hardware (STM32 MCU)          │
└─────────────────────────────────────────┘
\`\`\`

---

## Advanced Features

### CAN Message Routing
Hashtable-based routing with O(1) lookup

### Thread-Safe Queues
ISR-safe operations with critical sections

### Database Storage
Persistent CAN signal storage

---

## Examples

See \`examples/\` directory for complete examples.

---

## Support

- GitHub Issues: https://github.com/Gad9889/STM32-Platform/issues
- Wiki: Check project wiki for detailed documentation

---

**Platform Version:** v2.0.0 (Major refactor - breaking changes from v1.x)
