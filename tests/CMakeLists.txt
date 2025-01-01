include(GoogleTest)
find_package(GTest REQUIRED)

add_executable(
  result_test
  ./small_tests.cpp
  ./static_tests.cpp
  ./tests.cpp
  ./test_coro.cpp
  ./combine/test_and_then.cpp
  ./combine/test_map.cpp
  ./combine/test_lift.cpp
  ./combine/test_map_err.cpp
  ./combine/test_or_else.cpp)

target_link_libraries(result_test PUBLIC result gtest::gtest)

gtest_discover_tests(result_test)
