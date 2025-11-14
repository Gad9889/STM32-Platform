# CAN Communication Example

This example demonstrates how to use the STM32 Platform for CAN bus communication.

## Hardware Requirements

- STM32F4 or compatible microcontroller
- CAN transceiver (e.g., TJA1050, MCP2551)
- CAN bus with at least one other device

## Wiring

```
STM32          CAN Transceiver
-----          ---------------
CAN_TX (PA12) → TXD
CAN_RX (PA11) → RXD
3.3V          → VCC
GND           → GND

CAN Transceiver → CAN Bus
---------------    --------
CANH           →  CANH
CANL           →  CANL
```

## Configuration (STM32CubeMX)

1. **Enable CAN1**:

   - Mode: Master
   - Prescaler: 4
   - Time Quantum: BS1=13, BS2=2
   - Baud Rate: 500 kbit/s (for 42MHz APB1)

2. **Enable GPIO**:

   - PA11: CAN1_RX
   - PA12: CAN1_TX

3. **Clock Configuration**:
   - APB1: 42 MHz
   - System Clock: 168 MHz (for STM32F4)

## Code

See `main.c` for complete implementation.

## Usage

### Sending CAN Messages

```c
can_message_t tx_msg = {
    .id = 0x123,
    .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
};

plt_status_t status = plt_CanSendMsg(Can1, &tx_msg);
if (status != PLT_OK) {
    printf("Error sending CAN message: %s\n", plt_StatusToString(status));
}
```

### Receiving CAN Messages

Messages are received automatically via DMA and stored in a queue. Process them in your main loop:

```c
while (1) {
    plt_CanProcessRxMsgs();  // Calls your CanRxCallback
    HAL_Delay(10);
}
```

### Custom Callback

Define your callback in `callbacks.c`:

```c
void CanRxCallback(can_message_t *msg) {
    // Handle received message
    switch (msg->id) {
        case 0x100:
            // Motor controller status
            uint16_t rpm = (msg->data[0] << 8) | msg->data[1];
            printf("Motor RPM: %u\n", rpm);
            break;

        case 0x200:
            // Battery status
            uint16_t voltage = (msg->data[0] << 8) | msg->data[1];
            printf("Battery: %u.%uV\n", voltage/10, voltage%10);
            break;

        default:
            printf("Unknown CAN ID: 0x%03lX\n", msg->id);
            break;
    }
}
```

## Expected Output

```
Platform Initialized
CAN Initialized
Sending CAN message...
Received CAN ID: 0x100
Motor RPM: 1500
Received CAN ID: 0x200
Battery: 48.0V
```

## Troubleshooting

### No CAN messages received

1. Check wiring connections
2. Verify CAN bus termination (120Ω resistors)
3. Check baud rate matches other devices
4. Verify CAN filter configuration

### CAN Error: Bus-Off

1. Check for short circuits
2. Verify termination resistors
3. Reduce bus speed if cables are long
4. Check for electrical noise

### Messages send but not received by other devices

1. Verify other devices are on same baud rate
2. Check if other devices have matching filters
3. Test with CAN bus analyzer

## Next Steps

- Implement CAN error handling and recovery
- Add periodic transmission scheduler
- Configure message filtering for specific IDs
- Integrate with vehicle database layer

## References

- [Platform Architecture](../../docs/architecture.md)
- [CAN Module Documentation](../../Inc/can.h)
