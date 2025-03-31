# Generated Cmake Pico project file for Client

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Ensure the PICO_SDK_PATH environment variable is set or provide a fallback path
if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH "/path/to/pico-sdk" CACHE PATH "Path to the Pico SDK")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../pico_sdk_import.cmake)

project(client C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable for the client project
add_executable(client client.cpp motor.cpp)

pico_set_program_name(client "client")
pico_set_program_version(client "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(client 0)
pico_enable_stdio_usb(client 1)

# Add the standard library and hardware_gpio library to the build
target_link_libraries(client pico_stdlib hardware_gpio)

# Add the standard include files to the build
target_include_directories(client PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(client)
