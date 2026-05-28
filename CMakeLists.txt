cmake_minimum_required(VERSION 3.10)
project(hc-sr04)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
)

# Find the libgpiod library. Typically, install the libgpiod-dev package on your
# system to get the necessary headers and library files.
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

add_executable(hc-sr04 main.c dev.c)
target_include_directories(hc-sr04 PRIVATE . "${CMAKE_CURRENT_BINARY_DIR}")

target_include_directories(hc-sr04 PRIVATE ${GPIOD_INCLUDE_DIRS})
target_link_libraries(hc-sr04 PRIVATE ${GPIOD_LIBRARIES})
