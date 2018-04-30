#########
# CMAKE #
#########

cmake_minimum_required(VERSION 2.6.2)

project(seashell)

## set compilation flags
set(CMAKE_C_FLAGS "-W -Wall -Werror -pedantic -std=c99 -std=gnu99 -g")

## set the target name and source
add_executable(
    seashell
    src/main.c
    src/option_manager.c
    src/execution.c
    src/string_fct.c
)

