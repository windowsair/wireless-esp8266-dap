cmake_minimum_required(VERSION 3.5)
# Colored  Compiler Output with ninja
add_compile_options (-fdiagnostics-color=always)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(wireless_esp_dap)
