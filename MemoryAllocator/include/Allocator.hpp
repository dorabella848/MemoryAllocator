#include "Chunk.hpp"
#include <unistd.h>
#include <new>
#include <cstdint>
using namespace std;
class Allocator {
    private:
        int memorySize;
        int freeMemory;
        static inline uint8_t* memoryPool;
        Chunk* occHead = nullptr;
        Chunk* freeHead = nullptr;
        
    public:
        
        Allocator(size_t numBytes){
            memorySize = numBytes;
            freeMemory = numBytes;
            memoryPool = new uint8_t[memorySize];
            freeHead = new Chunk(0, memorySize, true);
            (*freeHead).startLoc = &memoryPool[0];
        };
        Chunk* getFreeHead();
        Chunk* getOccHead();
        void* getMemAddress(size_t index);
        void printChunks();
        void** malloc(size_t size);
        void free(void* ptr);
        void defragment();
        void** calloc(size_t number, size_t size);
        void** realloc(void* ptr, size_t size);
        
        
        ~Allocator(){
            delete[] memoryPool;
            // Delete all free and occupied chunks
            Chunk* currentChunk = nullptr;
            if(occHead == nullptr){
                currentChunk = freeHead;
            }
            else if(freeHead == nullptr){
                currentChunk = occHead;

            }
            else if(occHead->startIndex == 0){
                currentChunk = occHead;
            }
            else{
                currentChunk = freeHead;
            }

            Chunk* occCurrent = occHead;
            while(currentChunk != nullptr){
                if(currentChunk->AbsNext != nullptr){
                    currentChunk = currentChunk->AbsNext;
                    delete currentChunk->AbsPrev;
                }
                else{
                    delete currentChunk;
                    break;
                }
                

            }
        }
    };
