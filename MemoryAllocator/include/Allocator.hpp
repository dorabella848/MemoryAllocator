#include <new>
#include <cstdint>
struct Chunk;

class Allocator {
    private:
        int memorySize;
        int freeMemory;
        static inline uint8_t* memoryPool;
        Chunk* occHead = nullptr;
        Chunk* freeHead = nullptr;
        
    public:
        Allocator(std::size_t numBytes);
        ~Allocator();
        Chunk* getFreeHead();
        Chunk* getOccHead();
        void* getMemAddress(std::size_t index);
        void printChunks();
        void** malloc(std::size_t size);
        void free(void* ptr);
        void defragment();
        void** calloc(std::size_t number, std::size_t size);
        void** realloc(void* ptr, std::size_t size);
};