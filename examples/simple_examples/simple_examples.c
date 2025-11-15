/**
 * @file simple_example.c
 * @brief Simple examples showing the new API in action
 */

#include "main.h"
#include "stm32_platform.h"

/* Hardware handles from CubeMX */
CAN_HandleTypeDef hcan1;
UART_HandleTypeDef huart2;
ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim2;

/* ==================== Example 1: Hello World ==================== */

void example1_hello_world(void) {
    // Initialize platform with just UART
    PlatformHandles_t handles = {
        .hcan = NULL,
        .huart = &huart2,
        .hspi = NULL,
        .hadc = NULL,
        .htim = NULL
    };
    Platform.begin(&handles);
    
    // Print to UART
    P_UART.println("Hello World!");
    P_UART.printf("System clock: %d MHz\n", SystemCoreClock / 1000000);
    
    while (1) {
        // No process() needed in v2.0.0 - just handle messages
        P_UART.println("Heartbeat");
        HAL_Delay(1000);
    }
}

/* ==================== Example 2: CAN Echo ==================== */

void onCANReceived(CANMessage_t* msg) {
    // Echo back what we received
    P_UART.printf("Received CAN 0x%03X: ", msg->id);
    for (int i = 0; i < msg->length; i++) {
        P_UART.printf("%02X ", msg->data[i]);
    }
    P_UART.println("");
    
    // Echo back on CAN
    P_CAN.send(msg->id + 1, msg->data, msg->length);
}

void example2_can_echo(void) {
    PlatformHandles_t handles = {
        .hcan = &hcan1,
        .huart = &huart2,
        .hspi = NULL,
        .hadc = NULL,
        .htim = NULL
    };
    Platform.begin(&handles);
    
    // Register CAN handler for ID 0x100
    P_CAN.route(0x100, onCANReceived);
    
    P_UART.println("CAN Echo ready. Send message on 0x100, get echo on 0x101");
    
    while (1) {
        P_CAN.handleRxMessages();  // Process incoming CAN messages
    }
}

/* ==================== Example 3: ADC to CAN ==================== */

void example3_adc_to_can(void) {
    PlatformHandles_t handles = {
        .hcan = &hcan1,
        .huart = &huart2,
        .hspi = NULL,
        .hadc = &hadc1,
        .htim = NULL
    };
    Platform.begin(&handles);
    
    P_UART.println("Sending ADC values via CAN");
    
    while (1) {
        // Read ADC voltages
        float v1 = P_ADC.readVoltage(ADC_CHANNEL_1);
        float v2 = P_ADC.readVoltage(ADC_CHANNEL_2);
        float v3 = P_ADC.readVoltage(ADC_CHANNEL_3);
        
        // Convert to 16-bit values for CAN (scale by 1000 for mV)
        uint16_t ch1 = (uint16_t)(v1 * 1000);
        uint16_t ch2 = (uint16_t)(v2 * 1000);
        uint16_t ch3 = (uint16_t)(v3 * 1000);
        
        // Pack into CAN message
        uint8_t data[8] = {
            (ch1 >> 8) & 0xFF, ch1 & 0xFF,  // Channel 1 (2 bytes)
            (ch2 >> 8) & 0xFF, ch2 & 0xFF,  // Channel 2 (2 bytes)
            (ch3 >> 8) & 0xFF, ch3 & 0xFF,  // Channel 3 (2 bytes)
            0x00, 0x00
        };
        
        P_CAN.send(0x200, data, 8);
        
        P_UART.printf("ADC: %.2fV, %.2fV, %.2fV\n", v1, v2, v3);
        
        HAL_Delay(100);
    }
}

/* ==================== Example 4: PWM LED Breathing ==================== */

void example4_pwm_breathing(void) {
    PlatformHandles_t handles = {
        .hcan = NULL,
        .huart = &huart2,
        .hspi = NULL,
        .hadc = NULL,
        .htim = &htim2
    };
    Platform.begin(&handles);
    
    P_PWM.start(&htim2, TIM_CHANNEL_1);
    P_PWM.setFrequency(&htim2, TIM_CHANNEL_1, 1000);  // 1 kHz PWM
    
    P_UART.println("PWM breathing effect on TIM2 CH1");
    
    float dutyCycle = 0.0;
    float direction = 1.0;
    
    while (1) {
        // Breathing effect - no process() needed in v2.0.0
        dutyCycle += direction * 0.5;
        
        if (dutyCycle >= 100.0) {
            dutyCycle = 100.0;
            direction = -1.0;
        } else if (dutyCycle <= 0.0) {
            dutyCycle = 0.0;
            direction = 1.0;
        }
        
        P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, dutyCycle);
        
        HAL_Delay(10);
    }
}

/* ==================== Example 5: Multi-Peripheral Dashboard ==================== */

void onDashboardCAN(CANMessage_t* msg) {
    if (msg->id == 0x100) {
        // Speed message
        uint16_t speed = (msg->data[0] << 8) | msg->data[1];
        P_UART.printf("Speed: %d km/h\n", speed);
    }
}

