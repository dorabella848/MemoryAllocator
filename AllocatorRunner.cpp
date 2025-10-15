#include "Allocator.cpp"
#include <iostream>
int main()
{
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(33);
  int** test3 = (int**)alloc.calloc(10, sizeof(int));

  alloc.free(*test2);
  alloc.defragment();
  alloc.printChunks();

  // I finished calloc and added some unit tests for it
  // the way it works is that it calls malloc and then returns the pointer to the memory block similar
  // to how malloc's output is utilized, you can assign values to the indexs of an array
  // created using malloc by dereferencing the pointer a single time like: (*arr)[0] = 5;
  // this will assign the first index of the array to 5
  // The reason double pointers are used is the same for malloc in that we want to protect referencing after
  // defragmentation.
  
};