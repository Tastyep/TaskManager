include(ExternalProject)

# Create build folder name derived from version
string(REGEX REPLACE "beta\\.([0-9])$" "beta\\1" GTEST_DIR ${GTEST_VERSION})
string(REPLACE "." "_" GTEST_DIR ${GTEST_DIR})
set(GTEST_DIR gtest_${GTEST_DIR})
set(GTEST_ROOT_DIR ${PROJECT_BINARY_DIR}/${LIB_DIR}/${GTEST_DIR})

set(GTEST_C_COMPILER "${CMAKE_C_COMPILER}")
set(GTEST_CXX_COMPILER "${CMAKE_CXX_COMPILER}")
set(GTEST_C_FLAGS "")
set(GTEST_CXX_FLAGS "")
set(GTEST_LINKER_FLAGS "")
set(GTEST_DEPENDENCIES "")

# Configure external project
ExternalProject_Add(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-${GTEST_VERSION}
  PREFIX ${GTEST_ROOT_DIR}
  TIMEOUT 60
  CONFIGURE_COMMAND ${CMAKE_COMMAND}
    "-DCMAKE_POLICY_DEFAULT_CMP0056=NEW" # use link flags in cmake try_compile
    "-DCMAKE_INSTALL_PREFIX=${GTEST_ROOT_DIR}"
    "-DCMAKE_C_COMPILER=${GTEST_C_COMPILER}"
    "-DCMAKE_CXX_COMPILER=${GTEST_CXX_COMPILER}"
    "-DCMAKE_C_FLAGS=${GTEST_C_FLAGS}"
    "-DCMAKE_CXX_FLAGS=${GTEST_CXX_FLAGS}"
    "-DCMAKE_EXE_LINKER_FLAGS=${GTEST_LINKER_FLAGS}"
    <SOURCE_DIR>
  UPDATE_COMMAND ""
  BUILD_COMMAND $(MAKE)
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_UPDATE OFF
  LOG_INSTALL OFF
)

# Label target as third-party
set_target_properties(gtest PROPERTIES LABELS GoogleTest FOLDER "Third Party")

# Define dependency directories, mimic the needed behavior of find_package
# NOTE: Variable names must respect the conventions of a cmake find-module
set(GTest_VERSION ${GTEST_VERSION})
set(GTest_INCLUDE_DIRS ${GTEST_ROOT_DIR}/include)
set(GTest_LIBRARY_DIRS ${GTEST_ROOT_DIR}/lib)
set(GTest_LIBRARIES ${GTest_LIBRARY_DIRS}/libgtest.a ${GTest_LIBRARY_DIRS}/libgmock.a)
set(GTest_TARGETS gtest)
