function(regression_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE tula_lib unity)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)

    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

regression_test(rt_0000_example    0000_example.c)

add_custom_target(regression_tests
        COMMAND
        ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS
        rt_0000_example
)