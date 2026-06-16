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

# Find the hiredis library. Typically, install the libhiredis-dev package on your
# system to get the necessary headers and library files.
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS hiredis REQUIRED)

add_executable(hc-sr04
    main.c
    gpio.c
    gpiochip.c
    dirent_ext.c
    redis.c
    xadd.c
    pr.c
    sleep.c
    call_at_exit.c
    call_at.c
    cons.c
)
target_include_directories(hc-sr04 PRIVATE . "${CMAKE_CURRENT_BINARY_DIR}")

target_include_directories(hc-sr04 PRIVATE ${GPIOD_INCLUDE_DIRS})
target_link_libraries(hc-sr04 PRIVATE ${GPIOD_LIBRARIES})

target_include_directories(hc-sr04 PRIVATE ${HIREDIS_INCLUDE_DIRS})
target_link_libraries(hc-sr04 PRIVATE ${HIREDIS_LIBRARIES})

# Install the executable to /usr/local/bin
install(TARGETS hc-sr04 DESTINATION bin)
