include(GoogleTest)
find_package(GTest REQUIRED)

add_executable(value_or_error_test static_tests.cpp small_tests.cpp tests.cpp coro.cpp)

target_link_libraries(value_or_error_test PUBLIC value_or_error gtest::gtest)

gtest_discover_tests(value_or_error_test)
