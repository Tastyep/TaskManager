include(ExternalProject)

# Create build folder name derived from version
string(REGEX REPLACE "beta\\.([0-9])$" "beta\\1" CLANG_TIDY_DIR ${CLANG_TIDY_VERSION})
string(REPLACE "." "_" CLANG_TIDY_DIR ${CLANG_TIDY_DIR})
set(CLANG_TIDY_DIR clang_tidy_${CLANG_TIDY_DIR})
set(CLANG_TIDY_ROOT_DIR ${PROJECT_BINARY_DIR}/${LIB_DIR}/${CLANG_TIDY_DIR})
set(CLANG_TIDY_SCRIPT_DIR "clang-tidy/tool")
set(CLANG_TIDY_TARGET "clang-tidy-download")

# Configure external project
ExternalProject_Add(
  ${CLANG_TIDY_TARGET}
  GIT_REPOSITORY https://github.com/llvm-mirror/clang-tools-extra
  GIT_TAG release_${CLANG_TIDY_RELEASE}
  PREFIX ${CLANG_TIDY_ROOT_DIR}
  TIMEOUT 60
  CONFIGURE_COMMAND ""
  UPDATE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_UPDATE OFF
  LOG_INSTALL OFF
)

set(CLANG_TIDY_SCRIPT_DIR ${CLANG_TIDY_ROOT_DIR}/src/${CLANG_TIDY_TARGET}/${CLANG_TIDY_SCRIPT_DIR})
