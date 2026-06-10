function(integration_test test_name test_source)
#    add_executable(${test_name} ${test_source})
#    target_link_libraries(${test_name} PRIVATE tula_lib unity)
#    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
#
#    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

#integration_test(integration_test_scanner  test_buffered_reader.c)

add_custom_target(integration_tests
        COMMAND
        ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS
#        integration_test_scanner
)