#include <cstddef>
struct Chunk {
    int startIndex;
    void* startLoc;
    std::size_t chunkSize;
    bool Free;
    // Next free/occupied chunk dependent on current free state
    Chunk* next;
    // Previous free/occupied chunk dependent on current free state
    Chunk* prev;
    // Absolute next chunk in the memory pool regardless of current free state
    Chunk* AbsNext;
    // Absolute previous chunk in the memory pool regardless of current free state
    Chunk* AbsPrev;
    

    Chunk(int startingIndex, int chunkSize, bool Free) : 
    startIndex(startingIndex), 
    chunkSize(chunkSize), 
    Free(Free), 
    startLoc(nullptr), 
    next(nullptr), 
    prev(nullptr),
    AbsNext(nullptr),
    AbsPrev(nullptr) {};
    ~Chunk() {
        
    }
};