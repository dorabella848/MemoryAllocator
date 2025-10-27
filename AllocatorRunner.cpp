#include "Allocator.cpp"
#include <iostream>
int main()
{
  Allocator alloc(8096);
  int** test1 = (int**)alloc.calloc(5, 11);
  for(int i =0; i < 5; i++){
    (*test1)[i] = i*5;
  }
  int** test2 = (int**)alloc.realloc(*test1, 43);
  alloc.printChunks();

};