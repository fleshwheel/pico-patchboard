cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(patchboard
  patchboard.c
  usb_descriptors.c
)

pico_enable_stdio_usb(patchboard 0)
pico_enable_stdio_uart(patchboard 1)

target_include_directories(patchboard PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(patchboard)
target_link_libraries(patchboard PRIVATE pico_stdlib tinyusb_device tinyusb_board)
