cmake_minimum_required(VERSION 3.15)

message(STATUS "==== ENVIRONMENT VARIABLES ====")
execute_process(COMMAND ${CMAKE_COMMAND} -E environment)
message(STATUS "===============================")


# ---------------------------------------------------------------
# Test executable for development
# ---------------------------------------------------------------

add_executable(testLib main.cpp)

target_include_directories(testLib PRIVATE
    "${CMAKE_SOURCE_DIR}/../../src/cpp/include"
)

target_link_libraries(testLib PRIVATE
    rallyplot  # Name of the .lib file: plottingLib.lib
)
