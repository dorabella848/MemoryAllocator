# Build command: cmake --preset Debug -DENABLE_TESTS=ON
# Build command: cmake --build --preset Debug 

cmake_minimum_required(VERSION 3.28)
set(GOOGLETEST_VERSION 1.15.2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

option(ENABLE_TESTS "Build unit tests" OFF)
option(ENABLE_BENCHMARKS "Build google benchmarks" OFF)
option(ENABLE_GLCOV "Enable GCOV and LCOV" OFF)
if(ENABLE_GLCOV)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0 -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -O0 -g")
endif()

project(memoryallocator LANGUAGES CXX)
# Search for google tests on the computer, if it doesn't exist then check if the build config
# allows for git fetch to auto retrieve the files (this is for conveniency and can't be used for .deb
# packaging due to install rules)
# Users installing via a .deb package will have to have gtests already installed to build unit tests
if(ENABLE_TESTS AND NOT CMAKE_CROSSCOMPILING)
    enable_testing()
    include(GoogleTest) # for gtest_discover_tests
    find_package(GTest QUIET)

    if(NOT GTest_FOUND)
        include(FetchContent)
        # Get gtest lib
        FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(googletest)
        set_target_properties(gtest PROPERTIES CXX_CLANG_TIDY "")
        set_target_properties(gtest_main PROPERTIES CXX_CLANG_TIDY "")
        set_target_properties(gmock PROPERTIES CXX_CLANG_TIDY "")
        set_target_properties(gmock_main PROPERTIES CXX_CLANG_TIDY "")
    endif()
    add_executable(basicTest ${CMAKE_CURRENT_SOURCE_DIR}/tests/basic.cpp)
    target_include_directories(basicTest PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(basicTest PRIVATE
        gtest
        gtest_main
        memoryallocator
    )
    gtest_discover_tests(basicTest)

    add_executable(stlTest ${CMAKE_CURRENT_SOURCE_DIR}/tests/stl.cpp)
    target_include_directories(stlTest PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(stlTest PRIVATE
        gtest
        gtest_main
        memoryallocator
    )
    gtest_discover_tests(stlTest)
endif()

# Search for google benchmarks on the computer, if it doesn't exist then check if the build config
# allows for git fetch to auto retrieve the files (this is for conveniency and can't be used for .deb
# packaging due to install rules)
if(ENABLE_BENCHMARKS)

    find_package(benchmark QUIET)
    if(NOT benchmark_FOUND)
        # Get GBenchmark lib
        set(BENCHMARK_ENABLE_REGEX OFF)
        set(BENCHMARK_ENABLE_TESTING OFF)
        FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main
        )
        FetchContent_MakeAvailable(benchmark)
        set_target_properties(benchmark PROPERTIES CXX_CLANG_TIDY "")
    endif()

    add_executable(allocBenchmark ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/benchmark.cpp)
    target_include_directories(allocBenchmark PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(allocBenchmark PRIVATE
        benchmark
        benchmark_main
        memoryallocator
    )
endif()

add_library(memoryallocator STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/allocator.cpp
)
target_include_directories(memoryallocator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

add_executable(Runner ${CMAKE_CURRENT_SOURCE_DIR}/examples/runner.cpp)
target_include_directories(Runner PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(Runner PRIVATE 
    memoryallocator
)

install(TARGETS memoryallocator
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(
  DIRECTORY include/
  DESTINATION include
)

install(TARGETS Runner
        RUNTIME DESTINATION bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "memoryallocator")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Intern Dev tim.vasenkov@gmail.com")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6")
set(CPACK_DEBIAN_PACKAGE_RELEASE 1)

include(CPack)


#For running gcov and lcov: 

#There is a mismatched line error (58-62), cannot figure out why(?), lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch
#genhtml coverage.info --output-directory coverage-report --ignore-errors mismatch can be run to work around this

#lcov --capture --directory . --output-file coverage.info
#genhtml coverage.info --output-directory ../coverage_html
