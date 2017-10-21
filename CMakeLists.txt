#########
# CMAKE #
#########

cmake_minimum_required(VERSION 2.8.3)

project(seashell)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Werror -pedantic")

find_package(Curses REQUIRED)

include_directories(${CURSES_INCLUDE_DIR})

## Target
add_executable(
    seashell
    src/main.c
    src/string_fct.c
)

target_link_libraries(seashell ${CURSES_LIBRARIES})
