#include "STL_Allocator/STL_Allocator.hpp"
#include <string>
#include <iostream>

int main()
{
  Allocator alloc(100);
  void* ptr = alloc.malloc(5);
  void* ptr2 = alloc.malloc(60);
  void* ptr3 = alloc.malloc(20);
  alloc.free(ptr2);
  ptr = alloc.realloc(ptr, 65);
  alloc.printChunks();

};