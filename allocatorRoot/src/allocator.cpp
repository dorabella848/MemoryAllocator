#include "memoryAllocator/stl.hpp"
#include "chunk.hpp"
#include <cstdio>
#include <cmath> 
#include <cstring>
#include <iostream>
using namespace std;

// Ctor
Allocator::Allocator(size_t numBytes){
    memorySize = numBytes;
    memoryPool = new uint8_t[memorySize];
    // Initialize all bytes
    for (size_t i = 0; i < memorySize; i++){
        memoryPool[i] = ' ';
    }
    freeHead = new chunk(0, memorySize);
    (*freeHead).startLoc = &memoryPool[0];
};
// Dtor
Allocator::~Allocator(){
    delete[] memoryPool;
    // Delete all free and occupied chunks
    chunk* currentChunk = getFreeHead();

    while(currentChunk != nullptr){
        if(currentChunk->next != nullptr){
            currentChunk = currentChunk->next;
            delete currentChunk->prev;
        }
        else{
            delete currentChunk;
            break;
        }
    }
}

chunk* Allocator::getFreeHead(){
    return freeHead;
}
size_t Allocator::getMemoryTotal(){
    return memorySize;
}
void* Allocator::getMemAddress(size_t index){
    return &memoryPool[index];
}
void Allocator::updateSize(chunk* target, size_t newSize)
{
    target->chunkSize = newSize;
    if(newSize == 0){
        if(target == freeHead){
            freeHead = target->next;
        }
        target->orphan();
    }
}
void Allocator::printChunks(){
    size_t curIndex = 0;
    chunk* currentChunk = getFreeHead();
    // True = Free 
    // False = Occupied
    bool curType = false;
    // Account for the occurence that the first chunk may be free
    if((freeHead != nullptr) && (freeHead->startIndex == 0)){
        printf("Ptr: %-14p\n", &memoryPool[0]);
        printf("{ State:  %-8s", "Free");
        printf("| Size: %-*zu", (int)round(log10(memorySize)), freeHead->chunkSize);
        printf("| startIndex: %-*d", (int)round(log10(memorySize))-1, 0);
        cout << "}\n\n";
        // Begin searrch after free chunk
        curIndex = freeHead->chunkSize;
        currentChunk = currentChunk->next;
    }

    // Alternate between occupied and free chunks since there can be no free chunks adjacent to one another
    while(curIndex < memorySize-1){
        // Print the current free chunk's statistics
        if(curType){
            printf("Ptr: %-14p\n", currentChunk->startLoc);
            printf("{ State:  %-8s", "Free");
            printf("| Size: %-*zu", (int)round(log10(memorySize)), currentChunk->chunkSize);
            printf("| startIndex: %-*zu", (int)round(log10(memorySize)), currentChunk->startIndex);
            printf("| next: %-14p", (currentChunk->next != nullptr) ? currentChunk->next->startLoc : nullptr);
            printf("| prev: %-14p", (currentChunk->prev != nullptr) ? currentChunk->prev->startLoc : nullptr);
            cout << "}\n\n";
            curType = false;
            curIndex += currentChunk->chunkSize;
            currentChunk = currentChunk->next;
        }
        else{
            // Print the occupied chunk's "statistics"

            // The maximum number of bytes to check for occupied size before switching to free
            size_t occSize = findOccLength(curIndex, currentChunk);
            printf("Ptr: %-14p\n", &memoryPool[curIndex]);
            printf("{ State:  %-8s", "Occupied");
            printf("| Size: %-*zu", (int)round(log10(memorySize)), occSize);
            printf("| startIndex: %-*zu", (int)round(log10(memorySize)), curIndex);
            cout << "}\n\n";
            curIndex += occSize;
            if(curIndex >= getMemoryTotal()){
                return;
            }
            else if(currentChunk != nullptr && curIndex == currentChunk->startIndex){
                curType = true;
            }
        }
    }
    cout << "ENDOFMEMORY\n";
}

chunk* Allocator::merge(chunk *newFree){
   // Check in front
    if( (newFree->next != nullptr) && (newFree->startIndex + newFree->chunkSize == newFree->next->startIndex) ){
        chunk* nextFree = newFree->next;
        updateSize(newFree, newFree->chunkSize + nextFree->chunkSize);
        updateSize(nextFree, 0);
        delete nextFree;
        }
    // Check behind
    if(newFree->prev != nullptr && (newFree->startIndex == newFree->prev->startIndex + newFree->prev->chunkSize)){
        chunk* prevFree = newFree->prev;
        updateSize(prevFree, prevFree->chunkSize + newFree->chunkSize);
        updateSize(newFree, 0);
        delete newFree;
        
        return prevFree;
    }
    return newFree;
}

