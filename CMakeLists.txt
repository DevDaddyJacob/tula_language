cmake_minimum_required(VERSION 4.2)

project(tula C)

################################################################################
# Define the global settings for the build
################################################################################

# Define the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define the compiler flags
add_compile_options(-Wall -Wextra)

# Enable Unity memory extension before adding the subdirectory
set(UNITY_EXTENSION_MEMORY ON CACHE BOOL "" FORCE)


################################################################################
# Define the library for the tula program
################################################################################

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
        src/engine/scanner/scanner.c
        src/engine/scanner/scanner.h
)
target_include_directories(tula_lib PUBLIC src)


################################################################################
# Define the standard tula cli executable (named "tula")
################################################################################

add_executable(tula src/tula.c)

target_link_libraries(tula PRIVATE tula_lib)

target_compile_options(tula PRIVATE -Werror)

target_compile_definitions(tula PRIVATE TULA_EXECUTABLE_TYPE=0)


################################################################################
# Define the debugging tula cli executable (named "tulad")
################################################################################

add_executable(tulad src/tula.c)

target_link_libraries(tulad PRIVATE tula_lib)

target_compile_options(tulad PRIVATE)

target_compile_definitions(tulad PRIVATE TULA_EXECUTABLE_TYPE=1)


################################################################################
# Define additional targets
################################################################################

# Process CMakeLists from subdirectories
add_subdirectory(vendor/unity)
add_subdirectory(test/integration_tests)
add_subdirectory(test/unit_tests)