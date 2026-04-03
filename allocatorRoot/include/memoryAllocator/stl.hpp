#include "memoryAllocator/allocator.hpp"
#include <stdlib.h>
#include <memory>

template <typename T>
class StlAllocator{
public:
  using value_type = T;

  StlAllocator (const StlAllocator & other) = delete; // copy constructor
  StlAllocator (StlAllocator && other) = delete; // move constructor
  StlAllocator & operator=(const StlAllocator & other) = delete; // copy assignment
  StlAllocator & operator=(StlAllocator && other) = delete; // move assignment
  explicit StlAllocator(std::size_t blockCount = 1024) : alloc(std::make_shared<Allocator>(blockCount*sizeof(T))) {}
  /*
  * Allocates a chunk of {n * sizeof(T)} bytes and returns a T* to the chunk.
  * T is the datatype established during the creation of the allocator.
  */
  value_type* allocate(size_t n) {
    return (value_type*) alloc->malloc(sizeof(value_type) * n);
  }
  /*
  * Unreserves the memory for a given chunk in the memory pool.
  * Because this is an interface class that interacts with a non-fixed size memory block pool
  * inputting a value for n does not do anything.
  */
  void deallocate(value_type* p, size_t n=0) {
    alloc->free(p);
  }
  /*
  * Returns a Chunk* that points to the first free chunk.
  */
  Chunk* getFreeHead() { return alloc->getFreeHead(); };
  /*
  * Returns a Chunk* that points to the first occupied chunk.
  */
  Chunk* getOccHead() { return alloc->getOccHead(); };
  /*
  * Returns the remaining free memory.
  */
  std::size_t getFreeMemory() { return alloc->getFreeMemory(); };
  /*
  * Returns the total memory reserved for the memory pool.
  */
  std::size_t getMemoryTotal() { return alloc->getMemoryTotal(); };
  /* 
  * Returns a T* to an index within the memory pool.
  *
  * This is a testing function that can be used to ensure that the 
  * startIndex var stored within a chunk alligns with startLoc var for a chunk.
  */
  value_type* getMemAddress(std::size_t index){ 
    return static_cast<value_type>(alloc->getMemAddress(index)); 
  };
  /*
  * Prints the current memory pool's statistics and the chunk's connections, postions, and size 
  * in order of occurence within the memory pool to the console.
  */
  void printChunks(){ alloc->printChunks(); };
private:
  // basic_string makes a copy of this stl allocator, but we want it share the same 
  // memory pool, so make this a pointer. The default copy ctor will copy the pointer between
  // new instances
  std::shared_ptr<Allocator> alloc;
};
