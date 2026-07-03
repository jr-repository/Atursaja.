cmake_minimum_required(VERSION 3.20)
project(AturSajaHyperOrchestrator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ATURSAJA_BUILD_TESTS "Build the AturSaja test suite" ON)

add_library(atursaja
    src/util/string_utils.cpp
    src/parser/config_parser.cpp
    src/graph/dependency_graph.cpp
    src/scheduling/workflow.cpp
    src/scheduling/scheduler.cpp
    src/simulation/simulator.cpp
    src/analytics/report.cpp
)

target_include_directories(atursaja
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(atursaja
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

add_executable(atursaja-cli src/main.cpp)
target_link_libraries(atursaja-cli PRIVATE atursaja)

if(ATURSAJA_BUILD_TESTS)
    enable_testing()

    add_executable(atursaja-tests tests/run_tests.cpp)
    target_link_libraries(atursaja-tests PRIVATE atursaja)

    add_test(NAME atursaja-tests COMMAND atursaja-tests)
endif()
