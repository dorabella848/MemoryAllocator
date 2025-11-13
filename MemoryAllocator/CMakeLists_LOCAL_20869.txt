cmake_minimum_required(VERSION 3.28)
set(GOOGLETEST_VERSION 1.15.2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

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

# Test executable
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


include(GoogleTest)
gtest_discover_tests(allocatortest)