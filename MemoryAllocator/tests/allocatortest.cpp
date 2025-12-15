#include <gtest/gtest.h>
#include "Allocator.hpp"
#include <random>

// Move build files out of main directory,
// you can still call cmake using ../(whatever folder has CMakeLists.txt which should be MemoryAllocator)

// valgrind --leak-check=full --track-origins=yes ./build/allocatortest
// --gtest_filter={TestSuite}.{TestName}
// make
// ctest

// Test allocating almost all available memory (e.g., 9 out of 10)
template <typename T>
void TestConnections(Allocator<T>& alloc){
  // Finding first chunk
  Chunk<T>* currentChunk = nullptr;
  if(alloc.getOccHead() == nullptr){
      currentChunk = alloc.getFreeHead();
  }
  else if(alloc.getFreeHead() == nullptr){
      currentChunk = alloc.getOccHead();

  }
  else if(alloc.getOccHead()->startIndex == 0){
      currentChunk = alloc.getOccHead();
  }
  else{
      currentChunk = alloc.getFreeHead();
  }
  // Checking Connections
  size_t totalMem = 0;
  size_t totalFreeMem = 0;
  while(currentChunk != nullptr){
    totalMem += currentChunk->chunkSize;
    if(currentChunk->Free){
      totalFreeMem += currentChunk->chunkSize;
    }
    if(currentChunk->prev != nullptr){
      GTEST_ASSERT_LT(currentChunk->prev->startIndex, currentChunk->startIndex);
      GTEST_ASSERT_EQ(currentChunk->prev->next, currentChunk);
      GTEST_ASSERT_EQ(currentChunk->prev->Free, currentChunk->Free);
    }
    if(currentChunk->AbsPrev != nullptr){
      GTEST_ASSERT_LT(currentChunk->AbsPrev->startIndex, currentChunk->startIndex);
      GTEST_ASSERT_EQ(currentChunk->AbsPrev->AbsNext, currentChunk);
    }
    if(currentChunk->next != nullptr){
      GTEST_ASSERT_GT(currentChunk->next->startIndex, currentChunk->startIndex);
      GTEST_ASSERT_EQ(currentChunk->next->prev, currentChunk);
      GTEST_ASSERT_EQ(currentChunk->next->Free, currentChunk->Free);
    }
    if(currentChunk->AbsNext != nullptr){
      GTEST_ASSERT_EQ(currentChunk->AbsNext->AbsPrev, currentChunk);
      // Make sure no free chunks are together
      if(currentChunk->Free){
        GTEST_ASSERT_FALSE(currentChunk->AbsNext->Free);
      }
      GTEST_ASSERT_EQ(currentChunk->startIndex+currentChunk->chunkSize, currentChunk->AbsNext->startIndex);
    }
    currentChunk = currentChunk->AbsNext;
  }
  GTEST_ASSERT_EQ(totalFreeMem, alloc.getFreeMemory());
  GTEST_ASSERT_EQ(totalMem, alloc.getMemoryTotal());
}

TEST(AllocatorMalloc, MallocValidSize_AlmostFull) {
  Allocator<int> allocator(10);
  GTEST_ASSERT_NE(allocator.allocate(10), nullptr);
  TestConnections(allocator);
}

// Test allocating the last available chunk of memory
TEST(AllocatorMalloc, MallocValidSize_LastChunk) {
  Allocator<int> allocator(10);
  allocator.allocate(9); // Allocate first part
  GTEST_ASSERT_NE(allocator.allocate(1), nullptr);
  TestConnections(allocator);
}

// Test allocating zero bytes
TEST(AllocatorMalloc, MallocZeroBytes) {
  Allocator<int> allocator(10);
  GTEST_ASSERT_EQ(allocator.allocate(0), nullptr);
  TestConnections(allocator);
}

// Test allocating more memory than available
TEST(AllocatorMalloc, MallocTooMuch) {
  Allocator<int> allocator(10);
  GTEST_ASSERT_EQ(allocator.allocate(11), nullptr);
  TestConnections(allocator);
}

TEST(AllocatorMalloc, MallocNegative){
  Allocator<int> allocator(10);
  GTEST_ASSERT_EQ(allocator.allocate(-1), nullptr);
  TestConnections(allocator);
}

TEST(AllocatorMalloc, MallocFullPoolSize1Blocks){
  Allocator<int> allocator(10);
  for(int i = 0; i < 10; i++){
    GTEST_ASSERT_NE(allocator.allocate(1), nullptr);
  }
  TestConnections(allocator);
}

TEST(AllocatorFree, SingleBlockFreeTest){
  Allocator<int> alloc(10);
  int* test1 = alloc.allocate(1);
  alloc.deallocate(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 10*sizeof(int));
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);
  TestConnections(alloc);
}

TEST(AllocatorFree, CallocFreeSingleIndex){
  Allocator<int> alloc(10);
  int* test1 = alloc.calloc(1, 4);
  alloc.deallocate(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 10*sizeof(int));
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);

  GTEST_ASSERT_EQ(alloc.getFreeHead()->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->prev, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsNext, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsPrev, nullptr);
  TestConnections(alloc);
}

TEST(AllocatorFree, CallocFreeMultipleIndex){
  Allocator<int> alloc(10);
  int* test1 = alloc.calloc(10, 1);
  alloc.deallocate(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 10*sizeof(int));
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);

  GTEST_ASSERT_EQ(alloc.getFreeHead()->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->prev, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsNext, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsPrev, nullptr);
  TestConnections(alloc);
}

