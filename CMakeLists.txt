cmake_minimum_required(VERSION 3.10)
project(hc-sr04)

# Require the C compiler to support C11 standard. This ensures that the code is
# compiled with the appropriate language features and standards. GCC version 15
# narrows the default C standard to C17, which may cause compatibility issues
# with code that relies on C11 features, such as the interpretation of empty
# arguments. C11 allows for a more flexible handling of empty arguments.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
)

# Find libgpiod and hiredis libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
pkg_check_modules(HIREDIS REQUIRED hiredis)

add_executable(hc-sr04
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gpio_line.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gpiochip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/dirent_ext.c
    ${CMAKE_CURRENT_SOURCE_DIR}/redis.c
    ${CMAKE_CURRENT_SOURCE_DIR}/xadd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sleep.c
    ${CMAKE_CURRENT_SOURCE_DIR}/call_at_exit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/call_at.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cons.c
)
target_include_directories(hc-sr04 PRIVATE . "${CMAKE_CURRENT_BINARY_DIR}")

target_include_directories(hc-sr04 PRIVATE ${GPIOD_INCLUDE_DIRS})
target_link_libraries(hc-sr04 PRIVATE ${GPIOD_LIBRARIES})

target_include_directories(hc-sr04 PRIVATE ${HIREDIS_INCLUDE_DIRS})
target_link_libraries(hc-sr04 PRIVATE ${HIREDIS_LIBRARIES})

# Install the executable to /usr/local/bin
install(TARGETS hc-sr04 DESTINATION bin)
