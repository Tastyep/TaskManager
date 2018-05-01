cmake_minimum_required (VERSION 2.8.1)
cmake_policy(SET CMP0042 NEW)

set(PROJECT_NAME "TaskManager")
project (${PROJECT_NAME} CXX)

# Global variables
set(MODULE_DIR cmake)
set(SOURCE_DIR src)
set(INCLUDE_DIR include)
set(LIB_DIR lib)
set(TEST_DIR test)
set(IMPLEMENTATION_DIR detail)
set(TESTER_NAME "${PROJECT_NAME}-tester")
set(LIB_NAME ${PROJECT_NAME})

set(CLANG_TIDY_CONFIG_SOURCE "${PROJECT_SOURCE_DIR}/.clang-tidy")

# CMake properties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)
set(PREDEFINED_TARGETS_FOLDER "CMake")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/${MODULE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE ${BUILD_TYPE})

message(STATUS "Building ${BUILD_TYPE} ${PROJECT_NAME}\n----------------------------")

add_subdirectory(lib)

# Set compiler flags
include(CompilerFlags)

# Source files.
include(Sources)

include_directories (${PROJECT_SOURCE_DIR})
include_directories (${PROJECT_SOURCE_DIR}/${INCLUDE_DIR})

add_library(${LIB_NAME} STATIC ${LIB_SOURCE_FILES})

if(TASK_MANAGER_ENABLE_CLANGTIDY AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  include(EnableClangTidy)
  enable_clangtidy(SOURCES "${LIB_SOURCE_FILES}")
endif()

if(TASK_MANAGER_BUILD_TESTS)
  include(EnableTests)
endif()

install(TARGETS ${LIB_NAME} DESTINATION ${LIB_DIR})
install(FILES ${LIB_EXPOSED_HEADER_FILES} DESTINATION "${INCLUDE_DIR}/${LIB_NAME}")
