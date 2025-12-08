cmake_minimum_required(VERSION 3.28)
set(GOOGLETEST_VERSION 1.15.2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

option(EnableTests "Build unit tests" OFF)
option(EnableBenchmarks "Build google benchmarks" OFF)
option(EnGLCOV "Enable GCOV and LCOV" OFF)
if(EnGLCOV)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0 -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -O0 -g")
endif()

project(memoryallocator LANGUAGES CXX)
# Search for google tests on the computer, if it doesn't exist then check if the build config
# allows for git fetch to auto retrieve the files (this is for conveniency and can't be used for .deb
# packaging due to install rules)
# Users installing via a .deb package will have to have gtests already installed to build unit tests

if(GTest_FOUND)
    add_executable(allocatortest ${CMAKE_CURRENT_SOURCE_DIR}/tests/allocatortest.cpp)
    target_include_directories(allocatortest PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(allocatortest PRIVATE
        gtest
        gtest_main
        memoryallocator
    )
    gtest_discover_tests(allocatortest)

elseif(EnableTests)
    enable_testing()
    include(FetchContent)
    # Get gtest lib
    FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest) # for gtest_discover_tests

    add_executable(allocatortest ${CMAKE_CURRENT_SOURCE_DIR}/tests/allocatortest.cpp)
    target_include_directories(allocatortest PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(allocatortest PRIVATE
        gtest
        gtest_main
        memoryallocator
    )
    gtest_discover_tests(allocatortest)

endif()

# Search for google benchmarks on the computer, if it doesn't exist then check if the build config
# allows for git fetch to auto retrieve the files (this is for conveniency and can't be used for .deb
# packaging due to install rules)
if(benchmark_FOUND)
    add_executable(allocatorbenchmark ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/allocatorBenchmark.cpp)
    target_include_directories(allocatorbenchmark PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(allocatorbenchmark PRIVATE
        benchmark
        benchmark_main
        memoryallocator
    )

elseif(EnableBenchmarks)
    # Get GBenchmark lib
    set(BENCHMARK_ENABLE_REGEX OFF)
    set(BENCHMARK_ENABLE_TESTING OFF)
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
        memoryallocator
    )
endif()

add_library(memoryallocator STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Allocator.cpp
)
target_include_directories(memoryallocator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

add_executable(Runner ${CMAKE_CURRENT_SOURCE_DIR}/examples/AllocatorRunner.cpp)
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

if(benchmark_FOUND)
install(TARGETS allocatorbenchmark
        RUNTIME DESTINATION bin)
endif()
if(GTest_FOUND)
install(TARGETS allocatortest
        RUNTIME DESTINATION bin)
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "memoryallocator")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Intern Dev tim.vasenkov@gmail.com")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "libgtest-dev")
set(CPACK_DEBIAN_PACKAGE_RELEASE 1)

include(CPack)
#For running gcov and lcov: 

#./allocatortest 

#There is a mismatched line error (58-62), cannot figure out why(?), lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch
#genhtml coverage.info --output-directory coverage-report --ignore-errors mismatch can be run to work around this

#lcov --capture --directory . --output-file coverage.info
#genhtml coverage.info --output-directory ../coverage_html

#\\wsl$\Ubuntu\home\ellie\memoryallocator\coverage_html in File Explorer to find visual of code coverage 
