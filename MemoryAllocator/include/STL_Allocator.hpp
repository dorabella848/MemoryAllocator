#include "Allocator.hpp"
#include <stdlib.h>
#include <memory>

template <typename T>
class StlAllocator{
public:
  using value_type = T;

  StlAllocator() : alloc(std::make_shared<Allocator>(1024)) {}

  value_type* allocate(size_t n) {
    return (value_type*) alloc->malloc(sizeof(value_type) * n);
  }

  void deallocate(value_type* p, size_t n) {
    alloc->free(p);
  }
  Chunk* getFreeHead() { return alloc->getFreeHead(); };
  Chunk* getOccHead() { return alloc->getOccHead(); };
  std::size_t getFreeMemory();
  std::size_t getMemoryTotal();
  value_type* getMemAddress(std::size_t index){ 
    return static_cast<value_type>(alloc->getMemAddress(index)); 
  };
  void printChunks(){ alloc->printChunks(); };
  operator Allocator&() { return *alloc; }
private:
  // basic_string makes a copy of this stl allocator, but we want it share the same 
  // memory pool, so make this a pointer. The default copy ctor will copy the pointer between
  // new instances
  std::shared_ptr<Allocator> alloc;
};
