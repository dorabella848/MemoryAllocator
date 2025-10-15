#include "Allocator.cpp"
#include <iostream>
int main()
{
  Allocator alloc(8096);
  //int** test1 = (int**)alloc.malloc(10, sizeof(int));
  int** test1 = (int**)alloc.calloc(10, sizeof(int*));
  for (int i = 0; i < 10; ++i) {
      test1[i] = (int*)alloc.calloc(10, sizeof(int));
  }
  test1[2][0] = 56;

  alloc.printChunks();
  for (int i = 0; i < 10; ++i) {
    std::cout << test1[i][0] << " ";
  }
  
};