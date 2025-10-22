#include "Allocator.cpp"
#include <iostream>
int main()
{
   Allocator allocator(10);
  allocator.malloc(10);
  allocator.printChunks();

  // Maybe we could get rid of the prev variable in the chunks and in the functions
  // since its not used for anything and is just bloating overhead and reducing readability.


};