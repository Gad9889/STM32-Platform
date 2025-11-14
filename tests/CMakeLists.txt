cmake_minimum_required(VERSION 3.15)
project(STM32_Platform_Tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Unity Test Framework - Download if not present
include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.6.0
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unity
)
FetchContent_MakeAvailable(unity)

# Unity library target
set(UNITY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unity/src)
add_library(unity_framework STATIC
    ${UNITY_DIR}/unity.c
)
target_include_directories(unity_framework PUBLIC
    ${UNITY_DIR}
)

# Platform source files to test
set(PLATFORM_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Src)
set(PLATFORM_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Inc)

# Mock HAL headers
include_directories(
    ${PLATFORM_INC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

# Compile options
add_compile_options(-Wall -Wextra -Wpedantic)

# Add coverage flags for GCC
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

# Helper function to create tests
function(add_platform_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.c ${ARGN})
    target_link_libraries(${TEST_NAME} unity_framework)
    target_include_directories(${TEST_NAME} PRIVATE
        ${PLATFORM_INC_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Test executables
add_platform_test(test_utils 
    ${PLATFORM_SRC_DIR}/utils.c
    mocks/stm32_hal_mocks.c
)

add_platform_test(test_database
    ${PLATFORM_SRC_DIR}/database.c
    ${PLATFORM_SRC_DIR}/DbSetFunctions.c
    mocks/stm32_hal_mocks.c
)

add_platform_test(test_hashtable
    ${PLATFORM_SRC_DIR}/hashtable.c
    ${PLATFORM_SRC_DIR}/DbSetFunctions.c
    ${PLATFORM_SRC_DIR}/database.c
    mocks/stm32_hal_mocks.c
)

# Note: CAN, UART, SPI tests require more extensive mocking
# Uncomment when mocks are ready
# add_platform_test(test_can
#     ${PLATFORM_SRC_DIR}/can.c
#     ${PLATFORM_SRC_DIR}/utils.c
#     mocks/stm32_hal_mocks.c
# )

# Coverage target
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    
    if(LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory coverage
            COMMAND ${LCOV} --directory . --zerocounters
            COMMAND ${CMAKE_CTEST_COMMAND} --verbose
            COMMAND ${LCOV} --directory . --capture --output-file coverage/coverage.info
            COMMAND ${LCOV} --remove coverage/coverage.info '/usr/*' '*/tests/*' '*/unity/*' --output-file coverage/coverage.info
            COMMAND ${GENHTML} coverage/coverage.info --output-directory coverage
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )
    endif()
endif()

# Print configuration
message(STATUS "")
message(STATUS "STM32 Platform Tests Configuration:")
message(STATUS "  Unity directory: ${UNITY_DIR}")
message(STATUS "  Platform source: ${PLATFORM_SRC_DIR}")
message(STATUS "  Platform include: ${PLATFORM_INC_DIR}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
