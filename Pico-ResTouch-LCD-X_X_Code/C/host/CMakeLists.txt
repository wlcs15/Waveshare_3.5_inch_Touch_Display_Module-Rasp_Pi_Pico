cmake_minimum_required(VERSION 3.10)
project(pico_lcd_host C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-std=c11>)
endif()

option(RR_ENABLE_COVERAGE
       "Clang source-based coverage (llvm-cov). Host tests only." OFF)

set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

if(NOT EXISTS "${ROOT}/third_party/Unity/src/unity.c")
  message(FATAL_ERROR "Unity missing at third_party/Unity/src/unity.c")
endif()

add_library(unity_host STATIC "${ROOT}/third_party/Unity/src/unity.c")
target_include_directories(unity_host PUBLIC "${ROOT}/third_party/Unity/src")

add_library(quality_host STATIC
  "${ROOT}/lib/quality/bmp_policy.c"
  "${ROOT}/lib/quality/string_s.c")
target_include_directories(quality_host PUBLIC "${ROOT}/lib/quality")

add_executable(pico_lcd_tests "${ROOT}/tests/test_bmp_policy.c")
target_include_directories(pico_lcd_tests PRIVATE
  "${ROOT}/lib/quality"
  "${ROOT}/third_party/Unity/src"
  "${ROOT}/tests")
target_link_libraries(pico_lcd_tests PRIVATE unity_host quality_host)

if(RR_ENABLE_COVERAGE)
  if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "RR_ENABLE_COVERAGE requires Clang (llvm-cov), not GCC/MSVC.")
  endif()
  find_program(LLVM_PROFDATA NAMES llvm-profdata llvm-profdata.exe)
  find_program(LLVM_COV NAMES llvm-cov llvm-cov.exe)
  if(NOT LLVM_PROFDATA OR NOT LLVM_COV)
    message(FATAL_ERROR "RR_ENABLE_COVERAGE needs llvm-profdata and llvm-cov on PATH")
  endif()

  target_compile_options(pico_lcd_tests PRIVATE
                         -fprofile-instr-generate -fcoverage-mapping)
  target_link_options(pico_lcd_tests PRIVATE
                      -fprofile-instr-generate -fcoverage-mapping)
  target_compile_options(quality_host PRIVATE
                         -fprofile-instr-generate -fcoverage-mapping)

  set(_profraw "${CMAKE_BINARY_DIR}/pico_lcd_tests.profraw")
  set(_profdata "${CMAKE_BINARY_DIR}/pico_lcd_tests.profdata")
  set(_htmldir "${CMAKE_BINARY_DIR}/coverage")
  set(_libdir "${ROOT}/lib/quality")

  add_custom_target(coverage
    DEPENDS pico_lcd_tests
    COMMAND ${CMAKE_COMMAND} -E env
            "LLVM_PROFILE_FILE=${_profraw}"
            "$<TARGET_FILE:pico_lcd_tests>"
    COMMAND "${LLVM_PROFDATA}" merge -sparse "${_profraw}" -o "${_profdata}"
    COMMAND "${LLVM_COV}" report
            "$<TARGET_FILE:pico_lcd_tests>"
            "-instr-profile=${_profdata}"
            "-ignore-filename-regex=third_party|Unity"
            "${_libdir}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_htmldir}"
    COMMAND "${LLVM_COV}" show
            "$<TARGET_FILE:pico_lcd_tests>"
            "-instr-profile=${_profdata}"
            -format=html
            "-output-dir=${_htmldir}"
            "-ignore-filename-regex=third_party|Unity"
            "${_libdir}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Unity + llvm-cov for lib/quality (HTML in ${CMAKE_BINARY_DIR}/coverage)"
    USES_TERMINAL
    VERBATIM)
endif()
