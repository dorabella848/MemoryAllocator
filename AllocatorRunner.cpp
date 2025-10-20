#include "Allocator.cpp"
#include <iostream>
int main()
{
  Allocator alloc(8096);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(33);
  int** test3 = (int**)alloc.malloc(35);
  int** test4 = (int**)alloc.malloc(55);

  alloc.free(*test3);
  alloc.printChunks();
  //alloc.free(*test2);
  //int** test5 = (int**)alloc.malloc(58);
  test2 = (int**)alloc.realloc(*test2, 58);
  alloc.printChunks();

  // The error was caused by deleteChunk() in free() because it was setting 
  // occHead to nullptr if the deleted chunk wasnt the occHead
  

};