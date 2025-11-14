# API Redesign - Consumer-Grade Interface

## Current vs Proposed

### Current (Clunky)

```c
// Initialize
handler_set_t handlers = {
    .hcan1 = &hcan1,
    .huart2 = &huart2,
    .hspi1 = &hspi1
};

plt_callbacks_t callbacks = {
    .CAN_RxCallback = Can_MessageHandler,
    .UART_RxCallback = Uart_MessageHandler
};

plt_SetHandlers(&handlers);
plt_SetCallbacks(&callbacks);
plt_CanInit(64);
plt_UartInit(32);
plt_SpiInit(16);

// Usage
can_message_t msg = {0};
msg.id = 0x180;
memcpy(msg.data, myData, 8);
plt_CanSendMsg(Can1, &msg);

// Process in main loop
while(1) {
    plt_CanProcessRxMsgs();
    plt_UartProcessRxMsgs();
    plt_SpiProcessRxMsgs();
}
```

### Proposed (Modern)

```c
// Initialize - ONE function
STM32.begin(&hcan1, &huart2, &hspi1)
    .onCAN(Can_MessageHandler)
    .onUART(Uart_MessageHandler);

// Usage - Clean and intuitive
CAN.send(0x180, myData, 8);
UART.print("Hello World\n");
SPI.transfer(buffer, 16);
ADC.read(CHANNEL_1);
PWM.setFrequency(TIM2, 1000);
PWM.setDutyCycle(TIM2, CH1, 50.0);

// Process in main loop - ONE function
while(1) {
    STM32.process();  // Handles all peripherals automatically
}
```

---

## New Architecture

### 1. **Singleton Objects** (Arduino-like)

```c
// Global singletons (hidden from user)
extern STM32_t STM32;
extern CAN_t CAN;
extern UART_t UART;
extern SPI_t SPI;
extern ADC_t ADC;
extern PWM_t PWM;
```

### 2. **Fluent Initialization**

```c
// Inc/stm32_platform.h

typedef struct {
    // Fluent builder methods
    STM32_t* (*onCAN)(void (*callback)(CANMessage_t*));
    STM32_t* (*onUART)(void (*callback)(UARTMessage_t*));
    STM32_t* (*onSPI)(void (*callback)(SPIMessage_t*));

    // Core methods
    void (*process)(void);
    void (*setDebugOutput)(UART_HandleTypeDef* huart);
    const char* (*version)(void);
} STM32_t;

// Simple initialization
STM32_t* STM32_begin(CAN_HandleTypeDef* hcan,
                     UART_HandleTypeDef* huart,
                     SPI_HandleTypeDef* hspi);

// Usage
STM32.begin(&hcan1, &huart2, &hspi1)
    ->onCAN(HandleCANMessage)
    ->onUART(HandleUARTMessage);
```

### 3. **Clean Peripheral APIs**

#### CAN Interface

```c
typedef struct {
    uint16_t id;
    uint8_t data[8];
    uint8_t length;
} CANMessage_t;

typedef struct {
    // Send methods
    bool (*send)(uint16_t id, const uint8_t* data, uint8_t length);
    bool (*sendMessage)(const CANMessage_t* msg);

    // Configuration
    void (*setBaudrate)(uint32_t baudrate);
    void (*setFilter)(uint16_t id, uint16_t mask);

    // Status
    uint16_t (*available)(void);
    bool (*isReady)(void);
    uint32_t (*getErrorCount)(void);
} CAN_t;

// Usage examples
CAN.send(0x180, myData, 8);
CAN.setBaudrate(500000);

if (CAN.available() > 0) {
    // Messages ready
}
```

#### UART Interface