TEST(AllocatorFree, FragmentedFree){
  Allocator<int> alloc(10);
  int* test1 = alloc.allocate(1);
  int* test2 = alloc.allocate(2);
  int* test3 = alloc.allocate(3);
  alloc.deallocate(test2);
  GTEST_ASSERT_EQ(alloc.getOccHead()->next->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getOccHead()->next->chunkSize, 3*sizeof(int));
  TestConnections(alloc);
}
TEST(AllocatorFree, InformationOverwriteFree){
  Allocator<int> alloc(10);
  int* test1 = alloc.allocate(1);
  *test1 = 5;
  alloc.deallocate(test1);
  int* test2 = alloc.allocate(2);
  *test2 = 10;
  GTEST_ASSERT_EQ(*test2, 10);
  TestConnections(alloc);
}

TEST(AllocatorFree, FreeChunkAhead){
  Allocator<int> alloc(10);
  int* test1 = alloc.allocate(1);
  int* test2 = alloc.allocate(2);
  int* test3 = alloc.allocate(3);
  int* test4 = alloc.allocate(4);
  alloc.deallocate(test3);
  alloc.deallocate(test2);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 4);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 5*sizeof(int));
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, test2);
  TestConnections(alloc);
}

TEST(AllocatorFree, FreeChunkBehind){
  Allocator<int> alloc(10);
  int* test1 = alloc.allocate(1);
  int* test2 = alloc.allocate(2);
  int* test3 = alloc.allocate(3);
  int* test4 = alloc.allocate(4);
  alloc.deallocate(test2);
  alloc.deallocate(test3);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 4);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 5*sizeof(int));
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, test2);
  TestConnections(alloc);
}

TEST(AllocatorFree, NoFreeHead){
  Allocator<int> alloc(10);
  int* test1 = alloc.allocate(10);
  alloc.deallocate(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 0);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 10*sizeof(int));
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);
  TestConnections(alloc);
}

TEST(AllocatorFree, AfterFreeHead_No_Adj_Free){
  Allocator<int> alloc(16);
  int* test1 = alloc.allocate(1);
  int* test2 = alloc.allocate(2);
  int* test3 = alloc.allocate(3);
  int* test4 = alloc.allocate(4);
  int* test5 = alloc.allocate(5);
  alloc.deallocate(test2);
  alloc.deallocate(test4);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, test2);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->next->startLoc, test4);
  GTEST_ASSERT_EQ(alloc.getOccHead()->startLoc, test1);
  TestConnections(alloc);
}

TEST(AllocatorCalloc, AllAssigned){
  Allocator<int> alloc(10);
  int* test1 = alloc.calloc(1, 10);

  //cout << (*test1)[0];
  for (int i = 0; i < 10; i++) {
    test1[i] = i;
  }

  for (int i = 0; i < 10; i++) {
    GTEST_ASSERT_EQ(test1[i], i);
  }
  TestConnections(alloc);
}

TEST(AllocatorRealloc, NullInput){
  Allocator<int> alloc(10);
  int* chunk = alloc.reallocate(nullptr, 203);
  GTEST_ASSERT_EQ(chunk, nullptr);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, reallocToZero){
  Allocator<int> alloc(10);
  int* chunk = alloc.allocate(10);
  chunk = alloc.reallocate(alloc.getOccHead()->startLoc, 0);
  GTEST_ASSERT_EQ(chunk, alloc.getOccHead()->startLoc);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, GreaterThanAvailable){
  Allocator<int> alloc(6);
  int* test1 = alloc.allocate(1);
  int* test2 = alloc.allocate(2);
  int* test3 = alloc.allocate(3);
  test2 = alloc.reallocate(test2, 45);
  GTEST_ASSERT_EQ(test2, alloc.getOccHead()->next->startLoc);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, ReUseSamePos){
  Allocator<int> alloc(10);
  int* test1 = alloc.allocate(1);
  int* test2 = alloc.allocate(2);
  int* test3 = alloc.allocate(3);
  int* test4 = alloc.allocate(4);

  alloc.deallocate(test3);
  test2 = alloc.reallocate(test2, 5);
  GTEST_ASSERT_EQ(test2, (alloc.getOccHead()->next->startLoc));
  TestConnections(alloc);
}

TEST(AllocatorRealloc, InformationPreserved){
  Allocator<int> alloc(10);
  int* test1 = alloc.allocate(1);
  *test1 = 15;
  int* test2 = alloc.reallocate(test1, 2);
  GTEST_ASSERT_EQ(*test2, 15);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, CallocRealloc){
  Allocator<int> alloc(30);
  int* test1 = alloc.calloc(1, 5);
  for(int i =0; i < 5; i++){
    test1[i] = i*5;
  }
  int* test2 = alloc.reallocate(test1, 10);
  for(int i =0; i < 5; i++){
    GTEST_ASSERT_EQ(test2[i], i*5);
  }
  TestConnections(alloc);
}

TEST(AllocatorSTL, STLmalloc){
  std::vector<int, Allocator<int>> vec;
  for(int i = 0; i < 100; i++){
    vec.push_back(i);
  }
  for(int i = 0; i < 100; i++){
    GTEST_ASSERT_EQ(vec.at(i), i);
  }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
