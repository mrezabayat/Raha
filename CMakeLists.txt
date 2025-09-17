cmake_minimum_required(VERSION 3.26)
project(VideoPlayer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VPLAYER_ENABLE_TESTS "Build unit tests" ON)
option(VPLAYER_ENABLE_SANITIZERS "Enable sanitizers for debug builds" OFF)

include(${CMAKE_BINARY_DIR}/conan_deps.cmake OPTIONAL)

if(VPLAYER_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
endif()

add_subdirectory(src)

if(VPLAYER_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(DIRECTORY resources DESTINATION share/video-player)
