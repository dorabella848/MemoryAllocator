#include "Allocator.cpp"
#include <iostream>
int main()
{
  Allocator alloc(8192);
  int** test1 = (int**)alloc.malloc(11);
  int** test2 = (int**)alloc.malloc(22);
  int** test3 = (int**)alloc.malloc(33);
  alloc.free(*test2);
  alloc.defragment();
  cout << (alloc.getFreeHead()->startIndex) << endl;
  cout << alloc.getOccHead()->next->startIndex << endl;  
  cout << alloc.getOccHead()->next->startLoc << " EEEE " << *test3 << endl;
  cout << alloc.getOccHead()->startLoc << " EEEEE " << *test1 << endl;
  // // 44 test1 chunkSize + test3 chunkSize
  cout << alloc.getFreeHead()->startIndex << endl;
  cout << alloc.getFreeHead()->startLoc << endl;
  cout << alloc.getMemAddress(alloc.getFreeHead()->startIndex) << endl;
};