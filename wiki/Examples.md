# Examples

Complete code examples and usage patterns.

## Basic Examples

### Hello World

Simplest possible program:

```c
#include "stm32_platform.h"

extern UART_HandleTypeDef huart2;

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_UART2_Init();

    Platform.begin(NULL, &huart2, NULL, NULL, NULL);

    UART.println("Hello, STM32 Platform!");

    while (1) {
        HAL_Delay(1000);
    }
}
```

### CAN Send and Receive

```c
#include "stm32_platform.h"

extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart2;

void can_handler(CANMessage_t* msg) {
    UART.printf("CAN RX: ID=0x%03X, Data=", msg->id);
    for (int i = 0; i < msg->length; i++) {
        UART.printf("%02X ", msg->data[i]);
    }
    UART.println("");
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_CAN1_Init();
    MX_UART2_Init();

    Platform.begin(&hcan1, &huart2, NULL, NULL, NULL)
            ->onCAN(can_handler);

    UART.println("CAN example ready");

    uint8_t counter = 0;

    while (1) {
        // Process incoming messages
        CAN.handleRxMessages();

        // Send periodic message
        uint8_t data[] = {counter++, 0x11, 0x22};
        CAN.send(0x100, data, 3);

        HAL_Delay(1000);
    }
}
```

### ADC Reading

```c
#include "stm32_platform.h"

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart2;

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_ADC1_Init();
    MX_UART2_Init();

    Platform.begin(NULL, &huart2, NULL, &hadc1, NULL);

    ADC.calibrate();
    UART.println("ADC example ready");

    while (1) {
        ADC.handleConversions();

        uint16_t raw = ADC.readRaw(0);
        float voltage = ADC.readVoltage(0);

        UART.printf("ADC Ch0: %u (%.2fV)\\n", raw, voltage);

        HAL_Delay(500);
    }
}
```

### PWM Control

```c
#include "stm32_platform.h"

extern TIM_HandleTypeDef htim2;

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_TIM2_Init();

    Platform.begin(NULL, NULL, NULL, NULL, &htim2);

    // Start PWM on channel 1
    PWM.start(&htim2, TIM_CHANNEL_1);
    PWM.setFrequency(&htim2, 1000);  // 1 kHz

    // Fade in and out
    while (1) {
        // Fade in
        for (float duty = 0; duty <= 100; duty += 5) {
            PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, duty);
            HAL_Delay(50);
        }

        // Fade out
        for (float duty = 100; duty >= 0; duty -= 5) {
            PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, duty);
            HAL_Delay(50);
        }
    }
}
```

## Advanced Examples

### CAN Message Routing

Route different CAN IDs to different handlers:

```c
void motor_handler(CANMessage_t* msg) {
    uint16_t rpm = (msg->data[0] << 8) | msg->data[1];
    UART.printf("Motor RPM: %u\\n", rpm);
}

void battery_handler(CANMessage_t* msg) {
    float voltage = (msg->data[0] << 8 | msg->data[1]) / 100.0f;
    UART.printf("Battery: %.2fV\\n", voltage);
}

int main(void) {
    // ... initialization ...

    Platform.begin(&hcan1, &huart2, NULL, NULL, NULL);

    // Route specific IDs
    CAN.route(0x201, motor_handler);
    CAN.route(0x301, battery_handler);

    while (1) {
        CAN.handleRxMessages();  // Automatically calls correct handler
        HAL_Delay(10);
    }
}
```

### SPI Sensor Reading

```c
#include "stm32_platform.h"

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart2;

#define CS_PORT GPIOA
#define CS_PIN GPIO_PIN_4

uint8_t read_sensor_register(uint8_t reg) {
    SPI.select(CS_PORT, CS_PIN);

    SPI.transferByte(0x80 | reg);  // Read command
    uint8_t value = SPI.transferByte(0x00);

    SPI.deselect(CS_PORT, CS_PIN);

    return value;
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_SPI1_Init();
    MX_UART2_Init();

    Platform.begin(NULL, &huart2, &hspi1, NULL, NULL);

    while (1) {
        uint8_t id = read_sensor_register(0x00);
        uint8_t temp = read_sensor_register(0x01);

        UART.printf("Sensor ID: 0x%02X, Temp: %u°C\\n", id, temp);

        HAL_Delay(1000);
    }
}
```

