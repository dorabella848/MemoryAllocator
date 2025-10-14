#include "Allocator.cpp"
#include <iostream>
int main()
{
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(22);
  int** test3 = (int**)alloc.malloc(33);

  alloc.free(*test2);
  alloc.defragment();

  alloc.printChunks();
  
  // Defragment isnt working properly
};