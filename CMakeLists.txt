cmake_minimum_required(VERSION 3.15)

# Project definition
project(STM32_Platform 
    VERSION 1.0.0
    DESCRIPTION "Production-grade STM32 HAL Communication Platform"
    LANGUAGES C
)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(PLT_BUILD_TESTS "Build unit tests" OFF)
option(PLT_BUILD_EXAMPLES "Build example applications" OFF)

# C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ==================== Library Sources ====================

set(CORE_SOURCES
    Src/stm32_platform.c
    Src/callbacks.c
    Src/utils.c
    Src/platform_status.c
)

set(DATABASE_SOURCES
    Src/database.c
    Src/DbSetFunctions.c
    Src/hashtable.c
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${DATABASE_SOURCES}
)

# ==================== Library Target ====================

add_library(stm32_platform ${ALL_SOURCES})
add_library(STM32Platform::platform ALIAS stm32_platform)

# Include directories
target_include_directories(stm32_platform PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Inc>
    $<INSTALL_INTERFACE:include>
)

# Compiler definitions
target_compile_definitions(stm32_platform PUBLIC
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Compiler options
target_compile_options(stm32_platform PRIVATE
    $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<C_COMPILER_ID:MSVC>:/W4>
)

# STM32 HAL dependency (user must provide)
# Platform requires STM32 HAL headers and libraries
# User should add via target_link_libraries() in their project

# ==================== Installation ====================

include(GNUInstallDirs)

# Install library
install(TARGETS stm32_platform
    EXPORT STM32PlatformTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY Inc/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stm32_platform
    FILES_MATCHING PATTERN "*.h"
)

# Install CMake config files
install(EXPORT STM32PlatformTargets
    FILE STM32PlatformTargets.cmake
    NAMESPACE STM32Platform::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/STM32Platform
)

# ==================== Package Configuration ====================

include(CMakePackageConfigHelpers)

# Generate config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/STM32PlatformConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/STM32PlatformConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/STM32Platform
)

# Generate version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/STM32PlatformConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/STM32PlatformConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/STM32PlatformConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/STM32Platform
)

# ==================== Tests ====================

if(PLT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ==================== Examples ====================

if(PLT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ==================== Package Generation ====================

set(CPACK_PACKAGE_NAME "STM32_Platform")
set(CPACK_PACKAGE_VENDOR "Ben Gurion Racing")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Production-grade STM32 HAL Communication Platform")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    /.git
    /build
    /.vscode
    *.user
    *.bak
)

include(CPack)

# ==================== Summary ====================

message(STATUS "")
message(STATUS "STM32 Platform Configuration:")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler:     ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build tests:    ${PLT_BUILD_TESTS}")
message(STATUS "  Build examples: ${PLT_BUILD_EXAMPLES}")
message(STATUS "")
