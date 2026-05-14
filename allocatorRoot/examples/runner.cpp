#include "memoryAllocator/stl.hpp"
#include <string>
#include <iostream>
#include "chunk.hpp"
using namespace std;
int main()
{
  Allocator alloc(8192);
  
  alloc.printChunks();
};