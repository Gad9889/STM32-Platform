# GitHub Package Distribution Strategy

## Overview

Platform distributed as GitHub Package for dependency management and version control.

## Distribution Formats

### 1. Git Submodule (Current)

**Status:** Available now

```bash
git submodule add https://github.com/Ben-Gurion-Racing/STM32_Platform.git Drivers/STM32_Platform
git submodule update --init --recursive
```

**Advantages:**

- Source code included
- Easy debugging
- Version pinning via commit hash

**Disadvantages:**

- Manual updates required
- Build system integration not automatic

### 2. CMake FetchContent (Recommended)

**Status:** Requires CMakeLists.txt for library

**Usage:**

```cmake
include(FetchContent)

FetchContent_Declare(
    stm32_platform
    GIT_REPOSITORY https://github.com/Ben-Gurion-Racing/STM32_Platform.git
    GIT_TAG        v1.0.0
)

FetchContent_MakeAvailable(stm32_platform)

target_link_libraries(${PROJECT_NAME} PRIVATE stm32_platform)
```

**Requirements:**

- Platform needs CMakeLists.txt for library building
- Proper target exports
- Installation rules

**Timeline:** Next major task

### 3. GitHub Packages (Container Registry)

**Status:** Evaluation phase

Platform could be published to GitHub Packages as:

- **Source archive** (`.tar.gz` / `.zip`)
- **Pre-compiled library** (ARM GCC builds)
- **Header-only package** (inline implementations)

**Configuration:**

```yaml
# .github/workflows/release.yml
name: Release Package

on:
  release:
    types: [published]

jobs:
  package:
    runs-on: ubuntu-latest
    permissions:
      contents: read
      packages: write

    steps:
      - uses: actions/checkout@v3

      - name: Build library for multiple targets
        run: |
          # Build for F0, F1, F4, F7, H7
          ./scripts/build_all_targets.sh

      - name: Create package
        run: |
          mkdir -p package/{include,lib,examples,docs}
          cp -r Inc/* package/include/
          cp -r build/lib/* package/lib/
          cp -r examples/* package/examples/
          cp -r docs/* package/docs/
          tar -czf stm32-platform-${{ github.ref_name }}.tar.gz package/

      - name: Publish to GitHub Packages
        run: |
          echo "@ben-gurion-racing:registry=https://npm.pkg.github.com" >> .npmrc
          npm publish
        env:
          NODE_AUTH_TOKEN: ${{ secrets.GITHUB_TOKEN }}
```

**Installation:**

```bash
# Download from GitHub Releases
wget https://github.com/Ben-Gurion-Racing/STM32_Platform/releases/download/v1.0.0/stm32-platform-v1.0.0.tar.gz
tar -xzf stm32-platform-v1.0.0.tar.gz
```

### 4. PlatformIO Library

**Status:** Future consideration

Platform could be registered in PlatformIO Library Manager.

**Requirements:**

- Create `library.json`
- Register on PlatformIO Registry
- Maintain compatibility with PlatformIO build system

**Usage:**

```ini
[env:nucleo_f446re]
platform = ststm32
framework = stm32cube
lib_deps =
    ben-gurion-racing/STM32_Platform @ ^1.0.0
```

**Priority:** Low (team uses VS Code + CMake)

## Recommended Approach

### Phase 1: Library CMake Build (HIGH PRIORITY)

Create proper CMake library configuration:

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(STM32_Platform VERSION 1.0.0 LANGUAGES C)

# Library target
add_library(stm32_platform STATIC
    Src/platform.c
    Src/can.c
    Src/uart.c
    Src/spi.c
    Src/adc.c
    Src/tim.c
    Src/database.c
    Src/hashtable.c
    Src/utils.c
    Src/callbacks.c
    Src/platform_status.c
)

target_include_directories(stm32_platform PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Inc>
    $<INSTALL_INTERFACE:include>
)

# User must provide STM32 HAL
target_link_libraries(stm32_platform PUBLIC stm32_hal)

# Installation
install(TARGETS stm32_platform EXPORT STM32PlatformTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY Inc/ DESTINATION include)

install(EXPORT STM32PlatformTargets
    FILE STM32PlatformTargets.cmake
    NAMESPACE STM32Platform::
    DESTINATION lib/cmake/STM32Platform
)

# Package config
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/STM32PlatformConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/STM32PlatformConfig.cmake
    INSTALL_DESTINATION lib/cmake/STM32Platform
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/STM32PlatformConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/STM32PlatformConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/STM32PlatformConfigVersion.cmake
    DESTINATION lib/cmake/STM32Platform
)
```

### Phase 2: GitHub Releases with Assets

Automate release packaging:

1. Create release on GitHub
2. CI builds library for common targets
3. Attach archives to release
4. Users download stable versions

### Phase 3: Package Registry Integration

Publish to GitHub Packages once build system mature.

## Version Management

**Semantic Versioning:** MAJOR.MINOR.PATCH

- **MAJOR:** Breaking API changes
- **MINOR:** New features (backward compatible)
- **PATCH:** Bug fixes

**Tagging Strategy:**

```bash
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0
```

**Changelog:** Maintained in CHANGELOG.md (Keep a Changelog format)

## Distribution Checklist

- [ ] Create library CMakeLists.txt
- [ ] Add version header with `PLATFORM_VERSION_MAJOR/MINOR/PATCH`
- [ ] Create package config files
- [ ] Set up release automation workflow
- [ ] Document installation in README
- [ ] Create migration guide for version updates
- [ ] Test installation on clean system

## Current Status

**Distribution Method:** Git clone / manual copy

**Next Step:** Implement library CMake build system (see todo list)

---

## References

- [CMake Package Documentation](https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html)
- [GitHub Packages](https://docs.github.com/en/packages)
- [Semantic Versioning](https://semver.org/)
- [PlatformIO Library Spec](https://docs.platformio.org/en/latest/librarymanager/creating.html)
