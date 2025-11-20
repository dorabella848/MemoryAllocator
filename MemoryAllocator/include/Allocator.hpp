#include <new>
#include <cstdint>
struct Chunk;

class Allocator {
    private:
        int memorySize;
        int freeMemory;
        uint8_t* memoryPool;
        Chunk* occHead = nullptr;
        Chunk* freeHead = nullptr;
        
    public:
        Allocator(std::size_t numBytes);
        ~Allocator();
        Chunk* getFreeHead();
        Chunk* getOccHead();
        int getFreeMemory();
        void* getMemAddress(std::size_t index);
        void printChunks();
        void* malloc(std::size_t size);
        void free(void* ptr);
<<<<<<< HEAD
        void** calloc(std::size_t number, std::size_t size);
        void** realloc(void* ptr, std::size_t size);
};
=======
        void defragment();
        void* calloc(std::size_t number, std::size_t size);
        void* realloc(void* ptr, std::size_t size);
        
        
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
>>>>>>> 04b24b9 (remove double pointers)
