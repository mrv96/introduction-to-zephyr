cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_SOURCE_DIR}/../../modules/ilps28qsw")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(i2c_driver)

target_sources(app PRIVATE src/main.c)

target_compile_options(app PRIVATE
    -Wall
    -Wextra
    -Werror
)
