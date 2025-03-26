# Minimum CMake version
cmake_minimum_required(VERSION 3.20.0)

# Locate the Zephyr RTOS source
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Name the project
project(
    hello_blink
    VERSION 0.1.0
    DESCRIPTION "Hello while blinking"
    LANGUAGES C
)

add_subdirectory(lib/libmylib)

# Locate the source code for the application
target_sources(app PRIVATE src/main.c)

# Link the library to the executable. PRIVATE means that the library is not
# exposed to targets that depend on this target.
zephyr_library_link_libraries(mylib)
