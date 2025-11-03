cmake_minimum_required(VERSION 4.1.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

project(chip8_emu)

add_executable(${PROJECT_NAME})
add_subdirectory(raylib)
set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
target_link_libraries(${PROJECT_NAME} raylib)

target_sources(${PROJECT_NAME}
    PRIVATE
       chip8.cpp 
)
