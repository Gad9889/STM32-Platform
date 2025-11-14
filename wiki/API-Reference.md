# API Reference

Complete documentation for the STM32 Platform API.

## Platform Object

Core platform initialization and control.

### `Platform.begin()`

Initialize the platform with HAL peripheral handles.

```c
Platform_t* Platform.begin(
    CAN_HandleTypeDef* hcan,
    UART_HandleTypeDef* huart,
    SPI_HandleTypeDef* hspi,
    ADC_HandleTypeDef* hadc,
    TIM_HandleTypeDef* htim
);
```

**Parameters:**

- `hcan` - CAN handle (or `NULL` if not used)
- `huart` - UART handle (or `NULL` if not used)
- `hspi` - SPI handle (or `NULL` if not used)
- `hadc` - ADC handle (or `NULL` if not used)
- `htim` - Timer handle for PWM (or `NULL` if not used)

**Returns:** Pointer to Platform object for method chaining

**Example:**

```c
Platform.begin(&hcan1, &huart2, NULL, &hadc1, &htim2);
```

### `Platform.onCAN()`, `Platform.onUART()`, `Platform.onSPI()`

Register callback handlers (fluent API).

```c
Platform_t* Platform.onCAN(void (*callback)(CANMessage_t*));
Platform_t* Platform.onUART(void (*callback)(UARTMessage_t*));
Platform_t* Platform.onSPI(void (*callback)(SPIMessage_t*));
```

**Example:**

```c
void my_can_handler(CANMessage_t* msg) {
    P_UART.printf("CAN ID: 0x%03X\\n", msg->id);
}

Platform.begin(&hcan1, &huart2, NULL, NULL, NULL)
        ->onCAN(my_can_handler);
```

### `Platform.version()`

Get platform version string.

```c
const char* Platform.version(void);
```

### `Platform.getLastError()`

Get last error code.

```c
plt_status_t Platform.getLastError(void);
```

### `Platform.isHealthy()`

Check if platform is operating normally.

```c
bool Platform.isHealthy(void);
```

---

## CAN Object

Controller Area Network communication.

### `P_CAN.send()`

Send a CAN message.

```c
bool P_CAN.send(uint16_t id, const uint8_t* data, uint8_t length);
```

**Parameters:**

- `id` - CAN message ID (11-bit standard)
- `data` - Pointer to data bytes
- `length` - Data length (0-8 bytes)

**Returns:** `true` if sent successfully

**Example:**

```c
uint8_t data[] = {0xAA, 0xBB, 0xCC};
if (P_CAN.send(0x100, data, 3)) {
    P_UART.println("Sent!");
}
```

### `P_CAN.handleRxMessages()`

Process all received CAN messages in the queue.

```c
void P_CAN.handleRxMessages(void);
```

**Call this in your main loop to process incoming messages.**

### `P_CAN.availableMessages()`

Get number of messages waiting in RX queue.

```c
uint16_t P_CAN.availableMessages(void);
```

### `CAN.route()`

Route specific CAN ID to a handler function.

```c
void CAN.route(uint16_t id, void (*handler)(CANMessage_t*));
```

**Example:**

```c
void motor_handler(CANMessage_t* msg) {
    uint16_t rpm = (msg->data[0] << 8) | msg->data[1];
    P_UART.printf("RPM: %u\\n", rpm);
}

CAN.route(0x201, motor_handler);
```

### `CAN.isReady()`

Check if CAN peripheral is ready.

```c
bool CAN.isReady(void);
```

---

## UART Object

Universal Asynchronous Receiver/Transmitter.

### `P_UART.print()`

Print a string (no newline).

```c
void P_UART.print(const char* str);
```

### `P_UART.println()`

Print a string with newline.

```c
void P_UART.println(const char* str);
```

### `P_UART.printf()`

Formatted print (like standard printf).

```c
void P_UART.printf(const char* fmt, ...);
```

**Example:**

```c
int speed = 85;
float temp = 23.5;
P_UART.printf("Speed: %d km/h, Temp: %.1f°C\\n", speed, temp);
```

