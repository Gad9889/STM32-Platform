# Architecture

## System Design

The STM32 Platform uses a **layered architecture** separating concerns:

```
┌─────────────────────────────────────────┐
│     Application Layer (Your Code)       │
│  State machines, control algorithms     │
└──────────────┬──────────────────────────┘
               │ Callbacks
┌──────────────▼──────────────────────────┐
│     Platform Layer (This Library)       │
│  ┌─────────────────────────────────┐   │
│  │ CAN │ UART │ SPI │ ADC │ PWM   │   │
│  └──────────────┬──────────────────┘   │
│  ┌──────────────▼──────────────────┐   │
│  │  Queue Manager & Router         │   │
│  └─────────────────────────────────┘   │
└──────────────┬──────────────────────────┘
               │ HAL API
┌──────────────▼──────────────────────────┐
│   STM32 HAL (DMA, Interrupts)          │
└─────────────────────────────────────────┘
```

## Core Components

### 1. Singleton Objects

Each peripheral exposed as a global object:

```c
CAN_t CAN;      // CAN communication
UART_t UART;    // Serial communication
SPI_t SPI;      // SPI interface
ADC_t ADC;      // Analog input
PWM_t PWM;      // PWM output
Platform_t Platform;  // Core initialization
```

### 2. Message Queues

Thread-safe circular buffers prevent data loss:

- **CAN RX Queue** - Stores incoming CAN messages
- **UART TX Queue** - Buffers outgoing serial data
- **SPI RX Queue** - Stores SPI read data

```c
typedef struct {
    void* data;
    uint16_t capacity;
    uint16_t head;
    uint16_t tail;
    QueueStatus_t status;
} Queue_t;
```

### 3. Callback System

Event-driven architecture:

```c
// Register handlers
Platform.begin(&hcan1, &huart2, NULL)
        ->onCAN(my_can_handler);

// Handler called automatically
void my_can_handler(CANMessage_t* msg) {
    // Process message
}
```

### 4. DMA Integration

Zero-copy transfers for performance:

- **CAN**: DMA-based RX FIFO
- **UART**: DMA TX/RX with circular buffer
- **SPI**: Full-duplex DMA transfers
- **ADC**: Continuous DMA sampling with averaging

## Data Flow

### CAN Reception

```
Hardware → HAL IRQ → DMA → Queue → handleRxMessages() → User Callback
```

### UART Transmission

```
User Code → print() → TX Queue → DMA → HAL → Hardware
```

### ADC Sampling

```
Hardware → DMA → Averaging Buffer → readVoltage() → User Code
```

## Design Patterns

### Singleton Pattern

Global peripheral objects provide single point of access:

```c
CAN.send(0x123, data, 8);  // Direct access
```

### Fluent Interface

Method chaining for initialization:

```c
Platform.begin(&hcan1, &huart2, NULL)
        ->onCAN(handler1)
        ->onUART(handler2);
```

### Producer-Consumer

Interrupt handlers produce, user code consumes:

```c
// ISR produces
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    // Enqueue message
}

// User consumes
CAN.handleRxMessages();  // Process queue
```

## Memory Management

- **Static Allocation** - No dynamic memory (malloc-free)
- **Fixed Queues** - Compile-time sized buffers
- **Stack Usage** - Small footprint (~2KB typical)

### Memory Map

```
Flash: ~20KB (code + constants)
RAM:   ~4KB (queues + buffers)
Stack: ~2KB (typical usage)
```

## Thread Safety

### Single-Core Design

Platform assumes single-threaded execution:

- ISRs write to queues
- Main loop reads from queues
- No mutex needed for simple use

### RTOS Considerations

When using FreeRTOS:

```c
// Protect queue access
xSemaphoreTake(canMutex, portMAX_DELAY);
CAN.handleRxMessages();
xSemaphoreGive(canMutex);
```

## Performance

**Benchmarks on STM32F4 @ 168MHz:**

| Operation | Time | CPU Usage |
|-----------|------|-----------|
| CAN RX processing | ~5 µs/msg | <1% @ 1kHz |
| Queue push/pop | ~2 µs | Minimal |
| ADC averaging | ~100 µs | <5% |

**DMA ensures zero-copy transfers with minimal CPU involvement.**

## Extensibility

### Adding New Peripherals

1. Create header in `Inc/`
2. Implement driver in `Src/`
3. Add singleton object
4. Register with Platform

### Custom Message Routing

```c
// Route CAN IDs to specific handlers
CAN.route(0x100, motor_handler);
CAN.route(0x200, sensor_handler);
```

## References

- [API Reference](API-Reference) - Function documentation
- [Examples](Examples) - Implementation patterns
- [Getting Started](Getting-Started) - Quick start guide
