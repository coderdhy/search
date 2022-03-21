include_directories(${SQLITE3_HEADERS_DIR})
set(SAMPLES_SOURCE_FILES main.cc)

add_executable(simple_cpp_example ${SAMPLES_SOURCE_FILES})
target_link_libraries(simple_cpp_example PRIVATE simple)

add_custom_command(TARGET simple_cpp_example POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/一剑独尊.txt $<TARGET_FILE_DIR:simple_cpp_example>/一剑独尊.txt)