### `UART.write()`

Write raw bytes.

```c
bool UART.write(const uint8_t* data, uint16_t length);
```

### `UART.handleRxData()`

Process received UART data.

```c
void UART.handleRxData(void);
```

### `UART.availableBytes()`

Get number of bytes in RX buffer.

```c
uint16_t UART.availableBytes(void);
```

### `UART.read()`

Read one byte.

```c
uint8_t UART.read(void);
```

---

## SPI Object

Serial Peripheral Interface.

### `P_SPI.transfer()`

Transfer data (send and receive simultaneously).

```c
void P_SPI.transfer(uint8_t* txData, uint8_t* rxData, uint16_t length);
```

### `P_SPI.transferByte()`

Transfer a single byte.

```c
uint8_t P_SPI.transferByte(uint8_t data);
```

**Example:**

```c
uint8_t response = P_SPI.transferByte(0x42);
```

### `SPI.select()`, `SPI.deselect()`

Control chip select pin.

```c
void SPI.select(GPIO_TypeDef* port, uint16_t pin);
void SPI.deselect(GPIO_TypeDef* port, uint16_t pin);
```

**Example:**

```c
SPI.select(GPIOA, GPIO_PIN_4);
uint8_t cmd = P_SPI.transferByte(0x03);  // Read command
uint8_t data = P_SPI.transferByte(0x00); // Get data
SPI.deselect(GPIOA, GPIO_PIN_4);
```

---

## ADC Object

Analog-to-Digital Converter.

### `P_ADC.readRaw()`

Read raw ADC value.

```c
uint16_t P_ADC.readRaw(uint8_t channel);
```

**Returns:** Raw ADC value (0-4095 for 12-bit ADC)

### `P_ADC.readVoltage()`

Read ADC value as voltage.

```c
float P_ADC.readVoltage(uint8_t channel);
```

**Returns:** Voltage in volts

**Example:**

```c
float voltage = P_ADC.readVoltage(0);
P_UART.printf("Voltage: %.2fV\\n", voltage);
```

### `ADC.handleConversions()`

Process ADC conversions (for DMA mode).

```c
void ADC.handleConversions(void);
```

### `ADC.calibrate()`

Calibrate ADC.

```c
void ADC.calibrate(void);
```

---

## PWM Object

Pulse Width Modulation (Timer).

### `PWM.start()`, `PWM.stop()`

Start/stop PWM output.

```c
void PWM.start(TIM_HandleTypeDef* htim, uint32_t channel);
void PWM.stop(TIM_HandleTypeDef* htim, uint32_t channel);
```

### `P_PWM.setFrequency()`

Set PWM frequency in Hz.

```c
void P_PWM.setFrequency(TIM_HandleTypeDef* htim, uint32_t hz);
```

### `P_PWM.setDutyCycle()`

Set duty cycle (0-100%).

```c
void P_PWM.setDutyCycle(TIM_HandleTypeDef* htim, uint32_t channel, float percent);
```

**Example:**

```c
PWM.start(&htim2, TIM_CHANNEL_1);
P_PWM.setFrequency(&htim2, 1000);  // 1 kHz
P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 75.0);  // 75%
```

---

## Data Types

### `CANMessage_t`

```c
typedef struct {
    uint16_t id;
    uint8_t data[8];
    uint8_t length;
    uint32_t timestamp;
} CANMessage_t;
```

### `UARTMessage_t`

```c
typedef struct {
    uint8_t data[256];
    uint16_t length;
} UARTMessage_t;
```

### `plt_status_t`

Error codes:

- `PLT_OK` - Success
- `PLT_ERROR_INVALID_PARAM` - Invalid parameter
- `PLT_ERROR_NULL_POINTER` - NULL pointer
- `PLT_ERROR_QUEUE_FULL` - Queue overflow
- `PLT_ERROR_HAL` - HAL error

---

## Legacy API

The `plt_*` functions are still available for backward compatibility.

See [Migration Guide](Migration-Guide) for converting legacy code to the new API.
