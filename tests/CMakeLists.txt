include(GoogleTest)
find_package(GTest REQUIRED)

add_executable(result_test small_tests.cpp static_tests.cpp coro.cpp tests.cpp)

target_link_libraries(result_test PUBLIC result gtest::gtest)

gtest_discover_tests(result_test)
