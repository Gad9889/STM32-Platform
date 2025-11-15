# STM32 Platform VS Code Extension

Automated integration tool for deploying BGU Racing STM32 Platform into STM32 CMake projects.

## Capabilities

- **Automated Integration**: Detects STM32 CMake projects and deploys platform components
- **Project Detection**: Identifies CubeMX-generated projects and STM32 HAL configurations
- **Peripheral Selection**: Configure specific peripherals (CAN, UART, SPI, ADC, TIM)
- **Direct API**: Deploys v2.0.0 API with direct HAL integration
- **Code Generation**: Produces initialization templates for selected peripherals

## Operation

### Deployment Procedure

1. Open STM32 CMake project in VS Code
2. Execute `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
3. Enter "STM32 Platform: Integrate into Project"
4. Select required peripherals
5. Integration complete - platform operational

### Automatic Detection

Extension automatically scans workspace for STM32 projects and initiates integration protocol.

### Available Commands

- **STM32 Platform: Integrate into Project** - Deploy platform to current project
- **STM32 Platform: Check Project Compatibility** - Verify project compatibility

## Configuration Parameters

| Parameter                          | Function                                   | Default           |
| ---------------------------------- | ------------------------------------------ | ----------------- |
| `stm32platform.autoDetect`         | Enable automatic STM32 project detection   | `true`            |
| `stm32platform.defaultPeripherals` | Default peripheral modules for integration | `["CAN", "UART"]` |
| `stm32platform.useNewAPI`          | Deploy v2.0.0 direct API                   | `true`            |

## Integration Components

Platform integration deploys:

1. **Platform Files** to `Core/Inc/` and `Core/Src/` directories:

   - Core modules: `stm32_platform.h/c`, `platform_status.h/c`, `utils.h/c`
   - Peripheral modules: Selected from CAN, UART, SPI, ADC, PWM
   - Support modules: `hashtable.h/c`, `database.h/c`, `DbSetFunctions.h/c`

2. **Build Configuration Updates** (`CMakeLists.txt`):

   - Adds platform source files to build
   - Configures include directories
   - Sets peripheral compile definitions

3. **Initialization Template** (`platform_example.c`):

   - Platform initialization code
   - Peripheral configuration examples
   - Ready for deployment to main.c USER CODE sections

4. **Test Framework** (optional):
   - Unity test framework configuration
   - Test template structure
   - CI/CD workflow template

## Implementation Example: v2.0.0 API

```c
#include "stm32_platform.h"

void system_initialize(void) {
    PlatformHandles_t handles = {
        .hcan = &hcan1,
        .huart = &huart2,
        .hspi = &hspi1,
        .hadc = NULL,
        .htim = NULL
    };
    Platform.begin(&handles);

    P_CAN.route(0x100, can_message_handler);
    P_UART.println("Platform operational");
}

void system_loop(void) {
    P_CAN.handleRxMessages();

    uint8_t data[] = {0x01, 0x02, 0x03};
    P_CAN.send(0x123, data, 3);

    float voltage = P_ADC.readVoltage(ADC_CHANNEL_1);
    P_UART.printf("Voltage: %.2fV\n", voltage);
}
```

## System Requirements

- Visual Studio Code 1.60.0 or higher
- STM32 project with CMake build system
- STM32CubeMX-generated HAL configuration (recommended)
- ARM GCC toolchain

## Supported STM32 Families

All STM32 families supported by STM32CubeMX:

- STM32F0/F1/F2/F3/F4/F7
- STM32H7
- STM32G0/G4
- STM32L0/L1/L4/L5
- STM32WB/WL
- STM32U5

## Installation

1. Download `.vsix` file from [Releases](https://github.com/Gad9889/STM32-Platform/releases)
2. Open VS Code
3. Navigate to Extensions view (`Ctrl+Shift+X`)
4. Click "..." menu → "Install from VSIX..."
5. Select downloaded `.vsix` file

Or install from source:

```bash
cd vscode-extension
npm install
npm run compile
# Press F5 in VS Code to launch extension development host
```

## Operation Notes

**CubeMX Integration**: Extension inserts code into USER CODE sections to prevent deletion during CubeMX code regeneration.

**Peripheral Configuration**: Configure peripherals in STM32CubeMX before integration. Extension generates initialization code for enabled peripherals.

**Build System**: Extension requires CMake-based build system. For Makefile projects, manual integration required.

## Technical Support

Report issues: [GitHub Issues](https://github.com/Gad9889/STM32-Platform/issues)

Documentation: [Platform Wiki](https://github.com/Gad9889/STM32-Platform/wiki)

## License

MIT License - see [LICENSE](../LICENSE) file.

## Origin

Developed by **Ben Gurion Racing Team** for Formula Student electric vehicle applications.
