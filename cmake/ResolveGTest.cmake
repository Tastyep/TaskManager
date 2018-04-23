# Define google-test dependency
set(GTEST_VERSION 1.8.0)

message(STATUS "[gtest] Configuring external project")

# Resolve and compile google-test
include(ExternalGTest)

# Register google-test dependency
list(APPEND EXTERNAL_TEST_DEPENDENCIES ${GTest_TARGETS})

list(APPEND EXTERNAL_TEST_INCLUDE_DIRS ${GTest_INCLUDE_DIRS})
list(APPEND EXTERNAL_TEST_LIBRARY_DIRS ${GTest_LIBRARY_DIRS})
list(APPEND EXTERNAL_TEST_LIBRARIES ${GTest_LIBRARIES})
message(STATUS "[gtest] Using ${GTest_VERSION}")
