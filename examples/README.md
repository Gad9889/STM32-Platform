# STM32 Platform Examples

This directory contains complete, working examples demonstrating how to use the STM32 Platform in real applications.

## Available Examples

### 🚗 [CAN Communication](can_communication/)

**Status:** ✅ Complete  
**Difficulty:** Beginner  
**Hardware:** STM32F4 + CAN transceiver

Demonstrates:

- Platform initialization
- CAN message transmission
- CAN message reception with callbacks
- Printf debugging via UART
- Error handling

Entry-level example for platform initialization and basic operations.

---

## Coming Soon

### 📡 UART Communication (Planned)

Demonstrates inter-MCU communication and telemetry.

### 🔌 SPI Sensor Reading (Planned)

Demonstrates SPI communication with accelerometer/gyroscope.

### 📊 ADC Multi-Channel Sampling (Planned)

Demonstrates analog sensor reading and averaging.

### ⚙️ PWM Motor Control (Planned)

Demonstrates timer PWM generation for servo/motor control.

### 🎯 Complete Vehicle System (Planned)

Full integration example with all peripherals working together.

---

## How to Use Examples

Each example includes:

- **README.md** - Setup instructions and explanation
- **main.c** - Complete source code
- **Wiring diagram** - Hardware connections
- **CubeMX configuration** - Peripheral settings
- **Expected output** - What you should see
- **Troubleshooting** - Common issues and solutions

### Quick Start

1. **Choose an example** based on your peripherals
2. **Read the README** in the example directory
3. **Configure STM32CubeMX** as described
4. **Copy source files** to your project
5. **Build and flash** to your board
6. **Observe the output** via UART terminal

### Testing on Hardware

Most examples require:

- STM32 development board (NUCLEO, Discovery, or custom)
- ST-LINK debugger
- UART-USB adapter for printf output
- Peripheral-specific hardware (CAN transceiver, sensors, etc.)

### Running Tests Without Hardware

You can test platform logic without hardware using:

```bash
cd tests
mkdir build && cd build
cmake ..
make
ctest
```

---

## Example Template

Want to create your own example? Use this structure:

```
my_example/
├── README.md              # Setup and explanation
├── main.c                 # Source code
├── callbacks.c            # Custom callbacks
├── stm32_config.ioc       # CubeMX project (optional)
└── wiring_diagram.png     # Hardware connections (optional)
```

---

## Contributing Examples

We welcome community examples! To contribute:

1. Create a new directory under `examples/`
2. Include complete, working code
3. Write clear documentation
4. Test on real hardware
5. Submit a pull request

See [CONTRIBUTING.md](../CONTRIBUTING.md) for details.

---

## Hardware Compatibility

Examples are tested on:

- ✅ STM32F4 Discovery
- ✅ NUCLEO-F401RE
- ✅ NUCLEO-F446RE
- ⚠️ STM32F1 (should work, not extensively tested)
- ⚠️ STM32F7/H7 (should work, not extensively tested)

Report compatibility results via GitHub Issues.

---

## Resources

- **Platform Documentation**: [../docs/](../docs/)
- **Integration Guide**: [../docs/integration_guide.md](../docs/integration_guide.md)
- **API Reference**: [../Inc/](../Inc/)
- **Support**: [GitHub Issues](https://github.com/Ben-Gurion-Racing/STM32_Platform/issues)

---
