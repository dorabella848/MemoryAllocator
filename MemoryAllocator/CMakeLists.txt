cmake_minimum_required(VERSION 3.28)
set(GOOGLETEST_VERSION 1.15.2)
project(CodeDynamic)

enable_testing()
add_library(MemoryAllocator
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Allocator.cpp
)
target_include_directories(MemoryAllocator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(googletest)

# Test executable
add_executable(allocatortest ${CMAKE_CURRENT_SOURCE_DIR}/tests/allocatortest.cpp)
target_include_directories(allocatortest PRIVATE googletest/include)
target_link_libraries(allocatortest PRIVATE
    gtest
    gtest_main
    MemoryAllocator
)

add_executable(Runner ${CMAKE_CURRENT_SOURCE_DIR}/AllocatorRunner.cpp)
target_link_libraries(Runner PRIVATE MemoryAllocator)


include(GoogleTest)
gtest_discover_tests(allocatortest)