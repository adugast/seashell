#########
# CMAKE #
#########

cmake_minimum_required(VERSION 2.6.2)

project(seashell)

## set compilation flags
set(CMAKE_C_FLAGS "-W -Wall -pedantic -std=c99 -std=gnu99 -g")

## set the target name and source
add_executable(
    seashell
    src/main.c
    src/arguments_manager.c
    src/stream_manager.c
    src/history_manager.c
    src/execution.c
    src/ncurses_proxy.c
    src/parser.c
    src/builtin.c
)

## set the needed library
find_package(Curses REQUIRED)
target_link_libraries(seashell ncurses)

