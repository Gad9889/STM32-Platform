# STM32 Application Testing Starter Kit

This directory contains a basic testing setup for **your application logic** - not the platform itself.

## Purpose

Test your business logic, algorithms, and application code:
- ✅ State machines
- ✅ Control algorithms
- ✅ Data processing
- ✅ Message parsing
- ✅ Decision logic
- ✅ Calculations

**Note:** The STM32 Platform itself is already tested. This kit is for testing YOUR code that uses the platform.

## What's Included

- **CMakeLists.txt**: CMake configuration template
- **test_example.c**: Example test structure to copy
- **tests.yml**: GitHub Actions workflow template
- **README.md**: This file

## Quick Start

### 1. Create Your First Test

Copy `test_example.c` to test your module:

```bash
cp test_example.c test_vehicle_control.c
```

Edit it to test your functions:

```c
#include "unity.h"
#include "vehicle_control.h"  // Your module

void test_CalculateThrottle_ReturnsCorrectValue(void) {
    // Arrange
    float pedal_position = 0.5f;
    
    // Act
    float throttle = CalculateThrottle(pedal_position);
    
    // Assert
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, throttle);
}
```

### 2. Update CMakeLists.txt

Add your test and source file:

```cmake
add_executable(test_vehicle_control 
    test_vehicle_control.c 
    ${CMAKE_SOURCE_DIR}/Core/Src/vehicle_control.c
)
target_link_libraries(test_vehicle_control unity)
add_test(NAME vehicle_control_tests COMMAND test_vehicle_control)
```

### 3. Build and Run

```bash
cd tests
cmake -S . -B build -G Ninja
cmake --build build
ctest --output-on-failure
```

### 4. Enable GitHub Actions

```bash
mkdir -p .github/workflows
cp tests/tests.yml .github/workflows/
```

## Test Examples

### Testing Pure Logic (No Hardware)

```c
void test_StateMachine_IdleToActive_Transition(void) {
    // Your application state machine
    InitStateMachine();
    ProcessEvent(EVENT_START);
    TEST_ASSERT_EQUAL(STATE_ACTIVE, GetCurrentState());
}
```

### Testing Calculations

```c
void test_PIDController_ProportionalGain_IsCorrect(void) {
    PID_t pid = {.kp = 0.5f, .ki = 0.0f, .kd = 0.0f};
    float output = PID_Calculate(&pid, 10.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 5.0f, output);
}
```

### Testing Data Parsing

```c
void test_ParseCANMessage_ValidData_ExtractsSpeed(void) {
    uint8_t data[8] = {0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t speed = ParseSpeedFromCAN(data);
    TEST_ASSERT_EQUAL_UINT16(0x1234, speed);
}
```

### Testing Boundary Conditions

```c
void test_ThrottleLimit_ExceedsMax_ClampedTo100(void) {
    float throttle = LimitThrottle(150.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, throttle);
}
```

## Unity Assertions

Common assertions:
- `TEST_ASSERT_EQUAL(expected, actual)` - Exact equality
- `TEST_ASSERT_TRUE(condition)` - Boolean true
- `TEST_ASSERT_FALSE(condition)` - Boolean false
- `TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)` - Float comparison
- `TEST_ASSERT_NULL(pointer)` - Pointer is NULL
- `TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, length)` - Array comparison

See [Unity documentation](https://github.com/ThrowTheSwitch/Unity) for complete list.

## Best Practices

### ✅ DO Test:
- Business logic and algorithms
- State machines and transitions
- Data processing and parsing
- Calculations and formulas
- Error handling paths
- Edge cases and boundary conditions

### ❌ DON'T Test:
- HAL functions (already tested by ST)
- Platform functions (already tested in platform repo)
- Hardware behavior
- DMA transfers
- Interrupt timing

## Continuous Integration

After copying `tests.yml` to `.github/workflows/`, tests run automatically on:
- Every push to main/develop
- Every pull request  
- Manual workflow dispatch

## Troubleshooting

**Build errors**: Make sure `APP_SOURCES` in CMakeLists.txt includes all .c files your tests need

**Linker errors**: Add missing source files to your test executable definition

**Test failures**: Run with `ctest --output-on-failure --verbose` for details
