#include "Allocator.cpp"
#include <iostream>
int main()
{
  Allocator alloc(8096);
  int** test1 = (int**)alloc.calloc(5, 11);
  alloc.printChunks();
  for(int i = 0; i < 5; i++){
    (*test1)[i] = i*5;
  }
  int** test2 = (int**)alloc.realloc(*test1, 80);
  for(int i = 0; i < 5; i++){
    cout << (*test2)[i] << " ";
  }
  alloc.printChunks();


};