#include <gtest/gtest.h>
#include "Allocator.cpp"

TEST(allocator, malloc)
{
    MemoryAllocator allocator123 = MemoryAllocator(8192);
    GTEST_ASSERT_NE(allocator123.malloc(10), nullptr);
}
TEST(allocator, free)
{
    MemoryAllocator allocator123 = MemoryAllocator(8192);
    int* test = (int*)allocator123.malloc(10);
    int* test1 = (int*)allocator123.malloc(10);
    allocator123.free(test);
    GTEST_ASSERT_TRUE(allocator123.getFreeHead()->startIndex == 0);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}