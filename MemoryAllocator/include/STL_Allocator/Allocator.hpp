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
        /*
        * returns a free block that is the result of merging any adjacent free blocks
        */
        Chunk* merge(Chunk* newFree);
        /*
        *    Updates the next, prev, and free vars of a chunk
        *    ref chunk is the closest chunk behind or after toInsert of the opposite type
        */
        void insert_referece(Chunk *toInsert, Chunk *refChunk);
        /*
        *    inserts a chunk into the opposite free type list
        */
        void insert(Chunk *toInsert);
        
    public:
        Allocator(std::size_t numBytes);
        ~Allocator();
        Chunk* getFreeHead();
        Chunk* getOccHead();
        Chunk* getTrueHead();
        std::size_t getFreeMemory();
        std::size_t getMemoryTotal();
        void* getMemAddress(std::size_t index);
        void printChunks();
        void* malloc(std::size_t size);
        void free(void* ptr);
        void* calloc(std::size_t number, std::size_t size);
        void* realloc(void* ptr, std::size_t size);
};
