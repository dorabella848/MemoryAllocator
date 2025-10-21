#include <gtest/gtest.h>
#include "Allocator.cpp"
#include <random>

// valgrind --leak-check=full --track-origins=yes ./allocatortest 
// --gtest_filter={TestSuite}.{TestName}
// -lgtest -lgtest_main -pthread

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
TEST(AllocatorFree, CallocFreeSingleIndex){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.calloc(1, sizeof(int*));
  alloc.free(*test1);
  GTEST_ASSERT_TRUE(alloc.getFreeHead()->chunkSize == 8192);
  GTEST_ASSERT_TRUE(alloc.getOccHead() == nullptr);
}
TEST(AllocatorFree, CallocFreeMultipleIndex){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.calloc(10, sizeof(int*));
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
  GTEST_ASSERT_TRUE(**test2 == 10);

}
TEST(AllocatorDefragment, Malloc){
  //double free error-same block deallocated 2+ times 
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

TEST(AllocatorDefragment, Calloc){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(33);
  int** test3 = (int**)alloc.calloc(10, sizeof(int));

  alloc.free(*test2);
  alloc.defragment();

  GTEST_ASSERT_TRUE(11 == alloc.getOccHead()->next->startIndex);  
  GTEST_ASSERT_TRUE(alloc.getOccHead()->next->startLoc == *test3);
  GTEST_ASSERT_TRUE(alloc.getOccHead()->startLoc == *test1);
  GTEST_ASSERT_TRUE(alloc.getFreeHead()->startIndex == 51);
  GTEST_ASSERT_TRUE(alloc.getFreeHead()->startLoc == alloc.getMemAddress(alloc.getFreeHead()->startIndex));
}

TEST(AllocatorCalloc, AllAssigned){
  Allocator alloc(8096);
  int** test1 = (int**)alloc.calloc(10, sizeof(int));

  //cout << (*test1)[0];
  for (int i = 0; i < 10; i++) {
    (*test1)[i] = i;
  }

  for (int i = 0; i < 10; i++) {
    GTEST_ASSERT_TRUE((*test1)[i] == i);
  }
}


// Test Performance

// single 1 billion byte alloc
TEST(PerformAlloc, largeAllocation1b){
  Allocator alloc(1000000000);
  GTEST_EXPECT_TRUE(alloc.malloc(1000000000) != nullptr);
}
// 100k blocks
TEST(PerformAlloc, manyAlloc10M){
  Allocator alloc(10000000);
  for(int i = 0; i < 10000000; i++){
    GTEST_ASSERT_TRUE(alloc.malloc(1) != nullptr);
  }
  GTEST_ASSERT_TRUE(alloc.getFreeHead() == nullptr);
}
TEST(AllocatorRealloc, NullInput){
  Allocator alloc(8096);
  int* chunk = (int*)alloc.realloc(nullptr, 203);
  int **test = &chunk;
  GTEST_ASSERT_TRUE(*test == nullptr);
}

TEST(AllocatorRealloc, reallocToZero){
  Allocator alloc(8096);
  int* chunk = (int*)alloc.realloc(alloc.getOccHead(), 0);
  int **test2 = &chunk;
  GTEST_ASSERT_TRUE(*test2 == nullptr);
}

TEST(AllocatorRealloc, GreaterThanAvailable){
  Allocator alloc(8096);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(33);
  int** test3 = (int**)alloc.malloc(55);
  test2 = (int**)alloc.realloc(*test2, 45);
  GTEST_ASSERT_TRUE(*test2 == alloc.getOccHead()->next->next->startLoc);
  // add more asserts
}

TEST(AllocatorRealloc, ReUseSamePos){
  Allocator alloc(8096);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(33);
  int** test3 = (int**)alloc.malloc(35);
  int** test4 = (int**)alloc.malloc(55);

  alloc.free(*test3);
  test2 = (int**)alloc.realloc(*test2, 58);
  GTEST_ASSERT_TRUE((alloc.getOccHead()->next->startLoc) == *test2);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