void example5_dashboard(void) {
    STM32.begin(&hcan1, &huart2, NULL, &hadc1, &htim2)
         ->onCAN(onDashboardCAN);
    
    // Configure CAN to only receive speed messages
    CAN.setFilter(0x100, 0x7FF);
    
    P_UART.println("Dashboard ready");
    P_UART.printf("Platform version: %s\n", STM32.version());
    
    uint32_t lastUpdate = 0;
    
    while (1) {
        STM32.process();
        
        // Update dashboard every 100ms
        if (HAL_GetTick() - lastUpdate >= 100) {
            lastUpdate = HAL_GetTick();
            
            // Read sensors
            float voltage = P_ADC.readVoltage(ADC_CHANNEL_1);
            float temp = (voltage - 0.76) / 0.0025 + 25.0;  // Internal temp sensor
            
            // Control backlight based on time of day (example)
            static float brightness = 0.0;
            brightness += 0.5;
            if (brightness > 100.0) brightness = 0.0;
            P_PWM.setDutyCycle(&htim2, TIM_CHANNEL_1, brightness);
            
            // Send telemetry via CAN
            uint8_t telemetry[8] = {
                (uint8_t)temp,
                (uint8_t)brightness,
                0, 0, 0, 0, 0, 0
            };
            P_CAN.send(0x500, telemetry, 8);
        }
    }
}

/* ==================== Example 6: Error Handling ==================== */

void example6_error_handling(void) {
    STM32.begin(&hcan1, &huart2, NULL);
    
    P_UART.println("Testing error handling");
    
    // Try to send on uninitialized peripheral
    if (!P_CAN.send(0x100, NULL, 8)) {
        ErrorCode_t err = STM32.getLastError();
        P_UART.printf("Expected error: %s\n", STM32.getErrorString(err));
    }
    
    // Check system health
    if (!STM32.isHealthy()) {
        P_UART.println("System not healthy");
        P_UART.printf("CAN ready: %s\n", P_CAN.isReady() ? "yes" : "no");
        P_UART.printf("UART ready: %s\n", P_UART.isReady() ? "yes" : "no");
    }
    
    // Valid send
    uint8_t data[] = {0x01, 0x02, 0x03};
    if (P_CAN.send(0x100, data, 3)) {
        P_UART.println("CAN send successful");
    }
    
    while (1) {
        STM32.process();
        HAL_Delay(1000);
    }
}

/* ==================== Example 7: Configuration Builder ==================== */

void example7_configuration(void) {
    // Fluent configuration
    STM32.begin(&hcan1, &huart2, NULL, &hadc1, &htim2)
         ->onCAN(onCANReceived)
         ->setDebugOutput(&huart2)
         ->enableWatchdog(5000);  // 5 second timeout
    
    // Configure CAN
    CAN.setBaudrate(500000);
    CAN.setAutoRetransmit(true);
    
    // Configure UART
    UART.setBaudrate(115200);
    UART.setTimeout(1000);
    
    // Configure ADC
    ADC.setResolution(12);
    ADC.setReference(3.3);
    
    P_UART.println("System configured");
    
    while (1) {
        STM32.process();
        STM32.kickWatchdog();  // Pet the dog
        HAL_Delay(100);
    }
}

/* ==================== Example 8: Database Integration ==================== */

void example8_database(void) {
    STM32.begin(&hcan1, &huart2, NULL);
    
    // Initialize database
    Database.init();
    
    // Set values
    Database.set("vcu.speed", 120.5);
    Database.set("vcu.throttle", 75);
    Database.set("pedal.position", 80);
    
    // Read values
    float speed = Database.get("vcu.speed");
    int throttle = Database.getInt("vcu.throttle");
    
    P_UART.printf("Speed: %.1f km/h\n", speed);
    P_UART.printf("Throttle: %d%%\n", throttle);
    
    // Auto-sync to CAN
    Database.syncToCAN(0x400);  // Send database snapshot on CAN 0x400
    
    while (1) {
        STM32.process();
        HAL_Delay(1000);
    }
}

/* ==================== Example 9: Command Line Interface ==================== */

void commandHandler(char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        P_UART.println("Commands: status, reset, can, adc");
    }
    else if (strcmp(cmd, "status") == 0) {
        P_UART.printf("Uptime: %d ms\n", HAL_GetTick());
        P_UART.printf("CAN: %d msgs\n", P_CAN.getRxCount());
    }
    else if (strcmp(cmd, "reset") == 0) {
        P_UART.println("Resetting...");
        NVIC_SystemReset();
    }
    else if (strcmp(cmd, "can") == 0) {
        uint8_t test[] = {0xDE, 0xAD, 0xBE, 0xEF};
        P_CAN.send(0x123, test, 4);
        P_UART.println("CAN test message sent");
    }
    else {
        P_UART.printf("Unknown command: %s\n", cmd);
    }
}

void example9_cli(void) {
    STM32.begin(&hcan1, &huart2, NULL);
    
    P_UART.println("CLI Ready. Type 'help' for commands");
    P_UART.print("> ");
    
    char buffer[64] = {0};
    uint8_t pos = 0;
    
    while (1) {
        STM32.process();
        
        if (UART.available() > 0) {
            char c = UART.read();
            
            if (c == '\n' || c == '\r') {
                buffer[pos] = '\0';
                P_UART.println("");
                
                if (pos > 0) {
                    commandHandler(buffer);
                    pos = 0;
                }
                
                P_UART.print("> ");
            }
            else if (c == '\b' && pos > 0) {
                pos--;
                P_UART.print("\b \b");  // Erase character
            }
            else if (pos < 63) {
                buffer[pos++] = c;
                UART.write(c);  // Echo
            }
        }
    }
}

/* ==================== Main Function - Run Examples ==================== */

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_CAN1_Init();
    MX_USART2_UART_Init();
    MX_ADC1_Init();
    MX_TIM2_Init();
    
    // Choose which example to run
    example1_hello_world();
    // example2_can_echo();
    // example3_adc_to_can();
    // example4_pwm_breathing();
    // example5_dashboard();
    // example6_error_handling();
    // example7_configuration();
    // example8_database();
    // example9_cli();
}
