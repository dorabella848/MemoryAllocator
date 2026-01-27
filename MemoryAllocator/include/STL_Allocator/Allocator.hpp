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
        * Returns a chunk of the opposite type that is considered "adjacent" when in context
        * of the opposite type memory pool
        * 
        * Returns a nullptr if no opposite type chunk is found
        * 
        * This chunk may occur after or before the initialChunk
        */
        Chunk* findOppReference(Chunk* initialChunk);
        /*
        * Inserts a chunk into the opposite free type list by Updating the next, prev, and free vars of a chunk
        * 
        * ref chunk is the closest chunk behind or after toInsert of the opposite type
        * The intial chunk must at the very least have its absNext and absPrev variables properly assigned
        * 
        * No reference chunk is required if the opposite type list is empty
        */
        void insert(Chunk *toInsert, Chunk *refChunk=nullptr);
        
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
