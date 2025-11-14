# CMake Build System Guide

## Overview

Platform can be built as static/shared library using CMake for easy integration via `find_package()` or `FetchContent`.

---

## Quick Start

### Build and Install

```bash
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

### Use in Your Project

```cmake
find_package(STM32Platform 1.0 REQUIRED)

target_link_libraries(your_target PRIVATE
    STM32Platform::platform
    stm32_hal  # Your STM32 HAL library
)
```

---

## Build Options

| Option                 | Default    | Description                            |
| ---------------------- | ---------- | -------------------------------------- |
| `BUILD_SHARED_LIBS`    | OFF        | Build shared library instead of static |
| `PLT_BUILD_TESTS`      | OFF        | Build unit tests                       |
| `PLT_BUILD_EXAMPLES`   | OFF        | Build example applications             |
| `CMAKE_BUILD_TYPE`     | Release    | Build configuration (Debug/Release)    |
| `CMAKE_INSTALL_PREFIX` | /usr/local | Installation directory                 |

### Examples

```bash
# Build with tests
cmake -DPLT_BUILD_TESTS=ON ..

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Custom install location
cmake -DCMAKE_INSTALL_PREFIX=/opt/stm32_platform ..

# Shared library
cmake -DBUILD_SHARED_LIBS=ON ..
```

---

## Integration Methods

### Method 1: Install and find_package()

**Build and install platform:**

```bash
cd STM32_Platform
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/opt/stm32 ..
cmake --build .
sudo cmake --install .
```

**Use in your project:**

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(MySTM32Project)

# Find platform
find_package(STM32Platform 1.0 REQUIRED)

# Your executable/library
add_executable(my_app main.c)

# Link platform
target_link_libraries(my_app PRIVATE
    STM32Platform::platform
    stm32_hal
)
```

### Method 2: CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    stm32_platform
    GIT_REPOSITORY https://github.com/Ben-Gurion-Racing/STM32_Platform.git
    GIT_TAG        v1.0.0  # or main for latest
)

FetchContent_MakeAvailable(stm32_platform)

target_link_libraries(my_app PRIVATE
    STM32Platform::platform
    stm32_hal
)
```

### Method 3: Git Submodule

```bash
git submodule add https://github.com/Ben-Gurion-Racing/STM32_Platform.git libs/stm32_platform
```

```cmake
add_subdirectory(libs/stm32_platform)

target_link_libraries(my_app PRIVATE
    STM32Platform::platform
    stm32_hal
)
```

---

## Cross-Compilation for STM32

### Toolchain File Example

Create `arm-none-eabi.cmake`:

```cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Toolchain
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)

# MCU specific flags (example for STM32F446)
set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS_INIT "${MCU_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${MCU_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${MCU_FLAGS} -specs=nano.specs -specs=nosys.specs")

# Search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

### Build with Toolchain

```bash
mkdir build-arm && cd build-arm
cmake -DCMAKE_TOOLCHAIN_FILE=../arm-none-eabi.cmake ..
cmake --build .
```

---

## Package Generation

### Create Distribution Package

```bash
cd build
cmake ..
cmake --build .
cpack
```

Generates:

- `STM32_Platform-1.0.0-Linux.tar.gz`
- `STM32_Platform-1.0.0-Linux.zip`

### Source Package

```bash
cpack --config CPackSourceConfig.cmake
```

---

## Testing

### Build and Run Tests

```bash
cmake -DPLT_BUILD_TESTS=ON ..
cmake --build .
ctest --output-on-failure
```

### Test Coverage

Requires gcov/lcov:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DPLT_BUILD_TESTS=ON ..
cmake --build .
ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

---

## Project Structure

```
STM32_Platform/
├── CMakeLists.txt              # Main build configuration
├── cmake/
│   └── STM32PlatformConfig.cmake.in  # Package config template
├── Inc/                        # Public headers
├── Src/                        # Implementation
├── tests/
│   └── CMakeLists.txt          # Test configuration
└── examples/
    └── CMakeLists.txt          # Examples configuration
```

---

## Advanced Usage

### Custom Compiler Flags

```cmake
target_compile_options(STM32Platform::platform PRIVATE
    -O2
    -ffunction-sections
    -fdata-sections
)
```

### Link Time Optimization

```cmake
set_target_properties(stm32_platform PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION TRUE
)
```

### Static Analysis

```bash
cmake -DCMAKE_C_CLANG_TIDY="clang-tidy;-checks=*" ..
cmake --build .
```

---

## Troubleshooting

### Missing STM32 HAL

**Error:** Cannot find STM32 HAL headers

**Solution:** Provide HAL include path:

```cmake
target_include_directories(my_app PRIVATE
    /path/to/STM32Cube_FW_F4/Drivers/STM32F4xx_HAL_Driver/Inc
    /path/to/STM32Cube_FW_F4/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    /path/to/STM32Cube_FW_F4/Drivers/CMSIS/Include
)
```

### Version Conflicts

**Error:** STM32Platform version mismatch

**Solution:** Specify exact version:

```cmake
find_package(STM32Platform 1.0.0 EXACT REQUIRED)
```

### Custom Install Location Not Found

**Error:** Could not find STM32Platform

**Solution:** Set CMAKE_PREFIX_PATH:

```cmake
set(CMAKE_PREFIX_PATH "/opt/stm32;${CMAKE_PREFIX_PATH}")
find_package(STM32Platform REQUIRED)
```

---

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v3

      - name: Install ARM toolchain
        run: |
          sudo apt-get update
          sudo apt-get install -y gcc-arm-none-eabi

      - name: Configure
        run: cmake -B build -DPLT_BUILD_TESTS=ON

      - name: Build
        run: cmake --build build

      - name: Test
        run: cd build && ctest --output-on-failure

      - name: Package
        run: cd build && cpack

      - name: Upload artifacts
        uses: actions/upload-artifact@v3
        with:
          name: stm32-platform-packages
          path: build/STM32_Platform-*.tar.gz
```

---

## References

- [CMake Documentation](https://cmake.org/documentation/)
- [CMake Package Registry](https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html)
- [CPack Documentation](https://cmake.org/cmake/help/latest/module/CPack.html)
- [GNU Install Directories](https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html)
