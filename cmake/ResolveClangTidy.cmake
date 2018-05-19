# Define clang-tidy dependency
set(CLANG_TIDY_RELEASE 4.0)
set(CLANG_TIDY_VERSION ${CLANG_TIDY_DEFAULT_VERSION})
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CLANG_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
  string(REGEX MATCH "[0-9]+\\.[0-9]" CLANG_TIDY_VERSION ${CLANG_COMPILER_VERSION})
endif()
string(REPLACE "." "" CLANG_TIDY_RELEASE ${CLANG_TIDY_RELEASE})

message(STATUS "[clang-tidy] Configuring external project")

# Resolve and clang-tidy
include(ExternalClangTidy)

# Set the path of the script to the parent scope
set(CLANG_TIDY_SCRIPT_DIR ${CLANG_TIDY_SCRIPT_DIR} PARENT_SCOPE)

message(STATUS "[clang-tidy] Using ${CLANG_TIDY_VERSION}")
