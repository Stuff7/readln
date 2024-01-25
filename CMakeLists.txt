cmake_minimum_required(VERSION 3.10)
project(readln)

set(H "$ENV{HOME}")
set(CMAKE_C_STANDARD 99)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_options(
  -g
  -fsanitize=address
  -Wall
)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

file(GLOB SOURCES "src/*.c")

add_executable(readln ${SOURCES})
