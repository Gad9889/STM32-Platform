# CubeMX Integration Guide

## Integration Methods

### Method 1: Import Template Project (Recommended)

Platform provides `.ioc` template files for common STM32 families.

**Templates Available:**

- `templates/STM32F4_Platform_Template.ioc` - F4 family baseline
- `templates/STM32F1_Platform_Template.ioc` - F1 family baseline
- `templates/STM32H7_Platform_Template.ioc` - H7 family baseline

**Usage:**

1. Copy template to project directory
2. Open in STM32CubeMX
3. Adjust peripherals for specific requirements
4. Generate code
5. Platform files automatically integrated via user code sections

**Template Configuration:**

- CAN1: 500 kbit/s, auto-retransmission enabled
- UART1/2: 115200 baud, DMA enabled (TX: Normal, RX: Circular)
- SPI1: Master mode, 8-bit, DMA enabled
- ADC1: 3 channels, DMA circular mode
- TIM2/3: PWM capable, prescaler calculated at runtime

### Method 2: CubeMX Package (Middleware)

**Status:** Under development

Platform can be packaged as CubeMX middleware (Software Pack).

**Requirements:**

- Create `.pack` file following CMSIS-Pack standard
- Define component selection (CAN, UART, SPI, etc.)
- Provide initialization templates
- Include documentation and examples

**Distribution:**

- Host on GitHub releases
- Users add via CubeMX Package Manager
- Automatic code generation integration

**Timeline:** Q1 2026

### Method 3: Manual Integration (Current)

Standard approach documented in [integration_guide.md](integration_guide.md).

1. Generate base project with CubeMX
2. Copy platform files manually
3. Configure build system
4. Initialize in user code sections

## CubeMX Configuration Requirements

### Critical Settings

**DMA Configuration:**

- UART RX: Mode=Circular, Priority=High
- UART TX: Mode=Normal, Priority=Medium
- SPI TX/RX: Mode=Normal
- ADC: Mode=Circular, Data Width=Half Word

**Interrupt Priority:**

- DMA interrupts: Preempt=5, Sub=0
- Peripheral interrupts: Preempt=6, Sub=0
- SysTick: Preempt=15, Sub=0

**Clock Configuration:**

- APB1: 42 MHz (for CAN timing)
- APB2: 84 MHz
- System: 168 MHz (F4), adjust per family

### Code Generation Settings

**Project Manager → Code Generator:**

- Generate peripheral initialization as .c/.h pairs: **ENABLED**
- Keep user code when re-generating: **ENABLED**
- Delete previously generated files: **DISABLED**

**Project Manager → Advanced Settings:**

- Register Callback: **DISABLED** (platform uses HAL weak callbacks)
- Use HAL Library: **ENABLED**

## User Code Sections

Platform integrates via CubeMX user code sections:

```c
/* USER CODE BEGIN Includes */
#include "callbacks.h"
#include "platform_status.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 2 */
handler_set_t handlers = {
    .hcan1 = &hcan1,
    .huart2 = &huart2,
    // ... other peripherals
};
PlatformInit(&handlers, 64);
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1) {
    plt_CanProcessRxMsgs();
    plt_UartProcessRxMsgs();
    plt_SpiProcessRxMsgs();
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */
```

## Validation Script

Platform includes validation script to check CubeMX configuration:

```bash
python scripts/validate_cubemx_config.py project.ioc
```

Verifies:

- DMA configuration matches requirements
- Clock speeds within acceptable range
- Required peripherals enabled
- Interrupt priorities configured correctly

## Future: VS Code Extension

**Planned features:**

- One-click platform integration
- Automatic CubeMX configuration
- Build system generation
- IntelliSense integration

**Status:** Roadmap for v2.5

---

## References

- [Integration Guide](integration_guide.md)
- [Architecture](architecture.md)
- STM32CubeMX User Manual (UM1718)
- CMSIS-Pack Specification
