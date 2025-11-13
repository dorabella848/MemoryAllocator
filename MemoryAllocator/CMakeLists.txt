cmake_minimum_required(VERSION 3.28)
set(GOOGLETEST_VERSION 1.15.2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0 -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -O0 -g")
include(FetchContent)
set(BENCHMARK_ENABLE_REGEX OFF)
set(BENCHMARK_ENABLE_TESTING OFF)


project(MemoryAllocator LANGUAGES CXX)

enable_testing()
add_library(MemoryAllocator STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Allocator.cpp
)
target_include_directories(MemoryAllocator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

# Benchmark executable
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG main
)
FetchContent_MakeAvailable(benchmark)

add_executable(allocatorbenchmark ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/allocatorBenchmark.cpp)
target_include_directories(allocatorbenchmark PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(allocatorbenchmark PRIVATE
    benchmark
    benchmark_main
    MemoryAllocator
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(allocatortest ${CMAKE_CURRENT_SOURCE_DIR}/tests/allocatortest.cpp)
target_include_directories(allocatortest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(allocatortest PRIVATE
    gtest
    gtest_main
    MemoryAllocator
)

add_executable(Runner ${CMAKE_CURRENT_SOURCE_DIR}/examples/AllocatorRunner.cpp)
target_include_directories(Runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Runner PRIVATE MemoryAllocator)

if(CMAKE_CROSSCOMPILING)
  message(STATUS "Skipping gtest_discover_tests due to cross-compilation")
else()
  include(GoogleTest)
  gtest_discover_tests(allocatortest)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT WIN32)
  set(GTEST_DISCOVER_TESTS OFF)
endif()

if(NOT GTEST_DISCOVER_TESTS STREQUAL OFF)
  include(GoogleTest)
  gtest_discover_tests(allocatortest)
endif()



include(GoogleTest)
gtest_discover_tests(allocatortest)

#For running gcov and lcov: 

#./allocatortest 

#There is a mismatched line error (58-62), cannot figure out why(?), lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch
#genhtml coverage.info --output-directory coverage-report --ignore-errors mismatch can be run to work around this

#lcov --capture --directory . --output-file coverage.info
#genhtml coverage.info --output-directory ../coverage_html

#\\wsl$\Ubuntu\home\ellie\MemoryAllocator\coverage_html in File Explorer to find visual of code coverage 
