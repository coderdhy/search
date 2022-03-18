include_directories(${SQLITE3_HEADERS_DIR})
set(SAMPLES_SOURCE_FILES main.cc)

add_executable(simple_cpp_example ${SAMPLES_SOURCE_FILES})
target_link_libraries(simple_cpp_example PRIVATE simple)
