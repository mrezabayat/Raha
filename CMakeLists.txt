cmake_minimum_required(VERSION 3.26)
project(Raha VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RAHA_ENABLE_TESTS "Build unit tests" ON)
option(RAHA_ENABLE_SANITIZERS "Enable sanitizers for debug builds" OFF)

include(${CMAKE_BINARY_DIR}/conan_deps.cmake OPTIONAL)

if(RAHA_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
endif()

add_subdirectory(src)

if(RAHA_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(DIRECTORY resources DESTINATION share/raha)
