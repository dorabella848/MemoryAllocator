cmake_minimum_required(VERSION 3.28)
set(GOOGLETEST_VERSION 1.15.2)
project(CodeDynamic)

enable_testing()
find_library(GTest gtest)
add_subdirectory(googletest)

add_executable(allocatortest allocatortest.cpp)

target_include_directories(allocatortest PRIVATE googletest/include)
target_link_libraries(allocatortest PRIVATE gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(allocatortest)