# Contributing to STM32 Platform

Contribution procedures and coding standards for STM32 Platform.

## Table of Contents

- [Getting Started](#getting-started)
- [How to Contribute](#how-to-contribute)
- [Development Guidelines](#development-guidelines)
- [Testing](#testing)
- [Pull Request Process](#pull-request-process)

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/your-username/STM32_Platform.git
   cd STM32_Platform
   ```
3. **Create a branch** for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## How to Contribute

### Reporting Bugs

Required information:

- STM32 device family and model
- HAL library version
- Minimal reproducible code
- Expected vs actual behavior
- Error messages or stack traces

Verify issue does not exist before submission.

### Suggesting Enhancements

- Label: `enhancement`
- Include: use case, justification, implementation proposal

### Code Contributions

1. Select or create issue
2. Implement changes per coding standards
3. Add tests for new functionality
4. Update documentation
5. Submit pull request

## Development Guidelines

### Coding Standards

#### General Principles

- **MISRA-C compliance** where applicable
- **Defensive programming**: Always validate inputs
- **Clear naming**: Use descriptive variable and function names
- **Comment complex logic**: Help others understand your intent

#### Naming Conventions

```c
// Functions: lowercase with underscores, prefixed by module
void plt_CanInit(size_t queue_size);
void can_ProcessMessage(can_message_t *msg);

// Types: PascalCase with _t suffix
typedef struct {
    uint32_t id;
    uint8_t data[8];
} can_message_t;

typedef enum {
    PLT_OK = 0,
    PLT_ERROR = -1
} plt_status_t;

// Constants/Macros: UPPERCASE with underscores
#define CAN_MAX_DATA_LENGTH 8
#define QUEUE_DEFAULT_SIZE 32

// Global variables: Prefix with module, use camelCase
static handler_set_t *plt_handlers;
```

#### Code Structure

```c
// File header with description
/**
 * @file can.c
 * @brief CAN peripheral driver implementation
 * @author Your Name
 * @date 2025-11-14
 */

/* =============================== Includes =============================== */
#include "can.h"
#include <string.h>

/* =============================== Defines ================================ */
#define CAN_TIMEOUT_MS 100

/* =============================== Types ================================== */
// Internal types here

/* =============================== Private Variables ====================== */
static CAN_HandleTypeDef *pCan1;

/* =============================== Private Functions ====================== */
static void can_InitFilters(CAN_HandleTypeDef *hcan);

/* =============================== Public Functions ======================= */
void plt_CanInit(size_t queue_size) {
    // Implementation
}
```

### Function Documentation

Use Doxygen-style comments:

```c
/**
 * @brief Initialize CAN peripheral with DMA
 *
 * This function configures the CAN peripheral for reception and transmission
 * using DMA for non-blocking operation. It also initializes the RX queue.
 *
 * @param[in] queue_size Size of the reception queue (must be > 0)
 *
 * @return plt_status_t
 *   @retval PLT_OK        Initialization successful
 *   @retval PLT_ERROR     Invalid parameters or HAL error
 *   @retval PLT_NO_MEMORY Failed to allocate queue
 *
 * @pre HAL_CAN_MODULE_ENABLED must be defined
 * @pre plt_SetHandlers() must be called first
 *
 * @note This function should be called only once during initialization
 * @note The CAN peripheral must be configured in STM32CubeMX
 *
 * @see plt_CanProcessRxMsgs()
 * @see plt_CanSendMsg()
 */
plt_status_t plt_CanInit(size_t queue_size);
```

### Error Handling

Always check return values and handle errors gracefully:

```c
// Good
plt_status_t status = plt_CanInit(32);
if (status != PLT_OK) {
    // Handle error appropriately
    Error_Handler();
}

// Bad - Don't ignore return values
plt_CanInit(32);  // ❌
```

### Memory Management

```c
// Always check malloc/calloc results
void *buffer = malloc(size);
if (buffer == NULL) {
    return PLT_NO_MEMORY;
}

// Always free allocated memory
if (buffer != NULL) {
    free(buffer);
    buffer = NULL;  // Prevent double-free
}
```

### Input Validation

```c
plt_status_t plt_CanInit(size_t queue_size) {
    // Validate inputs
    if (queue_size == 0) {
        return PLT_INVALID_PARAM;
    }

    if (plt_handlers == NULL) {
        return PLT_NOT_INITIALIZED;
    }

    // Continue with initialization
}
```

## Testing

### Unit Tests

- Write tests for all new functions
- Use Unity testing framework
- Place tests in `tests/` directory
- Run tests before submitting PR

```c
// Example test
void test_CanInit_ValidParams_ReturnsOK(void) {
    plt_status_t status = plt_CanInit(32);
    TEST_ASSERT_EQUAL(PLT_OK, status);
}
```

### Integration Tests

- Test interaction between modules
- Verify DMA and interrupt behavior
- Test on actual hardware when possible

## Pull Request Process

1. **Update documentation** for any API changes
2. **Run all tests** and ensure they pass
3. **Update CHANGELOG.md** with your changes
4. **Keep commits atomic** - one logical change per commit
5. **Write clear commit messages**:

   ```
   Add CAN error recovery mechanism

   - Implement automatic bus-off recovery
   - Add error counters for monitoring
   - Update error callback with detailed info

   Fixes #123
   ```

6. **Request review** from maintainers
7. **Address feedback** promptly
8. **Squash commits** if requested before merge

### PR Checklist

- [ ] Code follows style guidelines
- [ ] Self-reviewed the code
- [ ] Commented complex sections
- [ ] Updated documentation
- [ ] Added tests for new features
- [ ] All tests pass
- [ ] Updated CHANGELOG.md
- [ ] No merge conflicts

## Project Structure

```
STM32_Platform/
├── Inc/                    # Public header files
│   ├── platform.h         # Core platform definitions
│   ├── can.h              # CAN peripheral API
│   ├── uart.h             # UART peripheral API
│   └── ...
├── Src/                    # Implementation files
│   ├── platform.c
│   ├── can.c
│   └── ...
├── tests/                  # Unit tests
│   ├── test_can.c
│   ├── test_uart.c
│   └── ...
├── examples/              # Example applications
│   ├── can_loopback/
│   ├── uart_echo/
│   └── ...
├── docs/                  # Documentation
│   ├── architecture.md
│   ├── api_reference.md
│   └── ...
└── README.md
```

## Questions

Open issue with `question` label for clarification.

## License

Contributions are licensed under MIT License.

## Development Environment

Recommended: VS Code with C/C++ extension.

---
