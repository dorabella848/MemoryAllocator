#include "STL_Allocator.hpp"
#include <string>
#include <iostream>

int main()
{
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(5);
  int** test2 = (int**)alloc.malloc(11);
  int** test3 = (int**)alloc.malloc(16);
  int** test4 = (int**)alloc.malloc(27);
  int** test5 = (int**)alloc.malloc(43);
  alloc.free(*test2);
  alloc.free(*test4);
  alloc.printChunks();
  // Keep track of all free chunks and implement defragment to automatically be called

  // Maybe we could get rid of the prev variable in the chunks and in the functions
  // since its not used for anything and is just bloating overhead and reducing readability.
  // maybe rewrite chunksize to be a size_t
  StlAllocator<char> alloctor;
  std::basic_string str("Hello, world!", alloctor);
  std::cout << str << std::endl;
};