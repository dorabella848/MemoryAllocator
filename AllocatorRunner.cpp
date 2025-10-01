#include "Allocator.cpp"
#include <iostream>
int main()
{
  Allocator alloc(10);
  alloc.printChunks();
  alloc.malloc(10);
  // int** test1 = (int**)alloc.malloc(11);
  // alloc.free(*test1);
   alloc.printChunks();
};
// newfree seems to be too high (8288 higher than test1)