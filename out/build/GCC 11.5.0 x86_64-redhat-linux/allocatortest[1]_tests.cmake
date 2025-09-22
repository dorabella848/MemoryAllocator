add_test([=[allocator.malloc]=]  [==[/home/internaccount/Downloads/CodeDynamic/out/build/GCC 11.5.0 x86_64-redhat-linux/allocatortest]==] [==[--gtest_filter=allocator.malloc]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[allocator.malloc]=]  PROPERTIES WORKING_DIRECTORY [==[/home/internaccount/Downloads/CodeDynamic/out/build/GCC 11.5.0 x86_64-redhat-linux]==] SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  allocatortest_TESTS allocator.malloc)
