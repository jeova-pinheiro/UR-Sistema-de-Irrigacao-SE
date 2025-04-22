# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
project(UR-Sistema-de-Irrigacao-SE C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(
    UR-Sistema-de-Irrigacao-SE UR-Sistema-de-Irrigacao-SE.c  
    pio_matrix.pio
    lib/ssd1306.c)

pico_set_program_name(UR-Sistema-de-Irrigacao-SE "UR-Sistema-de-Irrigacao-SE")
pico_set_program_version(UR-Sistema-de-Irrigacao-SE "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(UR-Sistema-de-Irrigacao-SE 1)
pico_enable_stdio_usb(UR-Sistema-de-Irrigacao-SE 1)

# Gerar o header do programa PIO corretamente:
pico_generate_pio_header(UR-Sistema-de-Irrigacao-SE ${CMAKE_CURRENT_LIST_DIR}/pio_matrix.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(UR-Sistema-de-Irrigacao-SE 0)
pico_enable_stdio_usb(UR-Sistema-de-Irrigacao-SE 1)

# Add the standard library to the build
target_link_libraries(UR-Sistema-de-Irrigacao-SE
        pico_stdlib 
        hardware_adc 
        hardware_pwm 
        hardware_i2c
        hardware_pio
        hardware_uart
      )

# Add the standard include files to the build
target_include_directories(UR-Sistema-de-Irrigacao-SE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(UR-Sistema-de-Irrigacao-SE 

        )

pico_add_extra_outputs(UR-Sistema-de-Irrigacao-SE)

