#include "Allocator.cpp"
#include <iostream>
int main()
{
  Allocator alloc(8096);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(33);
  int** test3 = (int**)alloc.malloc(55);
  alloc.free(*test2);
  int** test4 = (int**)alloc.malloc(45);
  //test2 = (int**)alloc.realloc(*test2, 45);
  alloc.printChunks();

  // Maybe we could get rid of the prev variable in the chunks and in the functions
  // since its not used for anything and is just bloating overhead and reducing readability.
  // maybe rewrite chunksize to be a size_t

};