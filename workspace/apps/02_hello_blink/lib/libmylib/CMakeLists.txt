# Specify a minimum CMake version
cmake_minimum_required(VERSION 3.20.0)

# Locate the Zephyr RTOS source
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Name the project
project(
    libmylib
    VERSION 0.1.0
    DESCRIPTION "Say Hello"
    LANGUAGES C
)

# Create a static library target named "mylib"
# add_library(
#     mylib
#     STATIC
#     src/mylib.c
# )
zephyr_library_named(mylib)
zephyr_library_sources(src/mylib.c)

# Set the include directories for the library. PUBLIC adds the directory
# to the search path for any targets that link to this library.
target_include_directories(
    mylib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# # Inherit compile flags used by the Zephyr project
# target_link_libraries(mylib PUBLIC zephyr_interface)
