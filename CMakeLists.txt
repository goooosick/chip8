cmake_minimum_required(VERSION 3.12)

project(chip8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include_directories(include/)
file(GLOB SOURCE "src/*.cc")

add_executable(chip8 src/main.cc ${SOURCE})

find_package(SDL2)
target_link_libraries(chip8 mingw32 SDL2main SDL2)
