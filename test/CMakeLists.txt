# Buffered Reader Tests
add_executable(test_buffered_reader test_buffered_reader.c)
target_link_libraries(test_buffered_reader PRIVATE tula_lib unity)
target_include_directories(test_buffered_reader PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME test_buffered_reader COMMAND test_buffered_reader)


# String Tests
add_executable(test_strings test_strings.c)
target_link_libraries(test_strings PRIVATE tula_lib unity)
target_include_directories(test_strings PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME test_strings COMMAND test_strings)


# Token Tests
add_executable(test_token test_token.c)
target_link_libraries(test_token PRIVATE tula_lib unity)
target_include_directories(test_token PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME test_token COMMAND test_token)
