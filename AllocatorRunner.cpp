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
  for(int i =0; i < 5; i++){
    cout << (*test2)[i] << " ";
  }

  // Maybe we could get rid of the prev variable in the chunks and in the functions
  // since its not used for anything and is just bloating overhead and reducing readability.


};