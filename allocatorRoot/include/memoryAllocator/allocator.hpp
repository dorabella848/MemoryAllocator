#include <cstdint>
struct chunk;

class Allocator {
    private:
        std::size_t memorySize;
        uint8_t* memoryPool;
        chunk* freeHead = nullptr;
        /*
        * returns a free block that is the result of merging any adjacent free blocks
        */
        chunk* merge(chunk* newFree);
        /*
        * Sets chunkSize for a chunk
        *
        * If the chunkSize is equal to 0, then the function also calls orphan().
        * Outputs an error to the console if the newSize is < 0.
        */
        void updateSize(chunk* target, std::size_t newSize);
        
    public:
        Allocator(std::size_t numBytes);
        Allocator (const Allocator & other) = delete; // copy constructor
        Allocator (Allocator && other) = delete; // move constructor
        Allocator & operator=(const Allocator & other) = delete; // copy assignment
        Allocator & operator=(Allocator && other) = delete; // move assignment
        
        ~Allocator();
        /*
        * Returns a Chunk* that points to the first free chunk.
        */
        chunk* getFreeHead();
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
        * Prints the current memory pool's statistics and the chunk's connections, postions, and size 
        * in order of occurence within the memory pool to the console.
        */
        void printChunks();
        /*
        * Allocates a chunk of {size} bytes and returns a void* to the chunk.
        */
        void* malloc(std::size_t size);
        /*
        * Finds the free block that occurs before or after an index
        * Prefers after over before
        * free blocks are not necessarily adjacent
        */
        chunk* findNearFree(size_t startingIndex);
        /*
        * Insert a new free chunk into the free list
        */
        void insertNewFree(chunk* newFree, chunk* nearFree);
        /*
        * Find the length of an occupied block by iterating through the memoryPool.
        * Begins at the starting index of the block (where * occurs/occured)
        * returns the total reserved space (including the initial reserve char)
        * ends when either running into another * (denoting the start of a different occupied space)
        * or when it reaches the starting index of nextFree
        * if nextFree is nullptr it assumes that no free chunk occurs after starting index
        */
        std::size_t findOccLength(std::size_t startingIndex, chunk* nextFree);
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
