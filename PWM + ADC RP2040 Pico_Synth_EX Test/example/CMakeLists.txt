# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion RISCV_RPI_2_0_0_5)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.12)
 
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
 
project(pico_synth_ex_example C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
 
pico_sdk_init()

add_executable(${PROJECT_NAME}
        example.c
        )

add_subdirectory(.. pico_synth_ex_i2s)

target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        pico_synth_ex_i2s
        hardware_adc
        hardware_clocks
        )

target_compile_definitions(${PROJECT_NAME} PRIVATE
        # Define only one audio output
        USE_AUDIO_PWM=1
        # USE_AUDIO_I2S=1
        )

pico_set_program_name(${PROJECT_NAME} ${PROJECT_NAME})

pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)