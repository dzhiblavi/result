option(VOE_BUILD_TESTS "Build tests" ON)

if(VOE_BUILD_TESTS AND (VOE_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
  enable_testing()
  add_subdirectory(tests)
endif()
