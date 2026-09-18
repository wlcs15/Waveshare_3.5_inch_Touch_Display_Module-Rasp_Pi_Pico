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
set(PICO_BOARD pico_w CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.12)
include(${CMAKE_CURRENT_LIST_DIR}/../../pico_sdk_import.cmake)
project(pico_lcd_on_target_tests)
pico_sdk_init()

set(ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

add_executable(on_target_tests
  main.c
  ${ROOT}/tests/test_bmp_policy.c
  ${ROOT}/lib/quality/bmp_policy.c
  ${ROOT}/lib/quality/string_s.c
  ${ROOT}/third_party/Unity/src/unity.c
)
target_compile_definitions(on_target_tests PRIVATE
  BMP_POLICY_TESTS_NO_MAIN
  PICO_STDIO_USB_CONNECT_WAIT_MS=15000)
target_include_directories(on_target_tests PRIVATE
  ${ROOT}/lib/quality
  ${ROOT}/third_party/Unity/src
  ${ROOT}/tests
)
target_link_libraries(on_target_tests pico_stdlib)
pico_enable_stdio_usb(on_target_tests 1)
pico_enable_stdio_uart(on_target_tests 0)
pico_add_extra_outputs(on_target_tests)

option(PICO_TEST_COVERAGE "Compile on-target tests with GCC --coverage (gcno only on device)" OFF)
if(PICO_TEST_COVERAGE)
  target_compile_options(on_target_tests PRIVATE --coverage -fno-exceptions)
  target_link_options(on_target_tests PRIVATE --coverage)
endif()
