add_library(integration_test_lib STATIC
        integration_test_helper.h
        integration_test_helper.c
)

target_compile_definitions(integration_test_lib PRIVATE
        TULAD_EXE_PATH="${CMAKE_BINARY_DIR}/tulad.exe"
        INTEGRATION_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

function(integration_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE integration_test_lib unity)
    target_compile_definitions(${test_name} PRIVATE
            TULAD_EXE_PATH="${CMAKE_BINARY_DIR}/tulad.exe"
            INTEGRATION_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

integration_test(it_scanner_tokens  it_scanner_tokens.c)

add_custom_target(integration_tests
        COMMAND
        ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS
        it_scanner_tokens
)

