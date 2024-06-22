# https://stackoverflow.com/questions/6352123/how-to-migrate-this-make-and-autotools-project-with-multiple-source-subdirectori
cmake_minimum_required(VERSION 3.8)
set(CMAKE_VERBOSE_MAKEFILE ON)
project(Mandelbrot LANGUAGES C)

link_directories("/opt/homebrew/lib")
include_directories("/opt/homebrew/include")

set(target_source_files
    build.c
    wgd/tinycthread/tinycthread.c
    wgd/uastar/uastar.c
)

add_executable(far ${target_source_files})
target_compile_options(far PRIVATE -Wall -Wextra)

target_link_libraries(far raylib)
