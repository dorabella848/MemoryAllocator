#include <gtest/gtest.h>
#include "STL_Allocator/STL_Allocator.hpp"
#include "STL_Allocator/Chunk.hpp"
#include <random>

// Move build files out of main directory,
// you can still call cmake using ../(whatever folder has CMakeLists.txt which should be MemoryAllocator)

// valgrind --leak-check=full --track-origins=yes ./build/allocatortest
// --gtest_filter={TestSuite}.{TestName}
// make
// ctest

// Test allocating almost all available memory (e.g., 9 out of 10)
void TestConnections(Chunk* occHead, Chunk* freeHead, std::size_t freeMemory, std::size_t totalMemory){
  // Finding first chunk
  Chunk* currentChunk = nullptr;
  if(occHead == nullptr){
      currentChunk = freeHead;
  }
  else if(freeHead == nullptr){
      currentChunk = occHead;

  }
  else if(occHead->startIndex == 0){
      currentChunk = occHead;
  }
  else{
      currentChunk = occHead;
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
  GTEST_ASSERT_EQ(totalFreeMem, freeMemory);
  GTEST_ASSERT_EQ(totalMem, totalMemory);
}

TEST(AllocatorMalloc, MallocValidSize_AlmostFull) {
  Allocator allocator(10);
  GTEST_ASSERT_NE(allocator.malloc(10), nullptr);
  TestConnections(allocator.getOccHead(), allocator.getFreeHead(), allocator.getFreeMemory(), allocator.getMemoryTotal());
}

TEST(AllocatorMalloc, MallocFull) {
  Allocator allocator(10);
  GTEST_ASSERT_NE(allocator.malloc(10), nullptr);
  GTEST_ASSERT_EQ(allocator.malloc(1), nullptr);
  TestConnections(allocator);
}

TEST(AllocatorMalloc, newOccHead) {
  Allocator alloc(10);
  void* ptr = alloc.malloc(1);
  alloc.malloc(9);
  alloc.free(ptr);
  GTEST_ASSERT_NE(alloc.malloc(1), nullptr);
  TestConnections(alloc);
}

// Test allocating the last available chunk of memory
TEST(AllocatorMalloc, MallocValidSize_LastChunk) {
  Allocator allocator(10);
  allocator.malloc(9); // Allocate first part
  GTEST_ASSERT_NE(allocator.malloc(1), nullptr);
  TestConnections(allocator.getOccHead(), allocator.getFreeHead(), allocator.getFreeMemory(), allocator.getMemoryTotal());
}

// Test allocating zero bytes
TEST(AllocatorMalloc, MallocZeroBytes) {
  Allocator allocator(10);
  GTEST_ASSERT_EQ(allocator.malloc(0), nullptr);
  TestConnections(allocator.getOccHead(), allocator.getFreeHead(), allocator.getFreeMemory(), allocator.getMemoryTotal());
}

// Test allocating more memory than available
TEST(AllocatorMalloc, MallocTooMuch) {
  Allocator allocator(10);
  GTEST_ASSERT_EQ(allocator.malloc(11), nullptr);
  TestConnections(allocator.getOccHead(), allocator.getFreeHead(), allocator.getFreeMemory(), allocator.getMemoryTotal());
}

TEST(AllocatorMalloc, MallocNegative){
  Allocator allocator(10);
  GTEST_ASSERT_EQ(allocator.malloc(-1), nullptr);
  TestConnections(allocator.getOccHead(), allocator.getFreeHead(), allocator.getFreeMemory(), allocator.getMemoryTotal());
}

TEST(AllocatorMalloc, MallocFullPoolSize1Blocks){
  Allocator allocator(10);
  for(int i = 0; i < 10; i++){
    GTEST_ASSERT_NE(allocator.malloc(1), nullptr);
  }
  TestConnections(allocator.getOccHead(), allocator.getFreeHead(), allocator.getFreeMemory(), allocator.getMemoryTotal());
}

TEST(AllocatorFree, SingleBlockFreeTest){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(11);
  alloc.free(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, CallocFreeSingleIndex){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.calloc(1, sizeof(int*));
  alloc.free(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);

  GTEST_ASSERT_EQ(alloc.getFreeHead()->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->prev, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsNext, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsPrev, nullptr);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, CallocFreeMultipleIndex){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.calloc(10, sizeof(int*));
  alloc.free(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);

  GTEST_ASSERT_EQ(alloc.getFreeHead()->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->prev, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsNext, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->AbsPrev, nullptr);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, FragmentedFree){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(11);
  int* test2 = (int*)alloc.malloc(22);
  int* test3 = (int*)alloc.malloc(33);
  alloc.free(test2);
  GTEST_ASSERT_EQ(alloc.getOccHead()->next->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getOccHead()->next->chunkSize, 33);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}
TEST(AllocatorFree, InformationOverwriteFree){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(11);
  *test1 = 5;
  alloc.free(test1);
  int* test2 = (int*)alloc.malloc(22);
  *test2 = 10;
  GTEST_ASSERT_EQ(*test2, 10);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, FreeChunkAhead){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(11);
  int* test2 = (int*)alloc.malloc(22);
  int* test3 = (int*)alloc.malloc(32);
  int* test4 = (int*)alloc.malloc(42);
  alloc.free(test3);
  alloc.free(test2);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 11);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 54);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, test2);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, FreeChunkBehind){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(11);
  int* test2 = (int*)alloc.malloc(22);
  int* test3 = (int*)alloc.malloc(32);
  int* test4 = (int*)alloc.malloc(42);
  alloc.free(test2);
  alloc.free(test3);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 11);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 54);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, test2);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, NoFreeHead){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(8192);
  alloc.free(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 0);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);
  GTEST_ASSERT_EQ(alloc.getOccHead(), nullptr);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, AfterFreeHead_No_Adj_Free){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(5);
  int* test2 = (int*)alloc.malloc(11);
  int* test3 = (int*)alloc.malloc(16);
  int* test4 = (int*)alloc.malloc(27);
  int* test5 = (int*)alloc.malloc(43);
  alloc.free(test2);
  alloc.free(test4);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, test2);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->next->startLoc, test4);
  GTEST_ASSERT_EQ(alloc.getOccHead()->startLoc, test1);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, freeNullptr){
  Allocator alloc(8096);
  alloc.free(nullptr);
  TestConnections(alloc);
}

