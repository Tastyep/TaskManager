# Enable testing for current directory and below
enable_testing(true)

message(STATUS "Configuring target '${TESTER_NAME}'")

set(TESTER_NAMES ${TESTER_NAME})

add_executable(${TESTER_NAME} ${TEST_SOURCE_FILES})
add_dependencies(${TESTER_NAME} ${EXTERNAL_TEST_DEPENDENCIES})
target_include_directories(${TESTER_NAME} SYSTEM PUBLIC ${EXTERNAL_TEST_INCLUDE_DIRS})
target_link_libraries (${TESTER_NAME} ${EXTERNAL_TEST_LIBRARIES} ${EXTERNAL_LIBRARIES} ${LIB_NAME})

# Register tester with ctest
set(TEST_COMMAND ${TESTER_NAME} --gtest_shuffle)
add_test(NAME ${TESTER_NAME} COMMAND ${TEST_COMMAND})

# Register tester with ctest to generate a XML test report for the CI
set(TEST_REPORT_NAME ${TESTER_NAME}-report)
set(TEST_REPORT_FILE ${TEST_REPORT_NAME}.xml)
set(TEST_REPORT_COMMAND ${TESTER_NAME} "--gtest_output=xml:${TEST_REPORT_FILE}")
add_test(NAME ${TEST_REPORT_NAME} COMMAND ${TEST_REPORT_COMMAND})

add_custom_target(${TEST_REPORT_NAME} DEPENDS ${TEST_REPORT_FILE})

# Enable coverage, coverage-lcov and coverage-gcovr targets depending on tool availability
if(TASK_MANAGER_ENABLE_COVERAGE)
  include(EnableTestCoverage)

  string(REGEX REPLACE "^.*/([^/]*)$" "\\1" BINARY_DIR ${PROJECT_BINARY_DIR})
  set(COVERAGE_TARGET_NAMES ${TESTER_NAMES})
  add_coverage_flags(TARGETS ${COVERAGE_TARGET_NAMES})
  enable_test_coverage(
    FILTER "${TEST_DIR}" "${BINARY_DIR}"
    EXCLUDE_DIRS "build"
    TESTS ${TEST_SOURCE_FILES}
    DEPTARGETS check
    RUNTARGETS ${COVERAGE_TARGET_NAMES}
  )
endif()

# Add custom target to build and run all testers
add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --tests-regex ".*"
  DEPENDS ${TESTER_NAMES})
