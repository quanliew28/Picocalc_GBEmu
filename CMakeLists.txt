# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Pico_GBemu C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Pico_GBemu Pico_GBemu.c 
drivers/lcd.c
drivers/lcd.h
lib/peanut_gb.h)

pico_set_program_name(Pico_GBemu "Pico_GBemu")
pico_set_program_version(Pico_GBemu "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Pico_GBemu 1)
pico_enable_stdio_usb(Pico_GBemu 1)

# Add the standard library to the build
target_link_libraries(Pico_GBemu
        pico_stdlib 
        hardware_spi 
        hardware_i2c
        hardware_gpio
        pico_multicore
        hardware_dma
        )

# Add the standard include files to the build
target_include_directories(Pico_GBemu PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(Pico_GBemu)

