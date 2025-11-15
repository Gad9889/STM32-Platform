# STM32 Platform

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CI](https://github.com/Gad9889/STM32-Platform/workflows/CI/badge.svg)](https://github.com/Gad9889/STM32-Platform/actions)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg)](CONTRIBUTING.md)

**Consumer-grade communication platform for STM32 microcontrollers.** Get CAN, UART, SPI, ADC, and PWM working in minutes with an intuitive, Arduino-style API.

Born from Ben-Gurion Racing's Formula Student team, now redesigned for developers who want **simplicity without sacrificing power**.

## ✨ Features

- **🎯 Consumer-Grade API**: Arduino-style interface - `P_CAN.send()`, `P_UART.println()`, `P_ADC.readVoltage()`
- **⚡ Type-Agnostic Integration**: Works with any HAL module combination - disable peripherals you don't need
- **🔧 Direct HAL Integration**: No middleware layer - 700+ lines of optimized code directly calls STM32 HAL
- **🛡️ Production-Ready**: Used in Formula Student racing, standardized error handling
- **🧪 Well-Tested**: 43 unit tests, 100% pass rate, CI/CD with GitHub Actions
- **📦 Thread-Safe**: ISR-safe queues with critical sections for reliable message handling
- **🚀 Zero Magic**: Explicit control - you decide when to process messages with `.handleRxMessages()`

## 🚀 API Overview

**v2.0.0 Consumer-Grade API** (Arduino-style with P_ prefix):

```c
// Initialize with only the peripherals you need
PlatformHandles_t handles = {
    .hcan = &hcan,
    .huart = &huart2,
    .hspi = NULL,        // Not using SPI? Set NULL
    .hadc = NULL,        // Not using ADC? Set NULL
    .htim = &htim1
};
Platform.begin(&handles);

// Use intuitive API
P_CAN.send(0x123, data, 8);
P_UART.printf("Speed: %d km/h\n", speed);
P_ADC.readVoltage(ADC_CHANNEL_1);
P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 50.0f);
```

---

## 🚀 Quick Start

### Option 1: Manual Integration (Recommended)

**📖 See [CUBEMX_INTEGRATION.md](CUBEMX_INTEGRATION.md) for complete step-by-step guide**

1. Copy platform files to your STM32CubeMX project:
   - `Inc/*.h` → `YourProject/Core/Inc/`
   - `Src/*.c` → `YourProject/Core/Src/`

2. Configure peripherals in CubeMX (enable only what you need)

3. Initialize in main.c (see example below)

### Option 2: Clone Repository

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

    // Initialize platform with peripheral handles
    PlatformHandles_t handles = {
        .hcan = &hcan1,
        .huart = &huart2,
        .hspi = NULL,   // Not using SPI
        .hadc = NULL,   // Not using ADC
        .htim = NULL    // Not using PWM
    };
    Platform.begin(&handles);

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

| Peripheral | API Examples                                   | Features                          |
| ---------- | ---------------------------------------------- | --------------------------------- |
| **CAN**    | `P_CAN.send()`, `P_CAN.handleRxMessages()`, `P_CAN.route()` | Hashtable routing, thread-safe queues |
| **UART**   | `P_UART.println()`, `P_UART.printf()`          | DMA, printf redirection, byte reception |
| **SPI**    | `P_SPI.transfer()`, `P_SPI.transferByte()`     | Full-duplex, configurable CS      |
| **ADC**    | `P_ADC.readRaw()`, `P_ADC.readVoltage()`       | Multi-channel, voltage conversion |
| **PWM**    | `P_PWM.setDutyCycle()`, `P_PWM.setFrequency()` | Dynamic frequency, start/stop     |

---

## 📦 Project Structure

```
STM32-Platform/
├── Inc/                        # Headers
│   ├── stm32_platform.h       # ⭐ Consumer-grade API (Platform, P_CAN, P_UART, etc.)
│   ├── platform_status.h      # Status codes and error handling
│   ├── hashtable.h            # CAN message routing (O(1) lookup)
│   ├── database.h             # Signal storage and management
│   ├── utils.h                # Queue implementation
│   └── DbSetFunctions.h       # Database setter functions
├── Src/                        # Implementation (700+ lines)
│   ├── stm32_platform.c       # ⭐ Direct HAL integration, thread-safe queues
│   ├── platform_status.c      # Status code utilities
│   ├── hashtable.c            # CAN routing implementation
│   ├── database.c             # Database implementation
│   ├── utils.c                # Queue + critical sections
│   └── DbSetFunctions.c       # Generated database setters
├── tests/                     # Unity unit tests (100% pass - 43 tests)
├── .github/workflows/         # CI/CD automation
├── CUBEMX_INTEGRATION.md      # Step-by-step integration guide
└── PLATFORM_INTEGRATION_ISSUES.md  # Real-world testing results
```

## 🎓 Code Examples

### CAN Communication

```c
#include "stm32_platform.h"

void can_handler(CANMessage_t* msg) {
    P_UART.printf("CAN ID: 0x%03X\n", msg->id);
}

int main(void) {
    // HAL init, clock config, peripheral init...
    
    PlatformHandles_t handles = {
        .hcan = &hcan1,
        .huart = &huart2,
        .hspi = NULL,
        .hadc = NULL,
        .htim = NULL
    };
    Platform.begin(&handles);
    
    // Register handler for specific CAN ID
    P_CAN.route(0x100, can_handler);

    while (1) {
        P_CAN.handleRxMessages();

        if (button_pressed) {
            uint8_t data[] = {0xAA, 0xBB};
            P_CAN.send(0x100, data, 2);
        }
    }
}


### UART Debug Logging

```c
#include "stm32_platform.h"

int main(void) {
    // HAL init, clock config, peripheral init...
    
    PlatformHandles_t handles = {
        .hcan = NULL,
        .huart = &huart2,  // Debug UART
        .hspi = NULL,
        .hadc = NULL,
        .htim = NULL
    };
    Platform.begin(&handles);

    // Now printf works automatically!
    P_UART.printf("System initialized! Voltage: %d.%dV\r\n", voltage/10, voltage%10);
    P_UART.println("Sensor readings OK");
}
```

### ADC Sampling

```c
#include "stm32_platform.h"

int main(void) {
    // HAL init, clock config, peripheral init...
    
    PlatformHandles_t handles = {
        .hcan = NULL,
        .huart = &huart2,
        .hspi = NULL,
        .hadc = &hadc1,  // ADC with channels configured
        .htim = NULL
    };
    Platform.begin(&handles);

    while (1) {
        // Read ADC voltages
        float voltage = P_ADC.readVoltage(ADC_CHANNEL_1);
        P_UART.printf("Voltage: %.2fV\r\n", voltage);
        HAL_Delay(100);
    }
}
```

### PWM Control

```c
#include "stm32_platform.h"

int main(void) {
    // HAL init, clock config, peripheral init...
    
    PlatformHandles_t handles = {
        .hcan = NULL,
        .huart = NULL,
        .hspi = NULL,
        .hadc = NULL,
        .htim = &htim2   // PWM timer
    };
    Platform.begin(&handles);

    // Start PWM at 1kHz, 50% duty cycle
    P_PWM.start(&htim2, TIM_CHANNEL_1);
    P_PWM.setFrequency(&htim2, TIM_CHANNEL_1, 1000);  // 1kHz
    P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 50.0f); // 50%

    // Change duty cycle dynamically
    P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 75.0f);

    // Stop PWM
    P_PWM.stop(&htim2, TIM_CHANNEL_1);
}
```

**\ud83d\udcd6 See [examples/](examples/) directory for complete projects and [CUBEMX_INTEGRATION.md](CUBEMX_INTEGRATION.md) for integration guide.**

---

## ⚙️ Configuration

### Peripheral Configuration

Configure peripherals in STM32CubeMX, then pass handles to the platform:

```c
PlatformHandles_t handles = {
    .hcan = &hcan,        // NULL if not using CAN
    .huart = &huart2,     // NULL if not using UART
    .hspi = &hspi1,       // NULL if not using SPI
    .hadc = &hadc1,       // NULL if not using ADC
    .htim = &htim2        // NULL if not using PWM
};
Platform.begin(&handles);
```

**Key Feature:** Only peripherals you enable in CubeMX will be compiled. Set unused handles to NULL.

### Queue Sizes

Adjust in `Inc/utils.h` based on your message rate:

```c
#define QUEUE_SIZE 64  // Larger for high CAN traffic
```

### ADC Reference Voltage

Default is 3.3V. Modify in `stm32_platform.c` if different:

```c
adc_state.vref = 3.3f;  // Your VREF voltage
```

---

## 🧪 Testing

### Unit Tests

Platform includes comprehensive unit tests using the [Unity Test Framework](http://www.throwtheswitch.org/unity).

**Test Results:** ✅ 100% pass rate (43 tests)

```bash
# Build and run tests
cd tests
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

**Test Suites:**
- `test_utils.c` - Queue operations (14 tests)
- `test_database.c` - Signal storage (13 tests)
- `test_hashtable.c` - CAN routing (16 tests)

### Integration Tests

Real-world integration testing performed with STM32F303 demo project. See `PLATFORM_INTEGRATION_ISSUES.md` for results.

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
