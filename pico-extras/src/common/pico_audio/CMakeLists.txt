# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

if (NOT TARGET pico_audio_headers)
    add_library(pico_audio_headers INTERFACE)
    target_include_directories(pico_audio_headers INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
    target_link_libraries(pico_audio_headers INTERFACE pico_util_buffer)
endif()

if (NOT TARGET pico_audio)
    add_library(pico_audio INTERFACE)

    target_sources(pico_audio INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/audio.cpp
    )
    if (NOT PICO_NO_HARDWARE AND NOT PICO_RISCV)
        target_sources(pico_audio INTERFACE
                ${CMAKE_CURRENT_LIST_DIR}/audio_utils.S
        )
    endif()

    target_link_libraries(pico_audio INTERFACE pico_audio_headers pico_sync)
endif()
