# STM32 Platform

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CI](https://github.com/Gad9889/STM32-Platform/workflows/CI/badge.svg)](https://github.com/Gad9889/STM32-Platform/actions)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg)](CONTRIBUTING.md)

**Consumer-grade communication platform for STM32 microcontrollers.** Get CAN, UART, SPI, ADC, and PWM working in minutes with an intuitive, Arduino-style API.

Born from Ben-Gurion Racing's Formula Student team, now redesigned for developers who want **simplicity without sacrificing power**.

## ✨ Features

- **🎯 Consumer-Grade API**: Arduino-style interface - `P_CAN.send()`, `P_UART.println()`, `P_ADC.readVoltage()`
- **⚡ One-Click Integration**: VS Code extension automatically adds platform to your STM32 CMake project
- **🤖 Auto-Detection**: Automatically detects STM32 family (F0/F1/F3/F4/F7/H7/L/G/WB/WL) and includes correct HAL
- **🔧 Explicit Control**: No magic - you decide when to process messages with `.handleRxMessages()`
- **🛡️ Production-Ready**: Used in Formula Student racing, standardized error handling with plt_status_t
- **🧪 Well-Tested**: 49 unit tests, 40%+ coverage, CI/CD with GitHub Actions
- **📦 Testing Starter Kit**: Optional Unity test framework setup for testing your application logic

## 🚀 Two APIs, Your Choice

**New API** (Recommended - Arduino-style with P_ prefix to avoid HAL conflicts):

```c
Platform.begin(&hcan1, &huart2, &hspi1, NULL, NULL);
P_CAN.send(0x123, data, 8);
P_UART.printf("Speed: %d km/h\n", speed);
P_ADC.readVoltage(0);
P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 50.0f);
```

**Legacy API** (Still supported - plt_ prefix functions):

```c
plt_SetHandlers(&handlers);
plt_status_t status = plt_CanInit(32);
if (status != PLT_OK) { /* handle error */ }
plt_CanSendMsg(Can1, &msg);
plt_CanProcessRxMsgs();
```

---

## 🚀 Quick Start

### Option 1: VS Code Extension (Easiest)

1. Download [latest `.vsix` from Releases](https://github.com/Gad9889/STM32-Platform/releases)
2. Install: Extensions → `...` → **Install from VSIX...**
3. Open your STM32 CMake project
4. Run command: **STM32 Platform: Integrate into Project**
5. Select peripherals → Done!

See [Installation Guide](https://github.com/Gad9889/STM32-Platform/wiki/Installation) for details.

### Option 2: Manual Integration

```bash
# 1. Clone
git clone https://github.com/Gad9889/STM32-Platform.git

# 2. Copy files to your project
cp -r STM32-Platform/Inc/* YourProject/Inc/
cp -r STM32-Platform/Src/* YourProject/Src/

# 3. Add to CMakeLists.txt
include_directories(Inc)
add_executable(${PROJECT_NAME} ... Src/stm32_platform.c ...)
```

### Your First Program

```c
#include "stm32_platform.h"

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_CAN1_Init();  // CubeMX generated
    MX_UART2_Init();

    // Initialize platform
    Platform.begin(&hcan1, &huart2, NULL, NULL, NULL);

    P_UART.println("Platform ready!");

    while (1) {
        // Handle incoming CAN messages
        P_CAN.handleRxMessages();

        // Send a message
        uint8_t data[] = {0x01, 0x02, 0x03};
        P_CAN.send(0x123, data, 3);

        HAL_Delay(100);
    }
}
```

**[→ View Complete Examples](examples/)**

---

## 📚 Documentation

All documentation is now in the **[Wiki](https://github.com/Gad9889/STM32-Platform/wiki)**:

- **[Getting Started](https://github.com/Gad9889/STM32-Platform/wiki/Getting-Started)** - Installation and first program
- **[API Reference](https://github.com/Gad9889/STM32-Platform/wiki/API-Reference)** - Complete API documentation
- **[Architecture](https://github.com/Gad9889/STM32-Platform/wiki/Architecture)** - System design and internals
- **[Examples](https://github.com/Gad9889/STM32-Platform/wiki/Examples)** - Code examples and patterns
- **[Migration Guide](https://github.com/Gad9889/STM32-Platform/wiki/Migration-Guide)** - Upgrading from legacy API
- **[VS Code Extension](https://github.com/Gad9889/STM32-Platform/wiki/VS-Code-Extension)** - Extension usage guide

## 🎯 Supported Peripherals

| Peripheral | New API                                        | Legacy API          | Features                          |
| ---------- | ---------------------------------------------- | ------------------- | --------------------------------- |
| **CAN**    | `P_CAN.send()`, `P_CAN.handleRxMessages()`     | `plt_CanSendMsg()`  | Multi-channel, filtering, routing |
| **UART**   | `P_UART.println()`, `P_UART.printf()`          | `plt_UartSendMsg()` | DMA, printf redirection           |
| **SPI**    | `P_SPI.transfer()`, `P_SPI.transferByte()`     | `plt_SpiSendMsg()`  | Full-duplex DMA                   |
| **ADC**    | `P_ADC.readRaw()`, `P_ADC.readVoltage()`       | `plt_AdcInit()`     | Multi-channel, circular DMA       |
| **PWM**    | `P_PWM.setDutyCycle()`, `P_PWM.setFrequency()` | `plt_TimInit()`     | Dynamic frequency calculation     |

---

## 📦 Project Structure

```
STM32-Platform/
├── Inc/                        # Headers
│   ├── stm32_platform.h       # ⭐ New consumer-grade API (P_* objects)
│   ├── platform.h             # Legacy API (plt_* functions)
│   ├── platform_status.h      # Error codes and status types
│   └── [can|uart|spi|adc|tim].h
├── Src/                        # Implementation
│   ├── stm32_platform.c       # ⭐ New API implementation
│   └── [peripheral].c         # Core drivers
├── vscode-extension/          # VS Code integration
├── test-kit/                  # Application testing starter kit
├── tests/                     # Platform unit tests (Unity)
└── .github/workflows/         # CI/CD automation
```

## 🎓 Code Examples

### CAN Communication

```c
#include "stm32_platform.h"

void can_handler(CANMessage_t* msg) {
    P_UART.printf("CAN ID: 0x%03X\n", msg->id);
}

int main(void) {
    Platform.begin(&hcan1, &huart2, NULL, NULL, NULL)
            ->onCAN(can_handler);

    while (1) {
        P_CAN.handleRxMessages();

        if (button_pressed) {
            uint8_t data[] = {0xAA, 0xBB};
            P_CAN.send(0x100, data, 2);
        }
    }
}

        // Store in database
        pMainDB->vcu_node->inverters[0].actual_speed = rpm;
        pMainDB->vcu_node->inverters[0].torque = torque;
    }
}

// Send CAN message
can_message_t tx_msg = {
    .id = 0x456,
    .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
};
plt_CanSendMsg(Can1, &tx_msg);

// In main loop
while(1) {
    plt_CanProcessRxMsgs();  // Process incoming messages
    HAL_Delay(10);
}
```

### Example 2: UART Debug Logging

```c
#include "callbacks.h"

// Initialize UART for printf
handler_set_t handlers = {
    .huart2 = &huart2  // Debug UART
};
PlatformInit(&handlers, 32);

// Now printf works automatically!
printf("System initialized! Voltage: %d.%dV\r\n", voltage/10, voltage%10);

// Send custom debug messages
uint8_t debug_data[] = "Sensor readings OK";
plt_DebugSendMSG(debug_data, strlen((char*)debug_data));
```

### Example 3: ADC Sampling

```c
// ADC automatically samples in background via DMA
// Averaged values stored in database

// Configure 3 channels in CubeMX
handler_set_t handlers = {
    .hadc1 = &hadc1  // ADC with 3 channels
};
PlatformInit(&handlers, 32);

// Access averaged ADC values
void ReadSensors(void) {
    // ADC data automatically published to CAN queue
    // Process via CanRxCallback with Internal_ADC message ID
}
```

### Example 4: PWM Control

```c
#include "tim.h"

// Configure timer in CubeMX
handler_set_t handlers = {
    .htim2 = &htim2
};
PlatformInit(&handlers, 32);

// Generate 1kHz PWM at 50% duty cycle
plt_StartPWM(Tim2, TIM_CHANNEL_1, 1000, 50.0f);

// Change duty cycle dynamically
plt_StartPWM(Tim2, TIM_CHANNEL_1, 1000, 75.0f);

// Stop PWM
plt_StopPWM(Tim2, TIM_CHANNEL_1);
```

See [examples/](examples/) directory for complete projects.

---

## ⚙️ Configuration

### Compile-Time Configuration

The platform uses conditional compilation for optional features:

```c
// In your project's preprocessor defines or stm32xxxx_hal_conf.h
#define HAL_CAN_MODULE_ENABLED      // Enable CAN support
#define HAL_UART_MODULE_ENABLED     // Enable UART support
#define HAL_SPI_MODULE_ENABLED      // Enable SPI support
#define HAL_ADC_MODULE_ENABLED      // Enable ADC support
#define HAL_TIM_MODULE_ENABLED      // Enable Timer support
```

### Runtime Configuration

```c
// Adjust queue sizes based on your message rate
#define RX_QUEUE_SIZE 64  // Larger for high CAN traffic
#define TX_QUEUE_SIZE 32  // Smaller if you send less

// ADC configuration in adc.h
#define ADC1_NUM_SENSORS 3
#define ADC1_SAMPLES_PER_SENSOR 50  // Averaging factor
```

---

## 🧪 Testing

### Unit Tests

Tests are written using the [Unity Test Framework](http://www.throwtheswitch.org/unity).

```bash
# Build and run tests
cd tests
mkdir build && cd build
cmake ..
make
ctest
```

### Integration Tests

Run on actual hardware:

1. Flash example applications
2. Connect peripherals (CAN transceiver, UART adapter, etc.)
3. Monitor via debugger or serial terminal

---

## 🛠️ Requirements

- STM32 microcontroller (F0/F1/F4/F7/H7/G0/G4/L4)
- STM32CubeMX (for HAL configuration)
- ARM GCC toolchain
- CMake 3.15+ (optional, for library build)

## 🤝 Contributing

Contributions welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

- Fork the repository
- Create a feature branch
- Add tests for new features
- Submit a pull request

## 📄 License

MIT License - see [LICENSE](LICENSE) file.

## 🙏 Acknowledgments

Developed by **Ben Gurion Racing Team** for Formula Student electric vehicles.

Now available as a consumer-grade platform for the embedded community.

---

**Made with ❤️ by BGU Racing**