void* Allocator::malloc(size_t size){

    if(size == 0){
        return nullptr;
    }

    chunk* freeCurrent = freeHead;
    while(freeCurrent != nullptr){
        // 1 byte must be reserved to denote the start of an occupied chunk
        if(freeCurrent->chunkSize >= size + 1){
            break;
        }
        freeCurrent = freeCurrent->next;
    }
    if(freeCurrent == nullptr){
        return nullptr;
    }
    updateSize(freeCurrent, freeCurrent->chunkSize - (size+1));
    memoryPool[freeCurrent->startIndex + freeCurrent->chunkSize] = '*';
    void* newOccupied = &memoryPool[freeCurrent->startIndex + freeCurrent->chunkSize + 1];
    if(freeCurrent->chunkSize == 0){
        delete freeCurrent;
    }
    return newOccupied;

}

size_t Allocator::findOccLength(size_t startingIndex, chunk* nextFree=nullptr){
    size_t size = 1;
    size_t upperLimit = 0;
    if(nextFree == nullptr){
        upperLimit = memorySize;
    }
    else{
        upperLimit = nextFree->startIndex;
    }
    while((startingIndex + size < upperLimit) && (memoryPool[startingIndex + size] != '*')){
        size += 1;
    }
    // Return the counted indexes plus the starting occupied character
    return size;
}

chunk* Allocator::findNearFree(size_t startingIndex){
    chunk* currentFree = getFreeHead();

    // Find the free chunk that occurs just before the occupied chunk
    while(currentFree != nullptr){
        if((currentFree->startIndex > startingIndex) || (currentFree->next == nullptr)){
            break;
        }
        currentFree = currentFree->next;
    }
    return currentFree;
}

void Allocator::insertNewFree(chunk* newFree, chunk* nearFree){
    if(nearFree->startIndex > newFree->startIndex){
        if(nearFree->prev != nullptr){
            newFree->prev = nearFree->prev;
            nearFree->prev->next = newFree;
        }
        newFree->next = nearFree;
        nearFree->prev = newFree;
    }
    else{
        if(nearFree->next != nullptr){
            newFree->next = nearFree->next;
            nearFree->next->prev = newFree;
        }
        newFree->prev = nearFree;
        nearFree->next = newFree;
    }
}

void Allocator::free(void* ptr){
    if(ptr == nullptr){
        return;
    }
    size_t occOffset = (uint8_t*)ptr - memoryPool - 1;
    // Check if ptr is an occupied block
    if(memoryPool[occOffset] == '*'){
        memoryPool[occOffset] = 0;
    }
    else{
        return;
    }

    // Create new Free chunk
    chunk* newFree = new chunk(occOffset, findOccLength(occOffset));
    (*newFree).startLoc = &memoryPool[newFree->startIndex];
    // Handle if there is no free head when freeing
    if(freeHead == nullptr || newFree->startIndex < freeHead->startIndex){
        freeHead = newFree;
        return;
    }
    insertNewFree(newFree, findNearFree(occOffset));
    this->merge(newFree);
}

void* Allocator::calloc(size_t number, size_t size){
    void* arr = (this->malloc(number*size));
    memset(arr, 0, number*size);
    return arr;
}

void* Allocator::realloc(void* ptr, size_t size){
    if(size == 0){
        free(ptr);
        return nullptr;
    }
    if(ptr == nullptr){
        return nullptr;
    }
    size_t occOffset = (uint8_t*)ptr - memoryPool - 1;
    size_t occSize = findOccLength(occOffset);
    // Check if ptr is an occupied block
    if(!(memoryPool[occOffset] == '*')){
        return nullptr;
    }
    if(occSize == size){
        return ptr;
    }
    else if(occSize > size){
        chunk* newFreechunk = new chunk((occOffset + (occSize-size)), occSize-size);
        if(getFreeHead() == nullptr || newFreechunk->startIndex < getFreeHead()->startIndex){
            freeHead = newFreechunk;
        }
        insertNewFree(newFreechunk, findNearFree(occOffset));
        merge(newFreechunk);
        return ptr;
    }
    else {
        // If the next chunk is free it might be possible to reallocate in place
        chunk* nearFreeChunk = findNearFree(occOffset);
        bool occursAfter = false;
        if(nearFreeChunk != nullptr){
            occursAfter = nearFreeChunk->startIndex == (occOffset + occSize);
        }

        if(occursAfter && (nearFreeChunk->chunkSize + occSize > size)){
            updateSize(nearFreeChunk, nearFreeChunk->chunkSize - (size-occSize));
            if(nearFreeChunk->chunkSize == 0){
                delete nearFreeChunk;
            }
            return ptr;
        }
        else{
            // An improvement can be made here to check if ptr can be free to make enough space by combining
            // nearby free chunks 
            void* newBlock = this->malloc(size);
            if(newBlock == nullptr){
                return nullptr;
            }
            this->free(ptr);
            memmove(newBlock, ptr, occSize-1);
            return newBlock;    
        }  
    }
}



