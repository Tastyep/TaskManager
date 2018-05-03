include(CMakeDetermineCCompiler)

# Check whether the CXX compiler supports C++11.
function(check_compiler)
  set(GCC_VERSION "5.0")
  set(CLANG_VERSION "3.4")

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${CLANG_VERSION})
      message(FATAL_ERROR "\nIn order to use C++14 features, ${PROJECT_NAME} cannot be built using a version of Clang less than ${CLANG_VERSION}")
    endif()
  elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${GCC_VERSION})
      message(FATAL_ERROR "\nIn order to use C++14 features, ${PROJECT_NAME} cannot be built using a version of GCC less than ${GCC_VERSION}")
    endif()
  endif()
endfunction()
