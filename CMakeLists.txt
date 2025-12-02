cmake_minimum_required(VERSION 4.1)

project(engine)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_INSTALL_RPATH "/use/local/lib")

find_package(SDL3 REQUIRED)
include_directories(${SDL3_INCLUDE_DIRS})
add_executable(engine src/test.c src/engine.c)
target_link_libraries(engine SDL3 SDL3_image SDL3_ttf)
