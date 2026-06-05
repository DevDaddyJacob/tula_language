cmake_minimum_required(VERSION 4.2)
project(tula C)

set(CMAKE_C_STANDARD 99)

#include(CTest)

# Enable Unity memory extension before adding the subdirectory
set(UNITY_EXTENSION_MEMORY ON CACHE BOOL "" FORCE)

# All source as a library so tests can link against it
add_library(tula_lib STATIC
        src/config.h
        src/tula.h
        src/util.c
        src/util.h
        src/state/cli.c
        src/state/cli.h
        src/common/strings.c
        src/common/strings.h
        src/state/gstate.c
        src/state/gstate.h
        src/engine/scanner/token.c
        src/engine/scanner/token.h
        src/common/buffered_reader.c
        src/common/buffered_reader.h
        src/common/os.c
        src/common/os.h
)
target_include_directories(tula_lib PUBLIC src)

# Main executable just provides main()
add_executable(tula src/tula.c)
target_link_libraries(tula PRIVATE tula_lib)

add_subdirectory(vendor/unity)
add_subdirectory(test)

add_custom_target(run_all_tests
        COMMAND
            ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS
            test_buffered_reader
            test_strings
            test_token
)