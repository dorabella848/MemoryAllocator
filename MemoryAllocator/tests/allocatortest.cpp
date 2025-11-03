#include <gtest/gtest.h>
#include "Allocator.hpp"
#include "Chunk.hpp"
#include <random>

// Move build files out of main directory,
// you can still call cmake using ../(whatever folder has CMakeLists.txt which should be MemoryAllocator)

// valgrind --leak-check=full --track-origins=yes ./build/allocatortest
// --gtest_filter={TestSuite}.{TestName}
// make
// ctest

// Test allocating almost all available memory (e.g., 9 out of 10)
void TestConnections(Allocator& alloc){
  // Finding first chunk
  Chunk* currentChunk = nullptr;
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
  while(currentChunk != nullptr){
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
}

TEST(AllocatorMalloc, MallocValidSize_AlmostFull) {
  Allocator allocator(10);
  GTEST_ASSERT_NE(allocator.malloc(10), nullptr);
  TestConnections(allocator);
}

// Test allocating the last available chunk of memory
TEST(AllocatorMalloc, MallocValidSize_LastChunk) {
  Allocator allocator(10);
  allocator.malloc(9); // Allocate first part
  GTEST_ASSERT_NE(allocator.malloc(1), nullptr);
  TestConnections(allocator);
}

// Test allocating zero bytes
TEST(AllocatorMalloc, MallocZeroBytes) {
  Allocator allocator(10);
  GTEST_ASSERT_EQ(allocator.malloc(0), nullptr);
  TestConnections(allocator);
}

// Test allocating more memory than available
TEST(AllocatorMalloc, MallocTooMuch) {
  Allocator allocator(10);
  GTEST_ASSERT_EQ(allocator.malloc(11), nullptr);
  TestConnections(allocator);
}

TEST(AllocatorMalloc, MallocNegative){
  Allocator allocator(10);
  GTEST_ASSERT_EQ(allocator.malloc(-1), nullptr);
  TestConnections(allocator);
}

TEST(AllocatorMalloc, MallocFullPoolSize1Blocks){
  Allocator allocator(10);
  for(int i = 0; i < 10; i++){
    GTEST_ASSERT_NE(allocator.malloc(1), nullptr);
  }
  TestConnections(allocator);
}

TEST(AllocatorFree, SingleBlockFreeTest){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  alloc.free(*test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);
  TestConnections(alloc);
}

TEST(AllocatorFree, CallocFreeSingleIndex){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.calloc(1, sizeof(int*));
  alloc.free(*test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);

  GTEST_ASSERT_EQ(alloc.getFreeHead()->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->prev, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsNext, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsPrev, nullptr);
  TestConnections(alloc);
}

TEST(AllocatorFree, CallocFreeMultipleIndex){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.calloc(10, sizeof(int*));
  alloc.free(*test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);

  GTEST_ASSERT_EQ(alloc.getFreeHead()->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->prev, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsNext, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsPrev, nullptr);
  TestConnections(alloc);
}

TEST(AllocatorFree, FragmentedFree){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(22);
  int** test3 = (int**)alloc.malloc(33);
  alloc.free(*test2);
  GTEST_ASSERT_EQ(alloc.getOccHead()->next->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getOccHead()->next->chunkSize, 33);
  TestConnections(alloc);
}
TEST(AllocatorFree, InformationOverwriteFree){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  **test1 = 5;
  alloc.free(*test1);
  int** test2 = (int**)alloc.malloc(22);
  **test2 = 10;
  GTEST_ASSERT_EQ(**test2, 10);
  TestConnections(alloc);
}

TEST(AllocatorFree, FreeChunkAhead){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(22);
  int** test3 = (int**)alloc.malloc(32);
  int** test4 = (int**)alloc.malloc(42);
  alloc.free(*test3);
  alloc.free(*test2);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 11);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 54);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, *test2);
  TestConnections(alloc);
}

TEST(AllocatorFree, FreeChunkBehind){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(22);
  int** test3 = (int**)alloc.malloc(32);
  int** test4 = (int**)alloc.malloc(42);
  alloc.free(*test2);
  alloc.free(*test3);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 11);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 54);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, *test2);
  TestConnections(alloc);
}

TEST(AllocatorFree, NoFreeHead){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(8192);
  alloc.free(*test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 0);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);
  TestConnections(alloc);
}

