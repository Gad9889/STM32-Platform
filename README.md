# STM32 Platform

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CI](https://github.com/Gad9889/STM32-Platform/workflows/CI/badge.svg)](https://github.com/Gad9889/STM32-Platform/actions)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg)](CONTRIBUTING.md)

**Hardened communication platform for STM32 microcontrollers.** Direct HAL integration with thread-safe queues for CAN, UART, SPI, ADC, and PWM peripherals.

Originated from Ben-Gurion Racing Formula Student team. Battle-tested in high-performance motorsport applications.

## Core Capabilities

- **Deterministic API**: Direct peripheral control - `P_CAN.send()`, `P_UART.println()`, `P_ADC.readVoltage()`
- **Conditional Compilation**: Type-agnostic integration - compile only required peripheral modules
- **Zero Abstraction Overhead**: No middleware layer - direct HAL function calls
- **Field-Proven**: Validated in Formula Student racing operations
- **Fully Tested**: 43 unit tests, 100% pass rate, automated CI/CD pipeline
- **ISR-Safe**: Thread-safe queues with critical section protection
- **Explicit Control**: Manual message processing - operator decides execution timing

## System Architecture

**v2.1.0 Multi-Instance API** (P\_ namespace):

```c
// Initialize with multiple peripheral instances
void* cans[] = {&hcan1, &hcan2};
void* uarts[] = {&huart1, &huart2, &huart3};
void* tims[] = {&htim1};

PlatformHandles_t handles = {
    .hcan = cans,
    .can_count = 2,
    .huart = uarts,
    .uart_count = 3,
    .hspi = NULL,
    .spi_count = 0,
    .hadc = NULL,
    .adc_count = 0,
    .htim = tims,
    .tim_count = 1
};
Platform.begin(&handles);

// Execute operations with instance index
P_CAN.send(0, 0x123, data, 8);      // CAN1
P_CAN.send(1, 0x456, data, 8);      // CAN2
P_UART.printf(0, "ECU: %d\n", val); // UART1
P_UART.printf(1, "DAQ: %d\n", val); // UART2
P_PWM.setDutyCycle(0, TIM_CHANNEL_1, 50.0f);
```

---

## Deployment Instructions

### Method 1: Manual Integration

1. Copy platform files to STM32CubeMX project:

   - `Inc/*.h` → `YourProject/Core/Inc/`
   - `Src/*.c` → `YourProject/Core/Src/`

2. Configure peripherals in CubeMX (enable required modules only)

3. Initialize in main.c (see operational example below)

### Method 2: Repository Clone

```bash
# Clone repository
git clone https://github.com/Gad9889/STM32-Platform.git

# Deploy files to target project
cp -r STM32-Platform/Inc/* YourProject/Inc/
cp -r STM32-Platform/Src/* YourProject/Src/

# Configure build system
include_directories(Inc)
add_executable(${PROJECT_NAME} ... Src/stm32_platform.c ...)
```

### Operational Example

```c
#include "stm32_platform.h"

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_CAN1_Init();
    MX_UART2_Init();

    // Initialize platform with peripheral handles
    void* cans[] = {&hcan1};
    void* uarts[] = {&huart2};
    
    PlatformHandles_t handles = {
        .hcan = cans,
        .can_count = 1,
        .huart = uarts,
        .uart_count = 1,
        .hspi = NULL,
        .spi_count = 0,
        .hadc = NULL,
        .adc_count = 0,
        .htim = NULL,
        .tim_count = 0
    };
    Platform.begin(&handles);

    P_UART.println(0, "Platform operational");

    while (1) {
        // Process received CAN messages (instance 0)
        P_CAN.handleRxMessages(0);

        // Transmit message on CAN instance 0
        uint8_t data[] = {0x01, 0x02, 0x03};
        P_CAN.send(0, 0x123, data, 3);

        HAL_Delay(100);
    }
}
```

---

## Technical Documentation

Complete technical specifications available in the **[Wiki](https://github.com/Gad9889/STM32-Platform/wiki)**:

- **[Getting Started](https://github.com/Gad9889/STM32-Platform/wiki/Getting-Started)** - Installation and initialization procedures
- **[API Reference](https://github.com/Gad9889/STM32-Platform/wiki/API-Reference)** - Complete API specification
- **[Architecture](https://github.com/Gad9889/STM32-Platform/wiki/Architecture)** - System design documentation
- **[Examples](https://github.com/Gad9889/STM32-Platform/wiki/Examples)** - Implementation patterns
- **[Migration Guide](https://github.com/Gad9889/STM32-Platform/wiki/Migration-Guide)** - Legacy API migration procedures

## Supported Peripherals

| Peripheral | API Functions                                               | Capabilities                          |
| ---------- | ----------------------------------------------------------- | ------------------------------------- |
| **CAN**    | `P_CAN.send()`, `P_CAN.handleRxMessages()`, `P_CAN.route()` | Hashtable routing, thread-safe queues |
| **UART**   | `P_UART.println()`, `P_UART.printf()`                       | DMA support, printf redirection       |
| **SPI**    | `P_SPI.transfer()`, `P_SPI.transferByte()`                  | Full-duplex, configurable CS          |
| **ADC**    | `P_ADC.readRaw()`, `P_ADC.readVoltage()`                    | Multi-channel, voltage conversion     |
| **PWM**    | `P_PWM.setDutyCycle()`, `P_PWM.setFrequency()`              | Dynamic frequency, start/stop control |

---

## Repository Structure

```
STM32-Platform/
├── Inc/                        # Header files
│   ├── stm32_platform.h       # Main API interface (Platform, P_CAN, P_UART, etc.)
│   ├── platform_status.h      # Status codes and error handling
│   ├── hashtable.h            # CAN message routing (O(1) lookup)
│   ├── database.h             # Signal storage system
│   ├── utils.h                # Queue implementation
│   └── DbSetFunctions.h       # Database setter functions
├── Src/                        # Implementation (700+ lines)
│   ├── stm32_platform.c       # Direct HAL integration, thread-safe queues
│   ├── platform_status.c      # Status utilities
│   ├── hashtable.c            # CAN routing implementation
│   ├── database.c             # Database implementation
│   ├── utils.c                # Queue + critical sections
│   └── DbSetFunctions.c       # Generated database setters
├── tests/                     # Unity unit tests (43 tests, 100% pass)
├── .github/workflows/         # CI/CD automation
└── CHANGELOG.md               # Version history
```

## Implementation Examples

### CAN Communication

```c
#include "stm32_platform.h"

void can_message_handler(CANMessage_t* msg) {
    P_UART.printf(0, "CAN ID: 0x%03X\n", msg->id);
}

int main(void) {
    // HAL initialization, clock configuration, peripheral initialization

    void* cans[] = {&hcan1};
    void* uarts[] = {&huart2};
    
    PlatformHandles_t handles = {
        .hcan = cans,
        .can_count = 1,
        .huart = uarts,
        .uart_count = 1,
        .hspi = NULL,
        .spi_count = 0,
        .hadc = NULL,
        .adc_count = 0,
        .htim = NULL,
        .tim_count = 0
    };
    Platform.begin(&handles);

    // Register handler for specific CAN ID on instance 0
    P_CAN.route(0, 0x100, can_message_handler);

    while (1) {
        // Process CAN instance 0 messages
        P_CAN.handleRxMessages(0);

        if (button_pressed) {
            uint8_t data[] = {0xAA, 0xBB};
            P_CAN.send(0, 0x100, data, 2);
        }
    }
}
```

### UART Data Transmission

```c
#include "stm32_platform.h"

int main(void) {
    // HAL initialization, clock configuration, peripheral initialization

    PlatformHandles_t handles = {
        .hcan = NULL,
        .huart = &huart2,
        .hspi = NULL,
        .hadc = NULL,
        .htim = NULL
    };
    Platform.begin(&handles);

    // Printf functionality operational
    P_UART.printf("System initialized. Voltage: %d.%dV\r\n", voltage/10, voltage%10);
    P_UART.println("Sensor readings nominal");
}
```

### ADC Acquisition

```c
#include "stm32_platform.h"

int main(void) {
    // HAL initialization, clock configuration, peripheral initialization

    PlatformHandles_t handles = {
        .hcan = NULL,
        .huart = &huart2,
        .hspi = NULL,
        .hadc = &hadc1,
        .htim = NULL
    };
    Platform.begin(&handles);

    while (1) {
        // Acquire ADC voltage
        float voltage = P_ADC.readVoltage(ADC_CHANNEL_1);
        P_UART.printf("Voltage: %.2fV\r\n", voltage);
        HAL_Delay(100);
    }
}
```

### PWM Generation

```c
#include "stm32_platform.h"

int main(void) {
    // HAL initialization, clock configuration, peripheral initialization

    PlatformHandles_t handles = {
        .hcan = NULL,
        .huart = NULL,
        .hspi = NULL,
        .hadc = NULL,
        .htim = &htim2
    };
    Platform.begin(&handles);

    // Initialize PWM at 1kHz, 50% duty cycle
    P_PWM.start(&htim2, TIM_CHANNEL_1);
    P_PWM.setFrequency(&htim2, 1000);
    P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 50.0f);

    // Modify duty cycle during operation
    P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 75.0f);

    // Terminate PWM generation
    P_PWM.stop(&htim2, TIM_CHANNEL_1);
}
```

---

## Configuration Parameters

### Peripheral Configuration

Configure peripherals in STM32CubeMX, then provide handles to platform:

```c
PlatformHandles_t handles = {
    .hcan = &hcan,        // NULL if CAN disabled
    .huart = &huart2,     // NULL if UART disabled
    .hspi = &hspi1,       // NULL if SPI disabled
    .hadc = &hadc1,       // NULL if ADC disabled
    .htim = &htim2        // NULL if PWM disabled
};
Platform.begin(&handles);
```

**Conditional Compilation:** Only peripherals enabled in CubeMX will be compiled. Set unused handles to NULL.

### Queue Sizing

Configure in `Inc/utils.h` based on message throughput requirements:

```c
#define QUEUE_SIZE 64  // Increase for high-traffic CAN networks
```

### ADC Reference Voltage

Default configuration is 3.3V. Modify in `stm32_platform.c` if different:

```c
adc_state.vref = 3.3f;  // Set to actual VREF voltage
```

---

## Test Suite

### Unit Testing

Platform includes comprehensive unit tests using [Unity Test Framework](http://www.throwtheswitch.org/unity).

**Test Status:** 100% pass rate (43 tests)

```bash
# Execute test suite
cd tests
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

**Test Modules:**

- `test_utils.c` - Queue operations (14 tests)
- `test_database.c` - Signal storage (13 tests)
- `test_hashtable.c` - CAN routing (16 tests)

### Integration Validation

Real-world integration testing performed with STM32F303 demonstration project.

---

## System Requirements

- STM32 microcontroller (F0/F1/F4/F7/H7/G0/G4/L4 series)
- STM32CubeMX (for HAL peripheral configuration)
- ARM GCC toolchain
- CMake 3.15+ (optional, for library build)

## Contributing

Contributions accepted. See [CONTRIBUTING.md](CONTRIBUTING.md) for procedures.

- Fork repository
- Create feature branch
- Add tests for new functionality
- Submit pull request

## License

MIT License - see [LICENSE](LICENSE) file.

## Origin

Developed by **Ben Gurion Racing Team** for Formula Student electric vehicle applications.

Deployed as platform for embedded systems development.

---

**Ben Gurion Racing - Formula Student Electric**
