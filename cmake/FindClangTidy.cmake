# Find-module for clang-tidy
# Defines:
#
# ClangTidy_FOUND - clang-tidy is available
# ClangTidy_EXECUTABLE - the clang-tidy binary
# ClangTidy_VERSION - the clang-tidy version

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

include(FindPackageHandleStandardArgs)

find_program(ClangTidy_EXECUTABLE NAMES clang-tidy HINTS ${ClangTidy_HINT_PATH})
if(ClangTidy_EXECUTABLE)
  execute_process(COMMAND ${ClangTidy_EXECUTABLE} --version OUTPUT_VARIABLE ClangTidy_VERSION)

  find_program(RunClangTidy_EXECUTABLE NAMES run-clang-tidy.py)
  if(RunClangTidy_EXECUTABLE)
    set(ClangTidy_FOUND)
  endif()
endif()

find_package_handle_standard_args(ClangTidy DEFAULT_MSG RunClangTidy_EXECUTABLE ClangTidy_VERSION)

# only visible in advanced view
mark_as_advanced(ClangTidy_EXECUTABLE ClangTidy_VERSION)