```c
typedef struct {
    // Arduino-style print
    void (*print)(const char* str);
    void (*println)(const char* str);
    void (*printf)(const char* fmt, ...);

    // Binary send
    bool (*write)(const uint8_t* data, uint16_t length);

    // Read
    uint16_t (*available)(void);
    uint8_t (*read)(void);
    uint16_t (*readBytes)(uint8_t* buffer, uint16_t length);

    // Configuration
    void (*setBaudrate)(uint32_t baudrate);
    void (*setTimeout)(uint16_t ms);
} UART_t;

// Usage examples
UART.println("System initialized");
UART.printf("Temperature: %.2f°C\n", temp);

if (UART.available() > 0) {
    uint8_t data = UART.read();
}
```

#### SPI Interface

```c
typedef struct {
    // Transfer
    void (*transfer)(uint8_t* txData, uint8_t* rxData, uint16_t length);
    uint8_t (*transferByte)(uint8_t data);

    // Configuration
    void (*setClockSpeed)(uint32_t hz);
    void (*setMode)(uint8_t mode);  // 0-3
    void (*setBitOrder)(bool msbFirst);

    // Chip select helpers
    void (*selectDevice)(GPIO_TypeDef* port, uint16_t pin);
    void (*deselectDevice)(GPIO_TypeDef* port, uint16_t pin);
} SPI_t;

// Usage examples
SPI.selectDevice(GPIOA, GPIO_PIN_4);
uint8_t response = SPI.transferByte(0xAA);
SPI.deselectDevice(GPIOA, GPIO_PIN_4);
```

#### ADC Interface

```c
typedef struct {
    // Read methods
    uint16_t (*read)(uint8_t channel);
    float (*readVoltage)(uint8_t channel);

    // Multi-channel
    void (*readMultiple)(uint8_t* channels, uint16_t* values, uint8_t count);

    // Configuration
    void (*setResolution)(uint8_t bits);  // 8, 10, 12
    void (*setReference)(float voltage);

    // Calibration
    void (*calibrate)(void);
} ADC_t;

// Usage examples
uint16_t raw = ADC.read(1);
float voltage = ADC.readVoltage(1);
float temp = (voltage - 0.76) / 0.0025 + 25.0;  // STM32 temp sensor
```

#### PWM Interface

```c
typedef struct {
    // Simple control
    void (*start)(TIM_HandleTypeDef* htim, uint32_t channel);
    void (*stop)(TIM_HandleTypeDef* htim, uint32_t channel);

    // Configuration
    void (*setFrequency)(TIM_HandleTypeDef* htim, uint32_t hz);
    void (*setDutyCycle)(TIM_HandleTypeDef* htim, uint32_t channel, float percent);

    // Advanced
    void (*setPulseWidth)(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t us);
} PWM_t;

// Usage examples
PWM.setFrequency(&htim2, 1000);     // 1 kHz
PWM.setDutyCycle(&htim2, CH1, 75.0); // 75%
PWM.start(&htim2, CH1);
```

---

## Complete Example - Old vs New

### Old Way (Clunky)

```c
/* main.c */
#include "platform.h"
#include "callbacks.h"

void Can_MessageHandler(can_message_t* msg) {
    // Handle message
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_CAN1_Init();
    MX_USART2_UART_Init();

    // Platform setup - verbose!
    handler_set_t handlers = {
        .hcan1 = &hcan1,
        .huart2 = &huart2
    };

    plt_callbacks_t callbacks = {
        .CAN_RxCallback = Can_MessageHandler
    };

    plt_SetHandlers(&handlers);
    plt_SetCallbacks(&callbacks);
    plt_CanInit(64);
    plt_UartInit(32);

    while (1) {
        plt_CanProcessRxMsgs();
        plt_UartProcessRxMsgs();

        // Send CAN message - clunky!
        can_message_t msg = {0};
        msg.id = 0x180;
        msg.data[0] = 0xAA;
        msg.data[1] = 0xBB;
        plt_CanSendMsg(Can1, &msg);

        HAL_Delay(100);
    }
}
```

### New Way (Clean)

