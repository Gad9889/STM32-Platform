# Welcome to STM32 Platform

**Consumer-grade communication platform for STM32 microcontrollers.**

Get CAN, UART, SPI, ADC, and PWM working in minutes with an intuitive, Arduino-style API.

## Quick Links

- **[Getting Started](Getting-Started)** - Install and write your first program
- **[API Reference](API-Reference)** - Complete function documentation
- **[Examples](Examples)** - Code samples and patterns
- **[VS Code Extension](VS-Code-Extension)** - One-click integration tool

## What is STM32 Platform?

A production-ready communication library designed to make STM32 peripheral programming as easy as Arduino:

```c
// Initialize
Platform.begin(&hcan1, &huart2, &hspi1);

// Use peripherals
P_CAN.send(0x123, data, 8);
P_UART.printf("Speed: %d km/h\n", speed);
float voltage = P_ADC.readVoltage(0);
P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 75.0);
```

## Key Features

✅ **Arduino-Style API** - Intuitive singleton objects: `P_CAN.send()`, `P_UART.println()`  
✅ **One-Click Integration** - VS Code extension adds platform to your project automatically  
✅ **Explicit Control** - You decide when to process messages  
✅ **Battle-Tested** - Used in Formula Student racing  
✅ **Well-Tested** - 49 unit tests, 90%+ coverage  
✅ **Two APIs** - New consumer API + legacy `plt_*` functions

## Supported Peripherals

| Peripheral | API Example                                | Features                          |
| ---------- | ------------------------------------------ | --------------------------------- |
| CAN        | `P_CAN.send()`, `P_CAN.handleRxMessages()`     | Multi-channel, filtering, routing |
| UART       | `P_UART.println()`, `P_UART.printf()`          | DMA, printf support               |
| SPI        | `P_SPI.transfer()`, `P_SPI.transferByte()`     | Full-duplex DMA                   |
| ADC        | `P_ADC.readRaw()`, `P_ADC.readVoltage()`       | Multi-channel, averaging          |
| PWM        | `P_PWM.setDutyCycle()`, `P_PWM.setFrequency()` | Frequency and duty control        |

## Getting Help

- **Questions?** Check [Getting Started](Getting-Started) or [Examples](Examples)
- **Bugs?** [Open an issue](https://github.com/Gad9889/STM32-Platform/issues)
- **Discussions** [GitHub Discussions](https://github.com/Gad9889/STM32-Platform/discussions)

---

**Ready to start?** → [Getting Started](Getting-Started)
