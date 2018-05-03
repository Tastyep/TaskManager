function(enable_clangtidy)
  cmake_parse_arguments(ARG "" "" "SOURCES" ${ARGN})

  message(STATUS "Configuring clang-tidy static analysis")

  # Attempt to find clang-tidy
  find_package(ClangTidy)

  if(ClangTidy_FOUND)
    # Add clang-tidy static analysis target
    add_custom_target(clang-tidy COMMAND ${RunClangTidy_EXECUTABLE}
      -header-filter=.*
      ${ARG_SOURCES}
    )
  endif()
endfunction()
