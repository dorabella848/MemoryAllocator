#include <cstdlib>
struct Chunk {
    int startIndex;
    void* startLoc;
    size_t chunkSize;
    bool Free;
    Chunk* next;
    

    Chunk(int startingIndex, int chunkSize, bool Free) : startIndex(startingIndex), chunkSize(chunkSize), Free(Free), startLoc(nullptr), next(nullptr) {};
};