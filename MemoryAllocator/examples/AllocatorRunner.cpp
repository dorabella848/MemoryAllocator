#include "STL_Allocator/STL_Allocator.hpp"
#include <string>
#include <iostream>

int main()
{
  Allocator alloc(100);
  void* ptr = alloc.malloc(55);
  alloc.realloc(ptr, 4);
  alloc.printChunks();
};