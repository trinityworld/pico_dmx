cmake_minimum_required(VERSION 3.19)

include(pico_sdk_import.cmake)

project(rp2040_dmx)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME}
        main.c
        )

pico_generate_pio_header(${CMAKE_PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/tx16.pio
)

target_link_libraries(${PROJECT_NAME}
		pico_stdlib
        hardware_dma
        hardware_irq
        hardware_pio
        )

pico_add_extra_outputs(${CMAKE_PROJECT_NAME})

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# add url via pico_set_program_url
#example_auto_set_url(${PROJECT_NAME})

#if (NOT PICO_NO_HARDWARE)
#    add_subdirectory(channel_irq)
#    add_subdirectory(control_blocks)
#    add_subdirectory(hello_dma)
#endif ()