#include <iostream>
#include <unistd.h>
#include <new>
#include <list>
#include <cstring>
#include <vector> 
#include "Chunk.hpp"
using namespace std;

class MemoryAllocator {
    private:
        int minChunkSize;
        int memorySize;
        static inline uint8_t* memoryPool;
        Chunk* occHead;
        Chunk* freeHead;
        
    public:
        
        MemoryAllocator(int numBytes){
            memorySize = numBytes;
            memoryPool = new uint8_t[memorySize];
            freeHead = new (&memoryPool[0]) Chunk(0, memorySize, true);
            (*freeHead).startLoc = &memoryPool[0];
        };
        Chunk* getFreeHead();
        void printChunks();
        void *malloc(size_t size);
        void free(void* ptr);
        void defragment();
        void* calloc(size_t number, size_t size);
        void* realloc(void* ptr, size_t size);
    };
