#include <iostream>
#include <unistd.h>
#include <new>
#include <list>
#include <cstring>
#include <vector> 
#include "Chunk.hpp"
using namespace std;

class Allocator {
    private:
        int memorySize;
        static inline uint8_t* memoryPool;
        Chunk* occHead = nullptr;
        Chunk* freeHead = nullptr;
        
    public:
        
        Allocator(int numBytes){
            memorySize = numBytes;
            memoryPool = new uint8_t[memorySize];
            freeHead = new Chunk(0, memorySize, true);
            (*freeHead).startLoc = &memoryPool[0];
        };
        Chunk* getFreeHead();
        Chunk* getOccHead();
        void* getMemAddress(size_t index);
        void printChunks();
        void *malloc(size_t size);
        void free(void* ptr);
        void defragment();
        void* calloc(size_t number, size_t size);
        void* realloc(void* ptr, size_t size);
        
        ~Allocator(){
            delete[] memoryPool;
            // Delete all free and occupied chunks
            Chunk* occCurrent = occHead;
            Chunk* freeCurrent = freeHead;
            Chunk* occNext = nullptr;
            Chunk* freeNext = nullptr;

            while(occCurrent != nullptr){
                occNext = occCurrent->next;
                delete occCurrent;
                occCurrent = occNext;
            }

            while(freeCurrent != nullptr){
                freeNext = freeCurrent->next;
                delete freeCurrent;
                freeCurrent = freeNext;
            }
        }
    };
