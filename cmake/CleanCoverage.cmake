# Enable CMP0009 new behaviour
cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)

# Remove old gcda files
file(GLOB_RECURSE GCDA_FILES "${PROJECT_BINARY_DIR}/*.gcda")
if(NOT GCDA_FILES STREQUAL "")
  file(REMOVE ${GCDA_FILES})
endif()
