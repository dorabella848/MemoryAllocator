#include <gtest/gtest.h>
#include "memoryAllocator/stl.hpp"
#include "chunk.hpp"

// Move build files out of main directory,
// you can still call cmake using ../(whatever folder has CMakeLists.txt which should be MemoryAllocator)

// valgrind --leak-check=full --track-origins=yes ./build/allocatortest
// --gtest_filter={TestSuite}.{TestName}
// make
// ctest

// Test allocating almost all available memory (e.g., 9 out of 10)
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

TEST(AllocatorSTL, STLmalloc){
  std::vector<int, StlAllocator<int>> vec;
  for(int i = 0; i < 100; i++){
    vec.push_back(i);
  }
  for(int i = 0; i < 100; i++){
    GTEST_ASSERT_EQ(vec.at(i), i);
  }
}

TEST(AllocatorSTL, STLtestConnections){
  StlAllocator<int> alloc;
  alloc.allocate(5);
  int* ptr = alloc.allocate(5);
  alloc.allocate(5);
  alloc.deallocate(ptr, 0);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

TEST(AllocatorSTL, NonDefaultConstructor){
  StlAllocator<int> alloc(14);
  alloc.allocate(5);
  alloc.allocate(5);
  GTEST_ASSERT_EQ(alloc.allocate(5), nullptr);
  TestConnections(alloc.getFreeHead(), alloc.getMemoryTotal());
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
