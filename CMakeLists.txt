cmake_minimum_required(VERSION 3.20.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(raycast)

find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-image CONFIG REQUIRED)

add_executable(raycast
    src/main.c
    src/prog.h
    src/prog.c
    src/player.h
    src/player.c
    src/common.h
    src/common.c
    src/map.h
    src/map.c
    src/entity.h
    src/entity.c
)

target_link_libraries(raycast PRIVATE
    SDL2::SDL2 SDL2::SDL2main
    SDL2::SDL2_image
)
target_compile_options(raycast PRIVATE -ggdb)

