# STM32 Platform Tests

This directory contains unit tests for the STM32 Platform using the Unity Test Framework.

## Structure

```
tests/
├── unity/                  # Unity test framework (submodule)
├── test_utils.c           # Queue implementation tests
├── test_can.c             # CAN module tests
├── test_uart.c            # UART module tests
├── test_database.c        # Database tests
├── test_hashtable.c       # Hash table tests
├── mocks/                 # Mock HAL functions
│   └── stm32_hal_mocks.c
├── CMakeLists.txt         # Build configuration
└── README.md              # This file
```

## Setup

### Prerequisites

- CMake 3.15 or higher
- GCC compiler (arm-none-eabi-gcc for target, gcc for host tests)
- Unity Test Framework

### Install Unity

```bash
cd tests
git clone https://github.com/ThrowTheSwitch/Unity.git unity
```

Or add as submodule:

```bash
git submodule add https://github.com/ThrowTheSwitch/Unity.git tests/unity
git submodule update --init --recursive
```

## Building Tests

### Host Tests (Native)

Build and run tests on your development machine:

```bash
cd tests
mkdir build && cd build
cmake ..
make
ctest --verbose
```

### Target Tests (STM32)

For running tests on actual hardware:

```bash
cd tests
mkdir build_target && cd build_target
cmake -DCMAKE_TOOLCHAIN_FILE=../arm-toolchain.cmake ..
make
# Flash to STM32 and view UART output
```

## Running Tests

### All Tests

```bash
ctest
```

### Specific Test

```bash
./build/test_utils
./build/test_can
```

### With Verbose Output

```bash
ctest --verbose
```

## Writing New Tests

### Test File Template

```c
#include "unity.h"
#include "module_to_test.h"
#include "mock_dependencies.h"

void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
}

void test_FunctionName_Condition_ExpectedResult(void) {
    // Arrange
    int input = 5;

    // Act
    int result = FunctionUnderTest(input);

    // Assert
    TEST_ASSERT_EQUAL(10, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_FunctionName_Condition_ExpectedResult);
    return UNITY_END();
}
```

### Test Naming Convention

- `test_<Function>_<Condition>_<ExpectedResult>`
- Example: `test_QueuePush_WhenFull_ReturnsError`

## Mocking

HAL functions are mocked to run tests without hardware:

```c
// In mocks/stm32_hal_mocks.c
HAL_StatusTypeDef HAL_CAN_AddTxMessage(
    CAN_HandleTypeDef *hcan,
    CAN_TxHeaderTypeDef *pHeader,
    uint8_t aData[],
    uint32_t *pTxMailbox)
{
    // Mock implementation
    return HAL_OK;
}
```

## Coverage

Generate code coverage report:

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Coverage ..
make
make coverage
```

View `coverage/index.html` in browser.

## Continuous Integration

Tests run automatically on GitHub Actions for every push/PR.

See `.github/workflows/test.yml` for configuration.

## Troubleshooting

### Issue: Unity not found

**Solution:** Run `git submodule update --init`

### Issue: HAL errors during compilation

**Solution:** Ensure mocks are properly included in CMakeLists.txt

### Issue: Tests pass but behavior fails on hardware

**Solution:** Check mock assumptions, run integration tests on target

## Test Coverage Goals

- [ ] Utils (Queues): 100%
- [ ] CAN Module: 90%
- [ ] UART Module: 90%
- [ ] SPI Module: 90%
- [ ] Database: 95%
- [ ] Hash Table: 100%

## Contributing

Requirements for new features:

1. Tests written before implementation (TDD)
2. All tests pass
3. Code coverage >85%
4. Test assumptions documented

Refer to [CONTRIBUTING.md](../CONTRIBUTING.md).
