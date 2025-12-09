#include "Allocator.hpp"
#include <iostream>
#include <vector>
int main()
{
  Allocator alloc(8192);
  
  std::vector<int> vec1 = {10,20,30};
  for (auto x: vec1) std::cout << x << " ";
  std::cout << '\n';

  // explicitly using custom allocator
  std::vector<int, Allocator> vec2 = {10,20,30};
  for (auto x: vec2) std::cout << x << " ";
  std::cout << '\n';

  return 0;

};