TEST(AllocatorFree, newOccHeadNext){
  Allocator alloc(8096);
  void* ptr = alloc.malloc(1);
  void* ptr2 = alloc.malloc(3);
  alloc.free(ptr);
  GTEST_ASSERT_EQ(alloc.getOccHead()->startLoc, ptr2);
  TestConnections(alloc);
}

TEST(AllocatorFree, testFreeAdjacentConnections){
  Allocator alloc(8096);
  void* ptr = alloc.malloc(1);
  void* ptr2 = alloc.malloc(3);
  void* ptr3 = alloc.malloc(1);
  alloc.free(ptr);
  alloc.free(ptr3);
  alloc.free(ptr2);
  TestConnections(alloc);
}

TEST(AllocatorFree, testFreeIteratorWhenInserting){
  Allocator alloc(8096);
  void* ptr;
  for(int i = 0; i < 10; i++){
    ptr = alloc.malloc(i);
    // This will test how free handles connections when it doesnt know where the occupied chunk is supposed
    // to be in the free list
    if(i==0 || i==4 || i==6){
      alloc.free(ptr);
    }
  }
  TestConnections(alloc);
}

TEST(AllocatorCalloc, AllAssigned){
  Allocator alloc(8096);
  int* test1 = (int*)alloc.calloc(10, sizeof(int));

  //cout << (*test1)[0];
  for (int i = 0; i < 10; i++) {
    test1[i] = i;
  }

  for (int i = 0; i < 10; i++) {
    GTEST_ASSERT_EQ(test1[i], i);
  }
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, NullInput){
  Allocator alloc(8096);
  int* chunk = (int*)alloc.realloc(nullptr, 203);
  GTEST_ASSERT_EQ(chunk, nullptr);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, reallocToZero){
  Allocator alloc(8096);
  int* chunk = (int*)alloc.realloc(alloc.getOccHead(), 0);
  GTEST_ASSERT_EQ(chunk, nullptr);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, GreaterThanAvailable){
  Allocator alloc(8096);
  int* test1 = (int*)alloc.malloc(11);
  int* test2 = (int*)alloc.malloc(33);
  int* test3 = (int*)alloc.malloc(55);
  test2 = (int*)alloc.realloc(test2, 45);
  GTEST_ASSERT_EQ(test2, alloc.getOccHead()->next->next->startLoc);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, ReUseSamePos){
  Allocator alloc(8096);
  int* test1 = (int*)alloc.malloc(11);
  int* test2 = (int*)alloc.malloc(33);
  int* test3 = (int*)alloc.malloc(35);
  int* test4 = (int*)alloc.malloc(55);

  alloc.free(test3);
  test2 = (int*)alloc.realloc(test2, 58);
  GTEST_ASSERT_EQ(test2, (alloc.getOccHead()->next->startLoc));
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, InformationPreserved){
  Allocator alloc(8096);
  int* test1 = (int*)alloc.malloc(11);
  *test1 = 15;
  int* test2 = (int*)alloc.realloc(test1, 43);
  GTEST_ASSERT_EQ(*test2, 15);
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, CallocRealloc){
  Allocator alloc(8096);
  int* test1 = (int*)alloc.calloc(5, 11);
  for(int i =0; i < 5; i++){
    test1[i] = i*5;
  }
  int* test2 = (int*)alloc.realloc(test1, 59);
  for(int i =0; i < 5; i++){
    GTEST_ASSERT_EQ(test2[i], i*5);
  }
  TestConnections(alloc.getOccHead(), alloc.getFreeHead(), alloc.getFreeMemory(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, ReallocNullptr){
  Allocator alloc(8096);
  GTEST_ASSERT_EQ(alloc.realloc(nullptr, 5), nullptr);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, ReallocTooMuch){
  Allocator alloc(100);
  void* ptr = alloc.malloc(5);
  GTEST_ASSERT_EQ(alloc.realloc(ptr, 1000), ptr);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, ReallocMore){
  Allocator alloc(100);
  void* ptr = alloc.malloc(5);
  GTEST_ASSERT_NE(alloc.realloc(ptr, 55), nullptr);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, ReallocLess){
  Allocator alloc(100);
  void* ptr = alloc.malloc(55);
  GTEST_ASSERT_NE(alloc.realloc(ptr, 4), nullptr);
  TestConnections(alloc);
}

TEST(AllocatorRealloc, ReallocMoreNewFreeHead){
  Allocator alloc(100);
  void* ptr = alloc.malloc(5);
  void* ptr2 = alloc.malloc(60);
  void* ptr3 = alloc.malloc(20);
  alloc.free(ptr2);
  GTEST_ASSERT_NE(alloc.realloc(ptr, 65), nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startLoc, ptr3);
  TestConnections(alloc);
}

TEST(getterFunctions, getMemAddress){
  Allocator alloc(8096);
  GTEST_ASSERT_NE(alloc.getMemAddress(0), nullptr);
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
