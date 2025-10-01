#include <gtest/gtest.h>
#include "Allocator.cpp"
#include <random>

// valgrind --leak-check=full --track-origins=yes ./allocatortest 
// --gtest_filter={TestSuite}.{TestName}

// Test allocating almost all available memory (e.g., 9 out of 10)
TEST(AllocatorMalloc, MallocValidSize_AlmostFull) {
  Allocator allocator(10);
  GTEST_EXPECT_TRUE(allocator.malloc(10) != nullptr);

}

// Test allocating the last available chunk of memory
TEST(AllocatorMalloc, MallocValidSize_LastChunk) {
  Allocator allocator(10);
  allocator.malloc(9); // Allocate first part
  GTEST_EXPECT_TRUE(allocator.malloc(1) != nullptr);
}

// Test allocating zero bytes
TEST(AllocatorMalloc, MallocZeroBytes) {
  Allocator allocator(10);
  GTEST_EXPECT_TRUE(allocator.malloc(0) == nullptr);
}

// Test allocating more memory than available
TEST(AllocatorMalloc, MallocTooMuch) {
  Allocator allocator(10);
  GTEST_EXPECT_TRUE(allocator.malloc(11) == nullptr);
}

TEST(AllocatorMalloc, MallocNegative){
  Allocator allocator(10);
  GTEST_EXPECT_TRUE(allocator.malloc(-1) == nullptr);
}

TEST(AllocatorMalloc, MallocFullPoolSize1Blocks){
  Allocator allocator(10);
  for(int i = 0; i < 10; i++){
    GTEST_ASSERT_TRUE(allocator.malloc(1) != nullptr);
  }
}

TEST(AllocatorFree, SingleBlockFreeTest){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  alloc.free(*test1);
  GTEST_ASSERT_TRUE(alloc.getFreeHead()->chunkSize == 8192);
  GTEST_ASSERT_TRUE(alloc.getOccHead() == nullptr);
    
}
TEST(AllocatorFree, FragmentedFree){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(22);
  int** test3 = (int**)alloc.malloc(33);
  alloc.free(*test2);
  GTEST_ASSERT_TRUE(alloc.getOccHead()->next->next == nullptr);
  GTEST_ASSERT_TRUE(alloc.getOccHead()->next->chunkSize == 33);

}
TEST(AllocatorFree, InformationOverwriteFree){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  **test1 = 5;
  alloc.free(*test1);
  int** test2 = (int**)alloc.malloc(22);
  **test2 = 10;
  GTEST_ASSERT_TRUE(**test1 == 10);

}
TEST(AllocatorDefragment, Defragment1){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(22);
  int** test3 = (int**)alloc.malloc(33);

  alloc.free(*test2);
  alloc.defragment();

  GTEST_ASSERT_TRUE(11 == alloc.getOccHead()->next->startIndex);  
  GTEST_ASSERT_TRUE(alloc.getOccHead()->next->startLoc == *test3);
  GTEST_ASSERT_TRUE(alloc.getOccHead()->startLoc == *test1);
  // 44 test1 chunkSize + test3 chunkSize
  GTEST_ASSERT_TRUE(alloc.getFreeHead()->startIndex == 44);
  GTEST_ASSERT_TRUE(alloc.getFreeHead()->startLoc == alloc.getMemAddress(alloc.getFreeHead()->startIndex));
}

TEST(AllocatorPerformance, Allocate1b){
  Allocator alloc(1000000000);
  GTEST_EXPECT_TRUE(alloc.malloc(1000000000) != nullptr);
}
TEST(AllocatorPerformance, Allocate100kblocks){
  Allocator alloc(100000);
  for(int i = 1; i < 100002; i++){
    alloc.malloc(1);
    if(i == 100000){
      GTEST_EXPECT_TRUE(alloc.malloc(1) == nullptr);
    }
  }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}