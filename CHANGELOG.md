# Changelog

All notable changes to the STM32 Platform project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-11-14

### Added

- **Safety and Validation:** NULL pointer checks, bounds checking, parameter validation in all core modules (platform.c, can.c, uart.c, spi.c, adc.c, tim.c, utils.c, database.c, hashtable.c)
- **Configuration System:** `platform_config.h` with compile-time flags and runtime configuration struct
- **Version Management:** Platform version information (MAJOR.MINOR.PATCH)
- **CMake Build System:** Complete library build configuration with install/export rules
- **Package Configuration:** CMake package config files for `find_package()` support
- **Build Documentation:** `docs/cmake_build_guide.md` with cross-compilation and integration examples
- Unit testing framework (Unity + CMock) with 49 tests
- Comprehensive error handling with `plt_status_t` enum (14 error codes)
- Example applications (CAN communication with complete documentation)
- Enhanced API documentation with Doxygen comments
- CI/CD pipeline with GitHub Actions
- Contributing guidelines with coding standards
- Architecture documentation with system diagrams
- CubeMX integration documentation with template project strategy
- Package distribution guide for GitHub Packages deployment
- Python validation script for STM32CubeMX configuration files (`scripts/validate_cubemx_config.py`)

### Changed

- Improved README with better examples, structure, and military-style tone
- Enhanced error propagation throughout the platform
- Standardized API return types across all modules
- Updated all documentation for VS Code as primary IDE
- Added comprehensive parameter validation to prevent buffer overflows

### Fixed

- Memory safety improvements
- Queue overflow protection
- Better error recovery mechanisms

## [1.0.0] - 2025-01-XX

### Added

- Initial release of STM32 Communication Platform
- CAN, UART, SPI, ADC, TIM peripheral support
- DMA-based asynchronous communication
- Queue-based message buffering
- Callback-driven architecture
- In-memory database system
- Hash table for message routing