```c
/* main.c */
#include "stm32_platform.h"

void onCANMessage(CANMessage_t* msg) {
    UART.printf("CAN ID: 0x%03X\n", msg->id);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_CAN1_Init();
    MX_USART2_UART_Init();

    // Platform setup - ONE line!
    STM32.begin(&hcan1, &huart2, NULL)
         ->onCAN(onCANMessage);

    UART.println("System Ready");

    while (1) {
        STM32.process();  // Handles everything

        // Send CAN - intuitive!
        uint8_t data[] = {0xAA, 0xBB};
        CAN.send(0x180, data, 2);

        HAL_Delay(100);
    }
}
```

---

## Error Handling - New Approach

### Current (Mixed types)

```c
HAL_StatusTypeDef status = plt_CanSendMsg(Can1, &msg);
if (status != HAL_OK) {
    // Error
}
```

### Proposed (Boolean + errno-style)

```c
if (!CAN.send(0x180, data, 8)) {
    // Check last error
    ErrorCode_t err = STM32.getLastError();
    UART.printf("CAN Error: %s\n", STM32.getErrorString(err));
}

// Or chain error handler
CAN.send(0x180, data, 8) || STM32.onError(handleError);
```

---

## Advanced Features - Still Available

### Database Access (Simplified)

```c
// Current
database_t* db = db_GetDBPointer();
db->vcu_node->speed = 100;

// Proposed
VCU.speed = 100;
VCU.throttle = 75;
PEDAL.position = 50;

// Or via getter/setters
Database.set("vcu.speed", 100);
float speed = Database.get("vcu.speed");
```

### Message Routing (Hidden from user)

```c
// Current - manual hashtable
hash_member_t member = {
    .id = 0x180,
    .Set_Function = UpdateSpeed
};
hash_InsertMember(&member);

// Proposed - automatic routing
CAN.route(0x180, UpdateSpeed);
CAN.route(0x181, UpdateThrottle);
CAN.route(0x182, UpdateBrake);

// Or range routing
CAN.routeRange(0x180, 0x19E, HandlePedalMessages);
```

---

## Migration Path

### Phase 1: Add New API (Compatibility Layer)

Keep old API, add new alongside:

```c
// Old still works
plt_CanSendMsg(Can1, &msg);

// New available
CAN.send(0x180, data, 8);
```

### Phase 2: Deprecate Old API

```c
// Mark old functions deprecated
__attribute__((deprecated("Use CAN.send() instead")))
HAL_StatusTypeDef plt_CanSendMsg(CanChanel_t chanel, can_message_t *pData);
```

### Phase 3: Remove Old API (v2.0)

Clean break, new API only.

---

## Implementation Strategy

### 1. Create Wrapper Layer

```c
// Src/stm32_api.c - new interface
// Src/stm32_legacy.c - old interface (calls new internally)
```

### 2. Singleton Implementation

```c
// Global objects
STM32_t STM32 = {
    .begin = STM32_begin,
    .process = STM32_process,
    // ...
};

CAN_t CAN = {
    .send = CAN_send,
    .available = CAN_available,
    // ...
};
```

### 3. Maintain Backward Compatibility

```c
// Old function wraps new
HAL_StatusTypeDef plt_CanSendMsg(CanChanel_t chanel, can_message_t *pData) {
    return CAN.send(pData->id, pData->data, 8) ? HAL_OK : HAL_ERROR;
}
```

---

## Benefits

1. **Familiar** - Arduino-style, industry standard
2. **Discoverable** - `CAN.` + autocomplete shows all methods
3. **Less typing** - `CAN.send()` vs `plt_CanSendMsg()`
4. **Chainable** - Fluent interface for configuration
5. **Consistent** - All peripherals follow same pattern
6. **Modern** - Feels like 2025, not 2010

---

## Comparison with Other Platforms

### Arduino

```c
Serial.begin(115200);
Serial.println("Hello");
```

### mbed

```c
Serial pc(USBTX, USBRX);
pc.printf("Hello\n");
```

### Our New API

```c
UART.begin(115200);
UART.println("Hello");
```

**Conclusion:** Match industry expectations while providing STM32 HAL power underneath.
