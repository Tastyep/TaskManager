message(STATUS "Configuring target '${TESTER_NAME}'")

# Attempt to find threads library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(ResolveGTest)

message(STATUS "TEST_SOURCE_FILES: ${TEST_SOURCE_FILES}")

# Add required unix libraries
list(APPEND EXTERNAL_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

add_executable(${TESTER_NAME} ${TEST_SOURCE_FILES})
target_include_directories(${TESTER_NAME} SYSTEM PUBLIC ${EXTERNAL_TEST_INCLUDE_DIRS})
add_dependencies(${TESTER_NAME} ${EXTERNAL_TEST_DEPENDENCIES})
target_link_libraries (${TESTER_NAME} ${EXTERNAL_TEST_LIBRARIES} ${EXTERNAL_LIBRARIES} ${PROJECT_NAME})