### Multi-Peripheral Application

Complete vehicle control unit example:

```c
#include "stm32_platform.h"

extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;

// CAN message IDs
#define CAN_MOTOR_CMD    0x100
#define CAN_SENSOR_DATA  0x200

// State
typedef struct {
    uint16_t throttle_raw;
    float throttle_percent;
    uint16_t motor_rpm;
    bool fan_enabled;
} VehicleState_t;

VehicleState_t vehicle = {0};

void can_handler(CANMessage_t* msg) {
    if (msg->id == 0x201) {
        // Motor controller response
        vehicle.motor_rpm = (msg->data[0] << 8) | msg->data[1];
        UART.printf("Motor: %u RPM\\n", vehicle.motor_rpm);
    }
}

void update_throttle(void) {
    // Read throttle pedal position
    vehicle.throttle_raw = ADC.readRaw(0);
    vehicle.throttle_percent = (vehicle.throttle_raw / 4095.0f) * 100.0f;

    // Send motor command
    uint8_t cmd[] = {
        (uint8_t)(vehicle.throttle_percent),
        0x00  // Reserved
    };
    CAN.send(CAN_MOTOR_CMD, cmd, 2);
}

void update_cooling(void) {
    // Enable fan if motor RPM > 5000
    if (vehicle.motor_rpm > 5000 && !vehicle.fan_enabled) {
        PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 100.0);
        vehicle.fan_enabled = true;
        UART.println("Fan ON");
    } else if (vehicle.motor_rpm < 4000 && vehicle.fan_enabled) {
        PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, 0.0);
        vehicle.fan_enabled = false;
        UART.println("Fan OFF");
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_CAN1_Init();
    MX_UART2_Init();
    MX_ADC1_Init();
    MX_TIM2_Init();

    Platform.begin(&hcan1, &huart2, NULL, &hadc1, &htim2)
            ->onCAN(can_handler);

    PWM.start(&htim2, TIM_CHANNEL_1);
    PWM.setFrequency(&htim2, 25000);  // 25 kHz for fan

    UART.println("Vehicle Control Unit Ready");
    UART.printf("Version: %s\\n", Platform.version());

    while (1) {
        // Process inputs
        CAN.handleRxMessages();
        ADC.handleConversions();

        // Update control logic
        update_throttle();
        update_cooling();

        // Status reporting
        if (HAL_GetTick() % 1000 == 0) {
            UART.printf("Throttle: %.1f%%, RPM: %u, Fan: %s\\n",
                vehicle.throttle_percent,
                vehicle.motor_rpm,
                vehicle.fan_enabled ? "ON" : "OFF");
        }

        HAL_Delay(10);
    }
}
```

## Debugging Tips

### Enable Verbose Logging

```c
// Log all CAN messages
void debug_can_handler(CANMessage_t* msg) {
    UART.printf("[%lu] CAN ID=0x%03X Len=%u Data=",
        HAL_GetTick(), msg->id, msg->length);
    for (int i = 0; i < msg->length; i++) {
        UART.printf("%02X ", msg->data[i]);
    }
    UART.println("");
}

Platform.begin(&hcan1, &huart2, NULL, NULL, NULL)
        ->onCAN(debug_can_handler);
```

### Check Platform Health

```c
if (!Platform.isHealthy()) {
    plt_status_t err = Platform.getLastError();
    UART.printf("Error: %s\\n", Platform.getErrorString(err));
}
```

### Monitor Queue Status

```c
uint16_t pending = CAN.availableMessages();
if (pending > 30) {
    UART.printf("Warning: %u messages in queue\\n", pending);
}
```

## See Also

- [API Reference](API-Reference) - Function documentation
- [Getting Started](Getting-Started) - Setup guide
- [Architecture](Architecture) - System design
