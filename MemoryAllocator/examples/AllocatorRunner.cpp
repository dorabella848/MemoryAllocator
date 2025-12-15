#include "Allocator.hpp"
#include <iostream>
#include <vector>
int main()
{
  Allocator<int> alloc(10);
  int* test1 = alloc.allocate(1);
  int* test2 = alloc.allocate(2);
  int* test3 = alloc.allocate(3);
  int* test4 = alloc.allocate(4);
  alloc.deallocate(test3);
  //alloc.deallocate(test2);
  alloc.printChunks();


};