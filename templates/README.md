# STM32CubeMX Template Projects

## Overview

Pre-configured CubeMX projects optimized for STM32 Platform integration.

---

## Available Templates

### STM32F446RE (Nucleo-64)

**File:** `STM32F446RE_Platform_Template.ioc`

**Configuration:**

- **CAN1:** 500 kbit/s, auto-retransmit enabled, FIFO0/FIFO1 filters configured
- **UART2:** 115200 baud, DMA TX (Normal), DMA RX (Circular)
- **SPI1:** Master mode, 8-bit, full-duplex, DMA TX/RX
- **ADC1:** 3 channels, DMA circular mode, 12-bit resolution
- **TIM2/TIM3:** PWM capable, channels 1-4 enabled
- **Clock:** 168 MHz system, 42 MHz APB1, 84 MHz APB2
- **DMA:** Priorities and modes optimized for platform
- **GPIO:** Platform-compatible pin assignments

**User Code Sections:** Pre-populated with platform initialization code

---

## Quick Start

### 1. Download Template

```bash
# Clone repository
git clone https://github.com/Ben-Gurion-Racing/STM32_Platform.git
cd STM32_Platform/templates
```

### 2. Open in CubeMX

1. Launch STM32CubeMX
2. **File → Load Project**
3. Select template for your MCU family
4. Adjust peripherals as needed
5. **Project → Generate Code**

### 3. Copy Platform Files

```bash
# Copy platform to generated project
cp -r ../Inc ../Src <your_project>/Drivers/STM32_Platform/
```

### 4. Update Build System

**VS Code + CMake:**

Add to `CMakeLists.txt`:

```cmake
add_subdirectory(Drivers/STM32_Platform)
target_link_libraries(${PROJECT_NAME} PRIVATE STM32Platform::platform)
```

**Makefile:**

Add to `Makefile`:

```makefile
# Platform sources
C_SOURCES += \
Drivers/STM32_Platform/Src/platform.c \
Drivers/STM32_Platform/Src/can.c \
Drivers/STM32_Platform/Src/uart.c \
# ... (all .c files)

# Platform includes
C_INCLUDES += \
-IDrivers/STM32_Platform/Inc
```

### 5. Initialize in main.c

Code is already in user sections from template:

```c
/* USER CODE BEGIN 2 */
handler_set_t handlers = {
    .hcan1 = &hcan1,
    .huart2 = &huart2,
    .hspi1 = &hspi1,
    .hadc1 = &hadc1,
    .htim2 = &htim2
};

plt_callbacks_t callbacks = {
    .CAN_RxCallback = Can_MessageHandler,
    .UART_RxCallback = Uart_MessageHandler
};

plt_SetHandlers(&handlers);
plt_SetCallbacks(&callbacks);
plt_CanInit(64);
plt_UartInit(32);
/* USER CODE END 2 */
```

---

## Template Validation

Verify template configuration:

```bash
python scripts/validate_cubemx_config.py templates/STM32F446RE_Platform_Template.ioc
```

Expected output:

```
✓ All checks passed! Configuration meets platform requirements.
```

---

## Customization

### Modify Peripherals

Templates provide baseline configuration. Adjust in CubeMX:

1. **Clock speeds** - maintain APB1 ≥ 42 MHz for CAN
2. **DMA priorities** - keep platform peripherals at configured levels
3. **GPIO pins** - change as needed for hardware layout
4. **Additional peripherals** - add without affecting platform

### Re-generate Code

CubeMX preserves user code sections when regenerating:

```
/* USER CODE BEGIN ... */
// Your platform initialization stays here
/* USER CODE END ... */
```

---

## Template Creation (For Developers)

To create new template:

1. Configure STM32CubeMX project with platform requirements
2. Add user code sections with platform init
3. Save `.ioc` file
4. Validate with `validate_cubemx_config.py`
5. Document peripherals and pins in README
6. Test code generation preserves user sections

---

## Template Families

### Planned Templates

- [ ] **STM32F103C8** (Blue Pill) - Basic CAN/UART
- [ ] **STM32F407VG** (Discovery) - Full peripheral set
- [ ] **STM32H743ZI** (Nucleo-144) - High performance
- [ ] **STM32F746NG** (Discovery) - With display integration
- [ ] **STM32G474RE** - Dual CAN, advanced timers

**Contribute templates:** See [CONTRIBUTING.md](../CONTRIBUTING.md)

---

## Troubleshooting

**Issue:** User code sections missing after generation

**Solution:** Enable in CubeMX:

- **Project Manager → Code Generator**
- Check "Keep User Code when re-generating"

**Issue:** DMA conflicts

**Solution:** Templates use specific DMA streams. Check generated `dma.c` for conflicts.

**Issue:** Clock configuration warnings

**Solution:** Templates optimize for platform. Verify PLL settings match your crystal/oscillator.

---

## References

- [Integration Guide](../docs/integration_guide.md)
- [CubeMX Integration](../docs/cubemx_integration.md)
- [Validation Script](../scripts/validate_cubemx_config.py)
