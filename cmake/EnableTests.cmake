message(STATUS "Configuring target '${TESTER_NAME}'")

add_executable(${TESTER_NAME} ${TEST_SOURCE_FILES})
add_dependencies(${TESTER_NAME} ${EXTERNAL_TEST_DEPENDENCIES})
target_include_directories(${TESTER_NAME} SYSTEM PUBLIC ${EXTERNAL_TEST_INCLUDE_DIRS})
target_link_libraries (${TESTER_NAME} ${EXTERNAL_TEST_LIBRARIES} ${EXTERNAL_LIBRARIES} ${LIB_NAME})
