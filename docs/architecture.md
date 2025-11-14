# STM32 Platform Architecture

## Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Data Flow](#data-flow)
- [Module Details](#module-details)
- [Design Patterns](#design-patterns)
- [Memory Management](#memory-management)
- [Thread Safety](#thread-safety)

## Overview

The STM32 Platform is designed as a **layered architecture** that provides hardware abstraction and message-oriented communication for STM32 microcontrollers. The platform separates concerns into distinct layers:

1. **Application Layer** - User code (FSM, control logic, algorithms)
2. **Platform Layer** - This library (peripheral drivers, queues, callbacks)
3. **HAL Layer** - STM32 HAL drivers (hardware abstraction)
4. **Hardware Layer** - Physical peripherals (CAN, UART, SPI, ADC, TIM)

## System Architecture

### Layered Design

```
┌───────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                          │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────────┐    │
│  │ State       │  │ Control      │  │ Safety           │    │
│  │ Machine     │  │ Algorithms   │  │ Monitoring       │    │
│  └──────┬──────┘  └──────┬───────┘  └────────┬─────────┘    │
│         │                 │                    │               │
│         └─────────────────┴────────────────────┘               │
│                           │                                    │
│                      [Callbacks]                               │
│                           │                                    │
└───────────────────────────┼────────────────────────────────────┘
                            │
┌───────────────────────────▼────────────────────────────────────┐
│                    PLATFORM LAYER                              │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │               Peripheral Managers                        │ │
│  │  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐     │ │
│  │  │ CAN  │  │ UART │  │ SPI  │  │ ADC  │  │ TIM  │     │ │
│  │  └───┬──┘  └───┬──┘  └───┬──┘  └───┬──┘  └───┬──┘     │ │
│  └──────┼─────────┼─────────┼─────────┼─────────┼─────────┘ │
│         │         │         │         │         │            │
│  ┌──────▼─────────▼─────────▼─────────▼─────────▼─────────┐ │
│  │           Message Queue Manager                         │ │
│  │  - Circular queues for RX/TX                           │ │
│  │  - Thread-safe operations                              │ │
│  │  - Overflow protection                                 │ │
│  └──────┬──────────────────────────────────────────────────┘ │
│         │                                                     │
│  ┌──────▼──────────────────────────────────────────────────┐ │
│  │           Message Router & Database                     │ │
│  │  - Hash table for message ID routing                   │ │
│  │  - Centralized data storage                            │ │
│  │  - Type-safe data access                               │ │
│  └─────────────────────────────────────────────────────────┘ │
└───────────────────────────┬────────────────────────────────────┘
                            │
┌───────────────────────────▼────────────────────────────────────┐
│                      HAL LAYER                                 │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │ STM32 HAL Drivers (DMA, Interrupts, Peripherals)        │ │
│  └──────────────────────────────────────────────────────────┘ │
└───────────────────────────┬────────────────────────────────────┘
                            │
┌───────────────────────────▼────────────────────────────────────┐
│                    HARDWARE LAYER                              │
│  [CAN Transceiver] [UART] [SPI Device] [Analog Sensors] [PWM] │
└────────────────────────────────────────────────────────────────┘
```

## Data Flow

### Message Reception Flow

```
1. Hardware receives data
   ↓
2. DMA transfers to buffer (zero CPU)
   ↓
3. DMA complete interrupt fires
   ↓
4. HAL calls platform's DMA callback
   ↓
5. Platform pushes message to queue
   ↓
6. Application calls plt_XxxProcessRxMsgs()
   ↓
7. Platform pops from queue & calls user callback
   ↓
8. User callback processes & updates database
```

### Message Transmission Flow

```
1. Application prepares message
   ↓
2. Calls plt_XxxSendMsg()
   ↓
3. Platform pushes to TX queue (if buffered)
   ↓
4. Platform initiates DMA transfer
   ↓
5. DMA handles transmission (zero CPU)
   ↓
6. DMA complete interrupt fires
   ↓
7. Platform processes next queued message
```

## Module Details

### Platform Core (`platform.c/h`)

**Responsibilities:**

- Handler registration (links HAL handles to platform)
- Callback registration (links user callbacks to platform)
- Central initialization orchestration

**Key Types:**

```c
typedef struct {
    CAN_HandleTypeDef *hcan1, *hcan2, *hcan3;
    UART_HandleTypeDef *huart1, *huart2, *huart3;
    SPI_HandleTypeDef *hspi1, *hspi2, *hspi3;
    ADC_HandleTypeDef *hadc1, *hadc2, *hadc3;
    TIM_HandleTypeDef *htim2, *htim3, *htim4;
} handler_set_t;

typedef struct {
    void (*CAN_RxCallback)(can_message_t *msg);
    void (*UART_RxCallback)(uart_message_t *msg);
    void (*SPI_RxCallback)(spi_message_t *msg);
} plt_callbacks_t;
```

### CAN Module (`can.c/h`)

**Architecture:**

```
┌─────────────────────────────────────────┐
│         CAN Module                      │
│                                         │
│  ┌──────────────────────────────────┐  │
│  │  Multi-Channel Manager           │  │
│  │  (CAN1, CAN2, CAN3)             │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  Filter Configuration            │  │
│  │  - Accept all in ID mask mode    │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  RX Queue (Circular)             │  │
│  │  - Configurable size             │  │
│  │  - Overflow protection           │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  Message Processing              │  │
│  │  - Dequeue & callback            │  │
│  │  - Error handling                │  │
│  └──────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

**Features:**

- Up to 3 CAN interfaces (hardware dependent)
- Configurable RX filtering per interface
- Interrupt-driven reception with FIFO0
- Error detection and recovery
- Mailbox management for transmission

### UART Module (`uart.c/h`)

**Architecture:**

```
┌─────────────────────────────────────────┐
│         UART Module                     │
│                                         │
│  ┌──────────────────────────────────┐  │
│  │  UART1: Inter-MCU Communication  │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  UART2: Debug & Printf           │  │
│  │  - Redirect stdout               │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  UART3: Additional Devices       │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  DMA Circular RX Buffer          │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  TX Queue with DMA               │  │
│  └──────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

**Features:**

- Printf redirection to UART2 (debug)
- DMA-based circular RX for continuous reception
- DMA-based TX with queue for non-blocking send
- Separate debug message queue

### SPI Module (`spi.c/h`)

**Architecture:**

```
┌─────────────────────────────────────────┐
│         SPI Module                      │
│                                         │
│  ┌──────────────────────────────────┐  │
│  │  Mode Detection                  │  │
│  │  - Master: TransmitReceive       │  │
│  │  - Slave: Receive only           │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  DMA Full-Duplex Transfer        │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  RX Queue                        │  │
│  │  - Automatic re-arm              │  │
│  └──────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

**Features:**

- Master and slave mode support
- Full-duplex DMA transfers
- Automatic re-arming after reception
- Dummy byte transmission in master mode

### ADC Module (`adc.c/h`)

**Architecture:**

```
┌─────────────────────────────────────────┐
│         ADC Module                      │
│                                         │
│  ┌──────────────────────────────────┐  │
│  │  Multi-Channel Scanning          │  │
│  │  (3 channels per ADC)            │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  DMA Continuous Sampling         │  │
│  │  (50 samples per channel)        │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  Software Averaging              │  │
│  │  - Noise reduction               │  │
│  │  - 12-bit resolution             │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  Publish to CAN Queue            │  │
│  │  (Internal_ADC message)          │  │
│  └──────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

**Features:**

- Up to 3 ADC modules (ADC1, ADC2, ADC3)
- 3 channels per ADC instance
- 50-sample averaging for noise reduction
- Automatic DMA restart after conversion
- Results published to CAN RX queue

### TIM Module (`tim.c/h`)

**Architecture:**

```
┌─────────────────────────────────────────┐
│         TIM Module                      │
│                                         │
│  ┌──────────────────────────────────┐  │
│  │  TIM2, TIM3, TIM4 Support        │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  PWM Generation                  │  │
│  │  - Configurable frequency        │  │
│  │  - 0-100% duty cycle             │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  Dynamic Period Calculation      │  │
│  │  - Auto prescaler setup          │  │
│  └──────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

**Features:**

- PWM generation with dynamic frequency/duty cycle
- Automatic prescaler calculation
- Multi-channel per timer support
- Runtime duty cycle updates

### Queue Manager (`utils.c/h`)

**Architecture:**

```
┌─────────────────────────────────────────┐
│      Circular Queue Implementation      │
│                                         │
│  ┌──────────────────────────────────┐  │
│  │  Generic Void Pointer Storage    │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  Head/Tail Management            │  │
│  │  - Lock-free for single producer │  │
│  │  - Overflow detection            │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  Status Tracking                 │  │
│  │  - EMPTY / OK / FULL / ERROR     │  │
│  └──────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

**Implementation:**

```c
typedef struct {
    void* data;
    size_t sizeof_data;
} QueueItem_t;

typedef struct {
    QueueItem_t* buffer;
    size_t head;
    size_t tail;
    size_t capacity;
    QueueStatus_t status;
} Queue_t;
```

### Hash Table Router (`hashtable.c/h`)

**Purpose:** Route CAN message IDs to appropriate parsing functions

**Architecture:**

```
┌─────────────────────────────────────────┐
│       Message ID Hash Table             │
│                                         │
│  ┌──────────────────────────────────┐  │
│  │  Hash Function (ID % TABLE_SIZE) │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  Collision Resolution            │  │
│  │  - Linear probing                │  │
│  └──────────┬───────────────────────┘  │
│             │                           │
│  ┌──────────▼───────────────────────┐  │
│  │  Function Pointer Storage        │  │
│  │  ID → Set_Function_t             │  │
│  └──────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

**Usage:**

```c
// Register parser
hash_member_t member = {
    .id = INV1_AV1_ID,
    .Set_Function = setInv1Av1Parameters
};
hash_InsertMember(&member);

// Lookup and call
Set_Function_t parser = hash_Lookup(msg->id);
if (parser != NULL) {
    parser(msg->data);
}
```

### Database (`database.c/h`)

**Purpose:** Centralized storage for vehicle/system state

**Structure:**

```c
typedef struct {
    pedal_node_t *pedal_node;       // Pedal inputs
    sub_node_t *sub_node;           // Sub-VCU data
    vcu_node_t *vcu_node;           // Main VCU state
    dashboard_node_t *dashboard_node; // Dashboard data
} database_t;
```

**Features:**

- Dynamic memory allocation
- Type-safe node access
- Organized by functional subsystem
- Single source of truth for system state

## Design Patterns

### 1. **Dependency Injection**

Handlers and callbacks are injected at initialization, allowing flexibility and testability.

```c
PlatformInit(&handlers, queue_size);
```

### 2. **Observer Pattern**

User registers callbacks that get notified on events (message reception).

```c
void CanRxCallback(can_message_t *msg) {
    // React to CAN message
}
```

### 3. **Producer-Consumer**

ISRs produce messages into queues; application consumes from main loop.

### 4. **Facade Pattern**

Platform layer hides HAL complexity behind simple API.

```c
plt_CanSendMsg(Can1, &msg);  // vs HAL_CAN_AddTxMessage(...)
```

### 5. **Strategy Pattern**

Hash table allows runtime selection of message parsing strategy based on ID.

## Memory Management

### Static Allocation

- All peripheral handles are static
- Fixed-size buffers for DMA

### Dynamic Allocation

- Queue buffers (`Queue_Init` uses `malloc`)
- Database nodes (allocated in `db_AllocateMemory`)

**Cleanup:**

- `Queue_free()` releases queue memory
- `db_FreeMemory()` releases database nodes

### Memory Layout

```
┌────────────────────────────┐ 0x2000_0000 (RAM Start)
│  HAL & System Variables    │
├────────────────────────────┤
│  Platform Static Variables │
│  - Peripheral handles      │
│  - Queue structures        │
├────────────────────────────┤
│  Heap (malloc)             │
│  - Queue buffers           │
│  - Database nodes          │
├────────────────────────────┤
│  Stack (growing down)      │
│  - Application variables   │
│  - Function call stack     │
└────────────────────────────┘ 0x2001_FFFF (RAM End)
```

## Thread Safety

### Current Implementation

- **NOT thread-safe by design**
- Assumes single-threaded or careful RTOS usage

### ISR Context

- DMA callbacks run in ISR context
- Only `Queue_Push` called from ISR
- Queue operations are lock-free for single producer

### RTOS Considerations

If using RTOS:

1. Wrap queue operations with mutexes
2. Call `plt_XxxProcessRxMsgs()` from single task
3. Protect database access with critical sections

```c
// Example RTOS protection
osMutexWait(canQueueMutex, osWaitForever);
Queue_Push(&canRxQueue, &msg);
osMutexRelease(canQueueMutex);
```

## Performance Characteristics

### Time Complexity

- Queue operations: O(1)
- Hash table lookup: O(1) average, O(n) worst (rare)
- Message processing: O(m) where m = queue depth

### Space Complexity

- Queue: O(capacity × message_size)
- Hash table: O(128 × sizeof(hash_member_t)) = 1KB
- Database: ~2KB (depends on node definitions)

### Latency

- ISR → Queue: <5 µs
- Queue → Callback: <10 µs
- Total message latency: <20 µs (typical)

## Scalability

### Adding New Peripherals

1. Create `new_peripheral.c/h` files
2. Follow existing patterns (init, process, callbacks)
3. Register in `PlatformInit()`

### Adding New Message Types

1. Define struct in `platform.h`
2. Add parser function in `DbSetFunctions.c`
3. Register in hash table

### Porting to New STM32 Family

1. Update HAL includes
2. Adjust DMA/interrupt names if changed
3. Update peripheral instance names (e.g., TIM1 vs TIM2)

## Error Handling Strategy

Currently uses:

```c
#define VALID(x) do{if((x) != HAL_OK){Error_Handler();}}while(0)
```

**Improvement (In Progress):**

```c
typedef enum {
    PLT_OK = 0,
    PLT_ERROR = -1,
    PLT_INVALID_PARAM = -2,
    PLT_NO_MEMORY = -3,
    PLT_NOT_INITIALIZED = -4,
    PLT_TIMEOUT = -5
} plt_status_t;

plt_status_t plt_CanInit(size_t queue_size) {
    if (queue_size == 0) return PLT_INVALID_PARAM;
    // ...
    return PLT_OK;
}
```

---

**Document Version:** 1.0  
**Last Updated:** 2025-11-14  
**Author:** Ben Gurion Racing Team
