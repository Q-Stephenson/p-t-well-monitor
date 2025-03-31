# Generated Cmake Pico project file for host

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Ensure the PICO_SDK_PATH environment variable is set or provide a fallback path
if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH "/path/to/pico-sdk" CACHE PATH "Path to the Pico SDK")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../pico_sdk_import.cmake)

project(host C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable for the host project
add_executable(host host.cpp)

pico_set_program_name(host "host")
pico_set_program_version(host "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(host 0)
pico_enable_stdio_usb(host 1)

# Add the standard library to the build
target_link_libraries(host pico_stdlib)

# Add the standard include files to the build
target_include_directories(host PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(host)
