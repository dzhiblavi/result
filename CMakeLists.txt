cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(
  result
  VERSION 0.0.1
  LANGUAGES CXX)

set(VOE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Global settings
include(./cmake/Ccache.cmake)
include(./cmake/CheckCompiler.cmake)
include(./cmake/Development.cmake)
include(./cmake/Sanitizers.cmake)
include(./cmake/CompileOptions.cmake)
include(./cmake/Werror.cmake)

add_subdirectory(src)

include(./cmake/Testing.cmake)
