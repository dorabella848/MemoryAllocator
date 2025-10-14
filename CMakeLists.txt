set(GOOGLETEST_VERSION 1.15.2)
project(CodeDynamic)

enable_testing()

add_subdirectory(googletest)

include_directories(googletest/include)

add_executable(allocatortest allocatortest.cpp)

target_link_libraries(allocatortest PRIVATE gtest)

include(GoogleTest)
gtest_discover_tests(allocatortest)