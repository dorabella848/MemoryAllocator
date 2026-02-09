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
        /*
        * Returns a Chunk* that points to the first free chunk.
        */
        Chunk* getFreeHead();
        /*
        * Returns a Chunk* that points to the first occupied chunk.
        */
        Chunk* getOccHead();
        /*
        * Returns a Chunk* that points to the chunk that occurs at the start of the memory pool.
        */
        Chunk* getTrueHead();
        /*
        * Returns the remaining free memory.
        */
        std::size_t getFreeMemory();
        /*
        * Returns the total memory reserved for the memory pool.
        */
        std::size_t getMemoryTotal();
        /* 
        * Returns a void* to an index within the memory pool.
        *
        * This is a testing function that can be used to ensure that the 
        * startIndex var stored within a chunk alligns with startLoc var for a chunk.
        */
        void* getMemAddress(std::size_t index);
        /*
        * Sets chunkSize for a chunk
        *
        * If the chunkSize is equal to 0, then the function also calls orphan().
        * Outputs an error to the console if the newSize is < 0.
        */
        void updateSize(Chunk* target, std::size_t newSize);
        /*
        * Prints the current memory pool's statistics and the chunk's connections, postions, and size 
        * in order of occurence within the memory pool to the console.
        */
        void printChunks();
        /*
        * Allocates a chunk of {size} bytes and returns a void* to the chunk.
        */
        void* malloc(std::size_t size);
        /*
        * Unreserves the memory for a given chunk in the memory pool.
        */
        void free(void* ptr);
        /*
        * Allocates a chunk of {number * size} bytes, sets the newly reserved memory to 0, 
        * and returns a void* to the chunk.
        */
        void* calloc(std::size_t number, std::size_t size);
        /*
        * Changes the number of reserved bytes for a given chunk. This may cause the position of 
        * the reserved chunk to move so it returns a void* the chunk regardless of if it was
        * moved or not.
        */
        void* realloc(void* ptr, std::size_t size);
};
