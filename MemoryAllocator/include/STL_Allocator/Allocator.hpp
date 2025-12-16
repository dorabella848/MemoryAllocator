#include <new>
#include <cstdint>
struct Chunk;

class Allocator {
    private:
        std::size_t memorySize;
        std::size_t freeMemory;
        uint8_t* memoryPool;
        Chunk* occHead = nullptr;
        Chunk* freeHead = nullptr;
        
    public:
        Allocator(std::size_t numBytes);
        ~Allocator();
        Chunk* getFreeHead();
        Chunk* getOccHead();
        std::size_t getFreeMemory();
        std::size_t getMemoryTotal();
        void* getMemAddress(std::size_t index);
        void printChunks();
        void* malloc(std::size_t size);
        void free(void* ptr);
        void* calloc(std::size_t number, std::size_t size);
        void* realloc(void* ptr, std::size_t size);
};