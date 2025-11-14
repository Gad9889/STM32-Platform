cmake_minimum_required(VERSION 3.15)
project(ApplicationTests C)

# Fetch Unity test framework
include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.6.0
)
FetchContent_MakeAvailable(unity)

# Test compiler flags
set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Wextra)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${unity_SOURCE_DIR}/src
)

# Your application logic source files (NOT platform files)
# Add your modules that contain business logic here
set(APP_SOURCES
    # Example: ${CMAKE_SOURCE_DIR}/Core/Src/state_machine.c
    # Example: ${CMAKE_SOURCE_DIR}/Core/Src/vehicle_control.c
    # Example: ${CMAKE_SOURCE_DIR}/Core/Src/sensor_processing.c
)

# Test executables - one per module you want to test
# Example: Testing a state machine module
# add_executable(test_state_machine test_state_machine.c ${CMAKE_SOURCE_DIR}/Core/Src/state_machine.c)
# target_link_libraries(test_state_machine unity)
# add_test(NAME state_machine_tests COMMAND test_state_machine)

# Enable testing
enable_testing()

# Add your test executables here
# Example template (uncomment and modify):
# add_executable(test_my_module test_my_module.c ${CMAKE_SOURCE_DIR}/Core/Src/my_module.c)
# target_link_libraries(test_my_module unity)
# add_test(NAME my_module_tests COMMAND test_my_module)
