#include <gtest/gtest.h>
#include "memoryAllocator/stl.hpp"
#include "chunk.hpp"

// Move build files out of main directory,
// you can still call cmake using ../(whatever folder has CMakeLists.txt which should be MemoryAllocator)

// valgrind --leak-check=full --track-origins=yes ./build/allocatortest
// --gtest_filter={TestSuite}.{TestName}
// make
// ctest

// Tests the free chunks and ensures there are no adjacent free chunks
// and the total allocated memroy listed is in line with the total memory
// in the alloc.
void TestConnections(chunk* freeHead, std::size_t totalMemory){
    chunk* currentChunk = freeHead;
    while(currentChunk != nullptr){
      size_t curEnd = currentChunk->startIndex + currentChunk->chunkSize;
      if(currentChunk->next != nullptr){
        GTEST_ASSERT_NE(curEnd, currentChunk->next->startIndex);
      }
      currentChunk = currentChunk->next;
    }
}

TEST(AllocatorMalloc, MallocFull) {
  Allocator alloc(10);
  void* ptr = alloc.malloc(9);
  GTEST_ASSERT_NE(ptr, nullptr);
  GTEST_ASSERT_EQ(alloc.malloc(1), nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorMalloc, newOccHead) {
  Allocator alloc(10);
  void* ptr1 = alloc.malloc(1);
  void* ptr2 = alloc.malloc(7);
  alloc.free(ptr1);
  void* ptr3 = alloc.malloc(1);
  GTEST_ASSERT_NE(ptr2, nullptr);
  //GTEST_ASSERT_NE(ptr3, nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

// Test allocating the last available chunk of memory
TEST(AllocatorMalloc, MallocValidSize_LastChunk) {
  Allocator alloc(10);
  void* ptr = alloc.malloc(7); // Allocate first part
  void* ptr1 = alloc.malloc(1);
  GTEST_ASSERT_NE(ptr, nullptr);
  GTEST_ASSERT_NE(ptr1, nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

// Test allocating zero bytes
TEST(AllocatorMalloc, MallocZeroBytes) {
  Allocator alloc(10);
  GTEST_ASSERT_EQ(alloc.malloc(0), nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

// Test allocating more memory than available
TEST(AllocatorMalloc, MallocTooMuch) {
  Allocator alloc(10);
  GTEST_ASSERT_EQ(alloc.malloc(10), nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, SingleBlockFreeTest){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(11);
  alloc.free(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, CallocFreeSingleIndex){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.calloc(1, sizeof(int*));
  alloc.free(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);

  GTEST_ASSERT_EQ(alloc.getFreeHead()->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->prev, nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, CallocFreeMultipleIndex){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.calloc(10, sizeof(int*));
  alloc.free(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);

  GTEST_ASSERT_EQ(alloc.getFreeHead()->next, nullptr);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->prev, nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, FragmentedFree){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(11);
  int* test2 = (int*)alloc.malloc(22);
  int* test3 = (int*)alloc.malloc(33);

  GTEST_ASSERT_NE(test1, nullptr);
  GTEST_ASSERT_NE(test2, nullptr);
  GTEST_ASSERT_NE(test3, nullptr);

  alloc.free(test2);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}
TEST(AllocatorFree, InformationOverwriteFree){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(11);
  *test1 = 5;
  alloc.free(test1);
  int* test2 = (int*)alloc.malloc(22);
  *test2 = 10;
  GTEST_ASSERT_EQ(*test2, 10);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, FreeChunkAhead){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(11);
  int* test2 = (int*)alloc.malloc(22);
  int* test3 = (int*)alloc.malloc(32);
  int* test4 = (int*)alloc.malloc(42);
  alloc.free(test3);
  alloc.free(test2);
  GTEST_ASSERT_NE(test1, nullptr);
  GTEST_ASSERT_NE(test4, nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, FreeChunkBehind){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(11);
  int* test2 = (int*)alloc.malloc(22);
  int* test3 = (int*)alloc.malloc(32);
  int* test4 = (int*)alloc.malloc(42);
  alloc.free(test2);
  alloc.free(test3);
  GTEST_ASSERT_NE(test1, nullptr);
  GTEST_ASSERT_NE(test4, nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, NoFreeHead){
  Allocator alloc(8192);
  int* test1 = (int*)alloc.malloc(8191);
  alloc.free(test1);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->startIndex, 0);
  GTEST_ASSERT_EQ(alloc.getFreeHead()->chunkSize, 8192);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
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
  GTEST_ASSERT_EQ(alloc.getFreeHead()->next->startLoc, alloc.getMemAddress(alloc.getFreeHead()->next->startIndex));
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, freeNullptr){
  Allocator alloc(8096);
  alloc.free(nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, newOccHeadNext){
  Allocator alloc(8096);
  void* ptr = alloc.malloc(1);
  void* ptr2 = alloc.malloc(3);
  alloc.free(ptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorFree, testFreeAdjacentConnections){
  Allocator alloc(8096);
  void* ptr = alloc.malloc(1);
  void* ptr2 = alloc.malloc(3);
  void* ptr3 = alloc.malloc(1);
  alloc.free(ptr);
  alloc.free(ptr3);
  alloc.free(ptr2);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
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
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
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
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, NullInput){
  Allocator alloc(8096);
  int* chunk = (int*)alloc.realloc(nullptr, 203);
  GTEST_ASSERT_EQ(chunk, nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, reallocToZero){
  Allocator alloc(8096);
  void* chunk = alloc.malloc(5);
  chunk = alloc.realloc(nullptr, 0);
  GTEST_ASSERT_EQ(chunk, nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, GreaterThanAvailable){
  Allocator alloc(8096);
  int* test1 = (int*)alloc.malloc(11);
  int* test2 = (int*)alloc.malloc(33);
  int* test3 = (int*)alloc.malloc(55);
  test2 = (int*)alloc.realloc(test2, 45);
  GTEST_ASSERT_NE(test1, nullptr);
  GTEST_ASSERT_NE(test3, nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, ReUseSamePos){
  Allocator alloc(8096);
  int* test1 = (int*)alloc.malloc(11);
  int* test2 = (int*)alloc.malloc(33);
  int* test3 = (int*)alloc.malloc(35);
  int* test4 = (int*)alloc.malloc(55);

  alloc.free(test3);
  test2 = (int*)alloc.realloc(test2, 58);
  GTEST_ASSERT_NE(test1, nullptr);
  GTEST_ASSERT_NE(test4, nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, InformationPreserved){
  Allocator alloc(8096);
  int* test1 = (int*)alloc.malloc(11);
  *test1 = 15;
  int* test2 = (int*)alloc.realloc(test1, 43);
  GTEST_ASSERT_EQ(*test2, 15);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
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
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, ReallocNullptr){
  Allocator alloc(8096);
  GTEST_ASSERT_EQ(alloc.realloc(nullptr, 5), nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, ReallocTooMuch){
  Allocator alloc(100);
  void* ptr = alloc.malloc(5);
  GTEST_ASSERT_EQ(alloc.realloc(ptr, 1000), ptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, ReallocMore){
  Allocator alloc(100);
  void* ptr = alloc.malloc(5);
  GTEST_ASSERT_NE(alloc.realloc(ptr, 55), nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, ReallocLess){
  Allocator alloc(100);
  void* ptr = alloc.malloc(55);
  GTEST_ASSERT_NE(alloc.realloc(ptr, 4), nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorRealloc, ReallocMoreNewFreeHead){
  Allocator alloc(100);
  void* ptr = alloc.malloc(5);
  void* ptr2 = alloc.malloc(60);
  void* ptr3 = alloc.malloc(20);
  alloc.free(ptr2);
  GTEST_ASSERT_NE(ptr, nullptr);
  GTEST_ASSERT_NE(ptr3, nullptr);
  GTEST_ASSERT_NE(ptr = alloc.realloc(ptr, 65), nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
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
