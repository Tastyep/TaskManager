include(CMakeParseArguments)

# - Adds the coverage compilation flags to the given targets.
#
# add_coverage_flags(TARGETS targets...)
function(add_coverage_flags)
  cmake_parse_arguments(ARG "" "" "TARGETS" ${ARGN})

  # Add coverage compile flags
  set_target_properties(${ARG_TARGETS} PROPERTIES COMPILE_FLAGS "--coverage"
                                                  LINK_FLAGS "--coverage")
  message(STATUS "[${ARG_TARGETS}] Add coverage flags")
endfunction()

# - Creates a special coverage build type and target on GCC.
#
# Defines a function enable_test_coverage which sets up code coverage generation.
# Optional arguments to this function can be used to filter unwanted results in
# the final report and to exclude coverage data from specific directories.
# Moreover targets with tests for the source code can be specified to trigger regenerating
# the report if the test has changed.
#
# enable_test_coverage([FILTER directories...]
#                      [EXCLUDE_DIRS directories...]
#                      [TESTS test targets...]
#                      [DEPTARGETS targets...]
#                      [RUNTARGETS targets...])
#
# The coverage report is based on gcov. Depending on the availability of lcov
# a HTML report will be generated and/or an XML report of gcovr is found.
# The generated coverage target executes all found solutions. Special targets
# exist to create e.g. only the xml report: coverage-xml.
function(enable_test_coverage)
  cmake_parse_arguments(ARG "" "" "FILTER;EXCLUDE_DIRS;DEPTARGETS;RUNTARGETS;TESTS" ${ARGN})
  message(STATUS "Configuring test coverage report for target(s): ${ARG_RUNTARGETS}")

  # Define coverage output files
  set(COVERAGE_DIR "${PROJECT_BINARY_DIR}/coverage")
  set(COVERAGE_RAW_FILE "${COVERAGE_DIR}/raw.info")
  set(COVERAGE_FILTERED_FILE "${COVERAGE_DIR}/lcov.info")
  set(COVERAGE_REPORT_LCOV_DIR "${COVERAGE_DIR}/lcov")
  set(LLVM_COV_SCRIPT "${COVERAGE_DIR}/llvm-cov-tool.sh")

  # Attempt to find gcov or llvm-cov
  set(GCOV_TOOL "")

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    find_program(LLVM_COV NAMES llvm-cov)
    if(LLVM_COV)
      # We need to wrap the call 'llvm-cov gcov' in a script so lcov can use it.
      # The temporary file is needed because file WRITE does not allow to set permissions.
      set(LLVM_COV_TMP "${PROJECT_BINARY_DIR}/llvm-cov-tool.sh")
      file(WRITE ${LLVM_COV_TMP}  "#!/bin/bash\n")
      file(APPEND ${LLVM_COV_TMP} "exec ${LLVM_COV} gcov \"\$@\"\n")
      file(MAKE_DIRECTORY ${COVERAGE_DIR})
      file(COPY ${LLVM_COV_TMP} DESTINATION ${COVERAGE_DIR} FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
      file(REMOVE ${LLVM_COV_TMP})
      set(GCOV_TOOL "${LLVM_COV_SCRIPT}")
    endif()
  else()
    find_program(GCOV NAMES gcov)
    if(GCOV)
      set(GCOV_TOOL "${GCOV}")
    endif()
  endif()

  if(GCOV_TOOL STREQUAL "")
    message(WARNING "Cannot enable coverage targets because no gcov tool was found.")
    return()
  endif()

  # Attempt to find lcov and gcovr
  find_package(LCOV)
  find_package(GCOVR)

  # Pick the appropriate tool
  if(LCOV_FOUND OR GCOVR_FOUND)
    set(TOOL_FOUND TRUE)
  endif()
  if(NOT TOOL_FOUND)
    message(WARNING "Cannot enable coverage targets because neither lcov nor gcovr were found.")
    return()
  endif()

  # Reset all lcov execution counts to zero
  if(LCOV_FOUND)
    set(DEPTARGETS ${DEPTARGETS} COMMAND ${LCOV_EXECUTABLE} -q -z
      -d ${PROJECT_BINARY_DIR}
      --gcov-tool ${GCOV_TOOL}
    COMMAND rm -f ${COVERAGE_RAW_FILE})
  endif()

  # Setup commands for dependent targets to run
  foreach(D ${ARG_DEPTARGETS})
    set(DEPTARGETS ${DEPTARGETS} COMMAND $(MAKE) ${D})
  endforeach()
  add_custom_target(clean-coverage DEPENDS ${ARG_TESTS} ${DEPTARGETS})

  # Add target to clean gcda files.
  add_custom_target(clean-gcda COMMAND ${CMAKE_COMMAND}
    -DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}
    -P ${CMAKE_MODULE_PATH}/CleanCoverage.cmake)
  foreach(T ${ARG_RUNTARGETS})
    add_dependencies(${T} clean-gcda)
  endforeach()

  # Configure LCOV HTML report generation
  if(LCOV_FOUND)
    message(STATUS "  lcov: HTML coverage report")

    # Setup coverage target
    add_custom_command(
      OUTPUT ${COVERAGE_RAW_FILE}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}
      COMMAND ${LCOV_EXECUTABLE} -q -c
        -d ${PROJECT_BINARY_DIR}
        -o ${COVERAGE_RAW_FILE}
        --gcov-tool ${GCOV_TOOL}
        --ignore-errors source
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      DEPENDS clean-coverage
      COMMENT "Collecting coverage data"
      VERBATIM
    )

    # Setup filter from function argument
    list(LENGTH ARG_FILTER FILTER_LENGTH)
    if(${FILTER_LENGTH} GREATER 0)
      set(FILTER COMMAND ${LCOV_EXECUTABLE} -q
        -o ${COVERAGE_FILTERED_FILE}
        -r ${COVERAGE_FILTERED_FILE}
        --gcov-tool ${GCOV_TOOL})
      foreach(F ${ARG_FILTER})
        set(FILTER ${FILTER} "*/${F}/*")
      endforeach()
    endif()

    # Filter specifically excluded sources
    add_custom_command(
      OUTPUT ${COVERAGE_FILTERED_FILE}
      COMMAND ${LCOV_EXECUTABLE} -q
        -e ${COVERAGE_RAW_FILE}
        -o ${COVERAGE_FILTERED_FILE}
        --gcov-tool ${GCOV_TOOL}
        "${PROJECT_SOURCE_DIR}*"
      ${FILTER}
      DEPENDS ${COVERAGE_RAW_FILE}
      COMMENT "Filtering recorded coverage data for project-relevant entries"
      VERBATIM
    )

    # Generate HTML report
    add_custom_command(
      OUTPUT ${COVERAGE_REPORT_LCOV_DIR}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_REPORT_LCOV_DIR}
      COMMAND ${GENHTML_EXECUTABLE}
      ARGS -q --legend --show-details -t "${PROJECT_NAME} test coverage"
           -o ${COVERAGE_REPORT_LCOV_DIR} ${COVERAGE_FILTERED_FILE}
      DEPENDS ${COVERAGE_FILTERED_FILE}
      COMMENT "Generating LCOV HTML coverage report in ${COVERAGE_REPORT_LCOV_DIR}"
      VERBATIM
    )
    add_custom_target(coverage-lcov DEPENDS ${COVERAGE_REPORT_LCOV_DIR})

    # Mark directory to be removed with make clean
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${COVERAGE_DIR})

    # Append dependency to global coverage target
    list(APPEND GLOBAL_DEPENDS coverage-lcov)
  endif()

  # Configure GCOVR console report generation
  if(GCOVR_FOUND)
    message(STATUS "  gcovr: text coverage report")

    # Setup filter from function argument
    set(FILTER "")
    foreach(F ${ARG_FILTER})
      set(FILTER ${FILTER} --exclude=.*${F}/.*)
    endforeach()

    # Setup excluded directories from function argument
    set(EXCLUDED_DIRS "")
    foreach(E ${ARG_EXCLUDE_DIRS})
      set(EXCLUDED_DIRS ${EXCLUDED_DIRS} --exclude-directories=${E})
    endforeach()

    add_custom_target(coverage-gcovr
      COMMAND ${GCOVR_EXECUTABLE}
        ${FILTER}
        ${EXCLUDED_DIRS}
        -r ${PROJECT_SOURCE_DIR}
        --object-directory=${PROJECT_BINARY_DIR}
        --gcov-executable=${GCOV_TOOL}
      DEPENDS clean-coverage
      COMMENT "Generating GCOVR coverage report"
    )

    # Append dependency to global coverage target
    list(APPEND GLOBAL_DEPENDS coverage-gcovr)
  endif()

  # Provide a global coverage target executing both steps if available
  add_custom_target(coverage DEPENDS ${GLOBAL_DEPENDS})
endfunction()
