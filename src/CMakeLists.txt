add_library(result INTERFACE)

target_include_directories(
  result INTERFACE $<BUILD_INTERFACE:${VOE_SOURCE_DIR}/include/>)
target_include_directories(
  result SYSTEM
  INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/>)
