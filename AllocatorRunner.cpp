#include "Allocator.cpp"
#include <iostream>
int main()
{
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(22);
  int** test3 = (int**)alloc.malloc(33);
  alloc.printChunks();
  alloc.free(*test2);
  //GTEST_ASSERT_TRUE(alloc.getOccHead()->next->next == nullptr);
  //GTEST_ASSERT_TRUE(alloc.getOccHead()->next->chunkSize == 33);
   alloc.printChunks();
};
// newfree seems to be too high (8288 higher than test1)