TEST(AllocatorFree, AfterFreeHead_No_Adj_Free){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(5);
  int** test2 = (int**)alloc.malloc(11);
  int** test3 = (int**)alloc.malloc(16);
  int** test4 = (int**)alloc.malloc(27);
  int** test5 = (int**)alloc.malloc(43);
  alloc.free(*test2);
  alloc.free(*test4);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, *test2);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->next->startLoc, *test4);
  GTEST_ASSERT_EQ(alloc.getOccHead()->startLoc, *test1);
  TestConnections(alloc);
}

TEST(AllocatorDefragment, Malloc){
  //double free error-same block deallocated 2+ times 
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(22);
  int** test3 = (int**)alloc.malloc(33);

  alloc.free(*test2);
  alloc.defragment();

  GTEST_ASSERT_EQ(alloc.getOccHead()->next->startIndex, 11);  
  GTEST_ASSERT_EQ(alloc.getOccHead()->next->startLoc, *test3);
  GTEST_ASSERT_EQ(alloc.getOccHead()->startLoc, *test1);
  // 44 test1 chunkSize + test3 chunkSize
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 44);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, alloc.getMemAddress(alloc.getFreeHead()->startIndex));
  TestConnections(alloc);
}

TEST(AllocatorDefragment, Calloc){
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(33);
  int** test3 = (int**)alloc.calloc(10, sizeof(int));

  alloc.free(*test2);
  alloc.defragment();

  GTEST_ASSERT_EQ(alloc.getOccHead()->next->startIndex, 11);  
  GTEST_ASSERT_EQ(alloc.getOccHead()->next->startLoc, *test3);
  GTEST_ASSERT_EQ(alloc.getOccHead()->startLoc, *test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 51);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, alloc.getMemAddress(alloc.getFreeHead()->startIndex));
  TestConnections(alloc);
}

TEST(AllocatorCalloc, AllAssigned){
  Allocator alloc(8096);
  int** test1 = (int**)alloc.calloc(10, sizeof(int));

  //cout << (*test1)[0];
  for (int i = 0; i < 10; i++) {
    (*test1)[i] = i;
  }

  for (int i = 0; i < 10; i++) {
    GTEST_ASSERT_EQ((*test1)[i], i);
  }
  TestConnections(alloc);
}


// Test Performance

// single 1 billion byte alloc
TEST(PerformAlloc, largeAllocation1b){
  Allocator alloc(1000000000);
  GTEST_ASSERT_NE(alloc.malloc(1000000000), nullptr);
}
// 10M blocks
TEST(PerformAlloc, manyAlloc10M){
  Allocator alloc(10000000);
  for(int i = 0; i < 10000000; i++){
    GTEST_ASSERT_NE(alloc.malloc(1), nullptr);
  }
  GTEST_ASSERT_EQ(alloc.getFreeHead(), nullptr);
}

TEST(AllocatorRealloc, NullInput){
  Allocator alloc(8096);
  int* chunk = (int*)alloc.realloc(nullptr, 203);
  int **test = &chunk;
  GTEST_ASSERT_EQ(*test, nullptr);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, reallocToZero){
  Allocator alloc(8096);
  int* chunk = (int*)alloc.realloc(alloc.getOccHead(), 0);
  int **test2 = &chunk;
  GTEST_ASSERT_EQ(*test2, nullptr);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, GreaterThanAvailable){
  Allocator alloc(8096);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(33);
  int** test3 = (int**)alloc.malloc(55);
  test2 = (int**)alloc.realloc(*test2, 45);
  GTEST_ASSERT_EQ(*test2, alloc.getOccHead()->next->next->startLoc);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, ReUseSamePos){
  Allocator alloc(8096);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(33);
  int** test3 = (int**)alloc.malloc(35);
  int** test4 = (int**)alloc.malloc(55);

  alloc.free(*test3);
  test2 = (int**)alloc.realloc(*test2, 58);
  GTEST_ASSERT_EQ(*test2, (alloc.getOccHead()->next->startLoc));
  TestConnections(alloc);
}

TEST(AllocatorRealloc, InformationPreserved){
  Allocator alloc(8096);
  int** test1 = (int**)alloc.malloc(11);
  **test1 = 15;
  int** test2 = (int**)alloc.realloc(*test1, 43);
  GTEST_ASSERT_EQ(**test2, 15);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, CallocRealloc){
  Allocator alloc(8096);
  int** test1 = (int**)alloc.calloc(5, 11);
  for(int i =0; i < 5; i++){
    (*test1)[i] = i*5;
  }
  int** test2 = (int**)alloc.realloc(*test1, 43);
  for(int i =0; i < 5; i++){
    GTEST_ASSERT_EQ((*test2)[i], i*5);
  }
  TestConnections(alloc);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
