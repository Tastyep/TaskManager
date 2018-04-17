include(CheckCompilerFlags)

# Check and define C compiler flags
check_cxx_compiler_flags(
  FLAGS
  "-std=c++1y"
  "-Wall" "-pedantic" "-Wextra"
  "-Wcast-align"
  "-Wcast-qual" "-Wconversion"
  "-Wdisabled-optimization"
  "-Wdocumentation"
  "-Wformat=2"
  "-Wformat-nonliteral" "-Wformat-security"
  "-Wimplicit" "-Wimport" "-Winit-self" "-Winline"
  "-Wmissing-field-initializers" "-Wmissing-format-attribute"
  "-Wmissing-include-dirs" "-Wmissing-noreturn"
  "-Wpacked" "-Wpointer-arith"
  "-Wredundant-decls"
  "-Wstack-protector"
  "-Wstrict-aliasing=2" "-Wswitch-default"
  "-Wunreachable-code" "-Wunused"
  "-Wunused-parameter"
  "-Wvariadic-macros"
  "-Wwrite-strings"
  "-Wno-builtin-macro-redefined"
  "-Wno-unknown-pragmas"
  "-Wno-suggest-attribute=noreturn"
  "-Wno-parentheses"
  # Enable colorized output
  "-fdiagnostics-color=auto"

  FLAGS_RELEASE
  "-march=native"
  "-Wno-inline"
  "-Wno-unused-parameter"
  "-Wno-unused-variable"
  "-Wno-unused-but-set-variable"
  "-Wno-maybe-uninitialized"
  "-Os"
)
