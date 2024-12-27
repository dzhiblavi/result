include(GoogleTest)
find_package(GTest REQUIRED)

add_executable(result_test static_tests.cpp small_tests.cpp tests.cpp coro.cpp)

target_link_libraries(result_test PUBLIC result gtest::gtest)

gtest_discover_tests(result_test)
