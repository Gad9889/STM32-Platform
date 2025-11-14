# STM32 Platform VS Code Extension

Seamlessly integrate the BGU Racing STM32 Platform into your STM32 CMake projects with one command.

## Features

- **One-Click Integration**: Automatically detects STM32 CMake projects and integrates the platform
- **Smart Detection**: Identifies CubeMX-generated projects and existing STM32 HAL setups
- **Selective Peripherals**: Choose which peripherals to enable (CAN, UART, SPI, ADC, TIM)
- **Modern API**: Option to use the new consumer-grade API (`P_CAN.send()`) or legacy API (`plt_CanSendMsg()`)
- **Example Generation**: Creates working example code tailored to your peripheral selection

## Usage

### Quick Start

1. Open an STM32 CMake project in VS Code
2. Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
3. Type "STM32 Platform: Integrate into Project"
4. Select the peripherals you want to use
5. Done! The platform is integrated and ready to use

### Auto-Detection

The extension automatically detects STM32 projects when you open a workspace and prompts you to integrate the platform.

### Commands

- **STM32 Platform: Integrate into Project** - Integrate platform into current project
- **STM32 Platform: Check Project Compatibility** - Check if current project is compatible

## Configuration

| Setting                            | Description                                               | Default           |
| ---------------------------------- | --------------------------------------------------------- | ----------------- |
| `stm32platform.autoDetect`         | Automatically detect STM32 projects and offer integration | `true`            |
| `stm32platform.defaultPeripherals` | Default peripherals to enable during integration          | `["CAN", "UART"]` |
| `stm32platform.useNewAPI`          | Use new consumer-grade API instead of legacy plt\_\* API  | `true`            |

## What Gets Integrated

When you integrate the platform, the extension:

1. **Copies platform files** to your `Inc/` and `Src/` directories:

   - Core: `platform.h/c`, `platform_status.h`, `platform_config.h/c`, `utils.h/c`
   - Selected peripherals: `can.h/c`, `uart.h/c`, `spi.h/c`, etc.
   - New API: `stm32_platform.h/c` (with P_CAN, P_UART, etc.)
   - CAN support: Auto-generates `DbSetFunctions.h/c` stub

2. **Updates CMakeLists.txt**:

   - Adds platform source files
   - Adds include directories
   - Sets compile definitions for enabled peripherals

3. **Creates example code** (`platform_example.c`):
   - Initialization function
   - Main loop with peripheral handling
   - Ready to copy into your `main.c`

4. **Optional: Testing Starter Kit** (if selected):
   - Unity test framework setup
   - Example test template
   - GitHub Actions workflow
   - For testing YOUR application logic (not platform)

## Example: New API

```c
#include "stm32_platform.h"

void app_init(void) {
    Platform.begin(&hcan1, &huart2, &hspi1, NULL, NULL)
            ->onCAN(my_can_handler);

    P_UART.println("Platform initialized!");
}

void app_loop(void) {
    P_CAN.handleRxMessages();

    if (P_CAN.availableMessages() > 0) {
        uint8_t data[] = {0x01, 0x02, 0x03};
        P_CAN.send(0x123, data, 3);
    }

    uint16_t temp = P_ADC.readRaw(0);
    P_UART.printf("Temperature: %u\n", temp);
}
```

## Requirements

- VS Code 1.85.0 or higher
- STM32 project with CMake build system
- STM32 HAL drivers

## Compatibility

The extension works with:

- CubeMX-generated projects
- Manual STM32 CMake projects
- STM32 families: F0, F1, F4, F7, H7, G0, G4, L4, etc.

## License

MIT - See LICENSE file in the main repository
