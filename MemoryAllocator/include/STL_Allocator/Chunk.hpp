#include <cstddef>
#include <iostream>
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
    /*
    * Removes next and prev connections to a chunk
    * if the chunk has a chunksize of 0, also remove abs ptrs
    */
    void orphan()
    {
        if(this->prev != nullptr){
            this->prev->next = this->next;
        }
        if(this->next != nullptr){
            this->next->prev = this->prev;
        }
        this->next = nullptr;
        this->prev = nullptr;

        if(this->chunkSize == 0){
            if(this->AbsPrev != nullptr){
                this->AbsPrev->AbsNext = this->AbsNext;
            }
            if(this->AbsNext != nullptr){
                this->AbsNext->AbsPrev = this->AbsPrev;
            }
            this->AbsNext = nullptr;
            this->AbsPrev = nullptr;
        }
    }

    ~Chunk() {

    }
};