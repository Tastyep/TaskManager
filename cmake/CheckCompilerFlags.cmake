include(CheckCXXCompilerFlag)

# Check whether the CXX compiler supports the given flags.
# Usage: check_cxx_compiler_flags(FLAGS;FLAGS_RELEASE;FLAGS_DEBUG)
macro(check_cxx_compiler_flags)
  cmake_parse_arguments(ARG "" "" "FLAGS;FLAGS_RELEASE;FLAGS_DEBUG" ${ARGN})

  # General flags
  # NOTE: We use a separate variable to hold the checked CXX flags to workaround a bug
  # when using -stdlib=libc++ that makes all of the subsequent checks fail.
  set(COMPILER_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "")
  foreach(F ${ARG_FLAGS})
    # Make a good unique variable name for the check
    string(REGEX REPLACE "[-+=]" "_" F_CHECK_NAME ${F})
    set(F_CHECK_CXX_NAME CHECK_CXX_FLAG${F_CHECK_NAME})
    # Do the check and add the definition if it passes
    check_cxx_compiler_flag(${F} ${F_CHECK_CXX_NAME} HAS_FLAG)
    if(${F_CHECK_CXX_NAME})
      set(COMPILER_CXX_FLAGS "${COMPILER_CXX_FLAGS} ${F}")
    endif(${F_CHECK_CXX_NAME})
  endforeach(F ${ARG_FLAGS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_CXX_FLAGS}")

  # Release flags
  foreach(F ${ARG_FLAGS_RELEASE})
    # Make a good unique variable name for the check
    string(REGEX REPLACE "[-+=]" "_" F_CHECK_NAME ${F})
    set(F_CHECK_CXX_NAME CHECK_CXX_FLAG${F_CHECK_NAME})
    # Do the check and add the definition if it passes
    check_cxx_compiler_flag(${F} ${F_CHECK_CXX_NAME})
    if(${F_CHECK_CXX_NAME})
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${F}")
      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${F}")
    endif(${F_CHECK_CXX_NAME})
  endforeach(F ${ARG_FLAGS})

  # Debug flags
  foreach(F ${ARG_FLAGS_DEBUG})
    # Make a good unique variable name for the check
    string(REGEX REPLACE "[-+=]" "_" F_CHECK_NAME ${F})
    set(F_CHECK_CXX_NAME CHECK_CXX_FLAG${F_CHECK_NAME})
    # Do the check and add the definition if it passes
    check_cxx_compiler_flag(${F} ${F_CHECK_CXX_NAME})
    if(${F_CHECK_CXX_NAME})
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${F}")
      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${F}")
    endif(${F_CHECK_CXX_NAME})
  endforeach(F ${ARG_FLAGS})
endmacro()
