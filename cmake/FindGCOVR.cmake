# Find-module for GCOVR
# Defines:
#
# GCOVR_EXECUTABLE - the gcovr script
#
# Uses:
#
# GCOVR_ROOT - root to search for the script

include(FindPackageHandleStandardArgs)

find_program(GCOVR_EXECUTABLE gcovr
             HINTS ${GCOVR_ROOT}
                   "${GCOVR_ROOT}/bin"
                   "${GCOVR_ROOT}/share/python"
                   "/usr/share/python"
                   "/usr/local/share/python"
                   "${CMAKE_INSTALL_PREFIX}/share/python")

find_package_handle_standard_args(GCOVR DEFAULT_MSG GCOVR_EXECUTABLE)

# only visible in advanced view
mark_as_advanced(GCOVR_EXECUTABLE)
