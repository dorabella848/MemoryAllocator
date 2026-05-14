#include <cstddef>
struct chunk {
    void* startLoc;
    std::size_t startIndex;
    std::size_t chunkSize;
    chunk* next;
    chunk* prev;
    

    chunk(std::size_t startingIndex, std::size_t chunkSize) : 
    startIndex(startingIndex), 
    chunkSize(chunkSize), 
    startLoc(nullptr), 
    next(nullptr), 
    prev(nullptr) {};
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
    }

    ~chunk() {

    }
};