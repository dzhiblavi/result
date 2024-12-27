add_library(value_or_error INTERFACE)

target_include_directories(
  value_or_error INTERFACE $<BUILD_INTERFACE:${VOE_SOURCE_DIR}/include/>)
target_include_directories(
  value_or_error SYSTEM
  INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/>)
