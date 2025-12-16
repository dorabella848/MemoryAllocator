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
  TestConnections(alloc);
}

TEST(AllocatorSTL, NonDefaultConstructor){
  StlAllocator<int> alloc(14);
  alloc.allocate(5);
  alloc.allocate(5);
  GTEST_ASSERT_EQ(alloc.allocate(5), nullptr);
  TestConnections(alloc);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
