
cmake_minimum_required(VERSION 3.16)

project(NexusNet VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_library(nexusnet_lib
    src/TcpServer.cpp
    src/TcpClient.cpp
)

target_include_directories(nexusnet_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(nexusnet_lib
    PUBLIC
        Threads::Threads
)

target_compile_options(nexusnet_lib PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(nexusnet_server
    src/server_main.cpp
)

target_link_libraries(nexusnet_server
    PRIVATE nexusnet_lib
)

add_executable(nexusnet_client
    src/client_main.cpp
)

target_link_libraries(nexusnet_client
    PRIVATE nexusnet_lib
)

enable_testing()

add_executable(nexusnet_tests
    tests/test_nexusnet.cpp
)

target_link_libraries(nexusnet_tests
    PRIVATE nexusnet_lib
)

add_test(
    NAME NexusNetTests
    COMMAND nexusnet_tests
)
