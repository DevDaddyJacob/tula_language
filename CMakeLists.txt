cmake_minimum_required(VERSION 4.2)
project(tula C)

set(CMAKE_C_STANDARD 99)

add_executable(tula
        src/config.h
        src/tula.c
        src/tula.h
        src/util.c
        src/util.h
        src/state/cli.c
        src/state/cli.h
        src/common/strings.c
        src/common/strings.h
        src/state/gstate.c
        src/state/gstate.h
        src/engine/scanner/scanner.c
        src/engine/scanner/scanner.h
        src/common/os.c
        src/common/os.h
)
