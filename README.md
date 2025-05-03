
# 🧠 Embedded Communication Platform for STM32

A modular, scalable communication and control platform for STM32 microcontrollers, supporting **CAN**, **UART**, **SPI**, **ADC**, and **TIM** peripherals using **HAL drivers**, **DMA**, and **callback-driven** architecture.

---

## 🚀 Overview

This platform abstracts and unifies peripheral communication across STM32 devices, enabling clean separation of logic and hardware layers. It uses **DMA for non-blocking data transfer**, **circular queues for message buffering**, and **callback registration** for real-time processing.

---

## 🧩 Features

- ✅ **HAL-based abstraction** for SPI, UART, CAN, ADC, and TIM
- 🔄 **DMA-enabled** transmit and receive for asynchronous communication
- 📦 **Queue-based buffering** for RX/TX channels
- 🧠 **Callback-driven** processing of incoming messages
- 🔌 **Dynamic handler registration** for peripheral handles
- 📊 **ADC multi-channel sampling** and real-time averaging
- ⚙️ **PWM generation** via TIM modules
- 🗂️ **In-memory database** to store parsed messages from peripherals

---

## 📁 Project Structure

```
├── platform.c           # Core platform init and handler registration
├── callbacks.c          # Application-defined callbacks for each protocol
├── spi.c                # SPI communication with DMA + queue
├── uart.c               # UART communication with DMA + printf redirection
├── can.c                # CAN communication with filter + RX queue
├── adc.c                # ADC data collection and processing
├── tim.c                # Timer and PWM control logic
├── utils.c              # Queue and memory management utilities
├── database.c           # In-memory database of platform values
├── DbSetFunctions.c     # Helper functions to set values in the database
```

---

## ⚙️ Dependencies

- STM32 HAL drivers (`stm32fxxx_hal_*`)
- Standard C library (`stdlib.h`, `string.h`)
- STM32CubeMX or STM32CubeIDE for peripheral config generation

---

## 🧪 Initialization Flow

```c
// Application Entry Point
PlatformInit(&handler_set, RX_QUEUE_SIZE);

// Inside PlatformInit()
plt_SetHandlers()
plt_SetCallbacks()
plt_CanInit()
plt_UartInit()
plt_SpiInit()
plt_AdcInit()
plt_TimInit()
```

---

## 🛠️ How to Use

1. Configure `handler_set_t` with initialized HAL handles (e.g., `hspi1`, `huart1`, etc.)
2. Call `PlatformInit(...)` in your `main.c`
3. Define your custom logic in `callbacks.c`
4. Periodically call:
   ```c
   plt_CanProcessRxMsgs();
   plt_UartProcessRxMsgs();
   plt_SpiProcessRxMsgs();
   ```

---

## 📌 Example: SPI Callback

```c
void SpiRxCallback(spi_message_t *msg) {
    // Parse and store into DB
    memcpy(&pMainDB->sub_node->sensor_data, msg->data, sizeof(sensor_data_t));
}
```

---

## 📚 Future Improvements

- [ ] Add unit testing framework (Ceedling or Unity)
- [ ] Add configuration file for easy platform setup
- [ ] Optional RTOS integration
- [ ] Add logging and debug interface


