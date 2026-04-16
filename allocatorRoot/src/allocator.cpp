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
    cout << "\nTotal Memory: " << memorySize << "\n";

    if(freeHead->startIndex != 0){
        cout << "occHead: " << &memoryPool[0] << "\n";
    }
    else{
        cout << "occHead: nullptr\n";
    }
    if(freeHead != nullptr){
        cout << "freeHead: " << freeHead->startLoc << "\n\n";
    }
    else{
        cout << "freeHead: nullptr\n\n";
    }

    // Account for the occurence that the first chunk may be occupied
    if(freeHead->startIndex != 0){
        printf("Ptr: %-14p\n", &memoryPool[0]);
        printf("{ Free:  %-8s", "Occupied");
        printf("| Size: %-*zu", (int)round(log10(memorySize)), freeHead->chunkSize);
        printf("| startIndex: %-*zu", (int)round(log10(memorySize))-1, 0);
        cout << "}\n\n";
    }

    // Alternate between occupied and free chunks since there can be no free chunks adjacent to one another
    chunk* currentChunk = getFreeHead();
    while(currentChunk != nullptr){
        // Print the current free chunk's statistics
        printf("Ptr: %-14p\n", currentChunk->startLoc);
        printf("{ Free:  %-8s", "Free");
        printf("| Size: %-*zu", (int)round(log10(memorySize)), currentChunk->chunkSize);
        printf("| startIndex: %-*zu", (int)round(log10(memorySize)), currentChunk->startIndex);
        printf("| next: %-14p", (currentChunk->next != nullptr) ? currentChunk->next->startLoc : nullptr);
        printf("| prev: %-14p", (currentChunk->prev != nullptr) ? currentChunk->prev->startLoc : nullptr);
        cout << "}\n\n";

        // Print the occupied chunk's "statistics"

        size_t nextOccStart = currentChunk->startIndex + currentChunk->startIndex;
        bool nextOccExists = nextOccStart < memorySize;
        if(nextOccExists){
            printf("Ptr: %-14p\n", &memoryPool[nextOccStart]);
            printf("{ Free:  %-8s", "Occupied");
            if(currentChunk->next != nullptr){
                printf("| Size: %-*zu", (int)round(log10(memorySize)), currentChunk->next->startIndex - nextOccStart);
            }
            else{
                printf("| Size: %-*zu", (int)round(log10(memorySize)), memorySize - nextOccStart);
            }
            printf("| startIndex: %-*zu", (int)round(log10(memorySize)), nextOccStart);
            cout << "}\n\n";
        }
        currentChunk = currentChunk->next;
        

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

    if(size <= 0){
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
    if(nextFree == nullptr){
        return memorySize - startingIndex + 1;
    }
    while((memoryPool[startingIndex + size] != '*') && (nextFree->startIndex != startingIndex + size)){
        size += 1;
    }
    // Return the counted indexes plus the starting occupied character
    return size;
}

chunk* Allocator::findNearFree(size_t startingIndex){
    chunk* currentFree = getFreeHead();

    // Find the free chunk that occurs just before the occupied chunk
    while(currentFree != nullptr){
        if(currentFree->startIndex > startingIndex){
            break;
        }
        else if(currentFree->next == nullptr){
            break;
        }
        currentFree = currentFree->next;
    }
    return currentFree;
}

void Allocator::free(void* ptr){
    size_t occOffset = (uint8_t*)ptr - memoryPool - 1;
    // Check if ptr is an occupied block
    if(memoryPool[occOffset] == '*'){
        memoryPool[occOffset] = 0;
    }
    else{
        return;
    }
    chunk* currentFree = findNearFree(occOffset);

    // Create new Free chunk
    chunk* newFree = new chunk(occOffset, findOccLength(occOffset));
    (*newFree).startLoc = &memoryPool[newFree->startIndex];
    // Handle if there is no free head when freeing
    if(freeHead == nullptr){
        freeHead = newFree;
        return;
    }
    if(currentFree->startIndex > occOffset){
        if(currentFree->prev != nullptr){
            newFree->prev = currentFree->prev;
            currentFree->prev->next = newFree;
        }
        newFree->next = currentFree;
        currentFree->prev = newFree;
    }
    else{
        if(currentFree->next != nullptr){
            newFree->next = currentFree->next;
            currentFree->next->prev = newFree;
        }
        newFree->prev = currentFree;
        currentFree->next = newFree;
    }

    this->merge(newFree);
}

void* Allocator::calloc(size_t number, size_t size){
    void* arr = (Allocator::malloc(number*size));
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

    chunk* target = getTrueHead();
    chunk* prevFree = nullptr;
    while(target->startLoc != ptr){
        target = target->AbsNext;
        if(target->Free){
            prevFree = target;
        }
    }
    
    // Check if its even possible to perform the new reallocation
    if( (size > target->chunkSize) && (freeMemory < size - target->chunkSize) ){
        return target->startLoc;
    }
    if(target->chunkSize == size){
        return target->startLoc;
    }
    else if (target->chunkSize > size){
        chunk* newFreechunk = splitchunk(target, target->chunkSize - size);
        if(prevFree == nullptr){
            this->insert(newFreechunk, this->findOppReference(newFreechunk));
        }
        else{
            this->insert(newFreechunk, prevFree);
        }
        this->merge(newFreechunk);
        return target->startLoc;
    }
    else {
        // If the next chunk is free it might be possible to reallocate in place
        if(target->AbsNext != nullptr && target->AbsNext->Free){
            // The chunk that is in front of target
            chunk* targetAfter = target->AbsNext;
            updateSize(target, size);
            updateSize(targetAfter, size - target->chunkSize);
            return target->startLoc;
        }
        else{
            std::size_t targetSize = target->chunkSize;
            // ptr is target->starloc
            this->free(ptr);
            void* newBlock = this->malloc(size);
            if (newBlock == nullptr){
                return nullptr;
            }
            memmove(newBlock, ptr, targetSize);
            return newBlock;    
        }  
    }
}



