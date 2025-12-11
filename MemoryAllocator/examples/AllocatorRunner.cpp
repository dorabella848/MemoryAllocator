#include "Allocator.hpp"
#include <iostream>
#include <vector>
int main()
{

  Allocator<int> alloc(8096);
  int* chunk = alloc.allocate(10);
  chunk = (int*)alloc.reallocate(alloc.getOccHead()->startLoc, 0);

  alloc.printChunks();
  
  
  std::vector<int> vec1 = {10,20,30};
  for (auto x: vec1) std::cout << x << " ";
  std::cout << '\n';

  // explicitly using custom allocator
  std::vector<int, Allocator<int>> vec2 = {10,20,30};
  for (auto x: vec2) std::cout << x << " ";
  std::cout << '\n';

  return 0;

};