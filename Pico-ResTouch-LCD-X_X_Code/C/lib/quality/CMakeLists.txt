add_library(quality bmp_policy.c string_s.c)
target_include_directories(quality PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
