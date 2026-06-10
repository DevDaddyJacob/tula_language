function(unit_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE tula_lib unity)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)

    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

unit_test(unit_test_buffered_reader test_buffered_reader.c)
unit_test(unit_test_strings         test_strings.c)
unit_test(unit_test_token           test_token.c)

add_custom_target(unit_tests
        COMMAND
        ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS
        unit_test_buffered_reader
        unit_test_strings
        unit_test_token
)