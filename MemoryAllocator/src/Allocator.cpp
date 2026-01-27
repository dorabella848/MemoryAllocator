#include "STL_Allocator/STL_Allocator.hpp"
#include "STL_Allocator/Chunk.hpp"
#include <cstdio>
#include <cmath> 
#include <cstring>
#include <iostream>
#include <stdexcept> // logic_error
using namespace std;

// Ctor
Allocator::Allocator(size_t numBytes){
    memorySize = numBytes;
    freeMemory = numBytes;
    memoryPool = new uint8_t[memorySize];
    freeHead = new Chunk(0, memorySize, true);
    (*freeHead).startLoc = &memoryPool[0];
};
// Dtor
Allocator::~Allocator(){
    delete[] memoryPool;
    // Delete all free and occupied chunks
    Chunk* currentChunk = nullptr;
    if(occHead == nullptr){
        currentChunk = freeHead;
    }
    else if(freeHead == nullptr){
        currentChunk = occHead;

    }
    else if(occHead->startIndex == 0){
        currentChunk = occHead;
    }
    else{
        currentChunk = freeHead;
    }

    Chunk* occCurrent = occHead;
    while(currentChunk != nullptr){
        if(currentChunk->AbsNext != nullptr){
            currentChunk = currentChunk->AbsNext;
            delete currentChunk->AbsPrev;
        }
        else{
            delete currentChunk;
            break;
        }
    }
}


Chunk* Allocator::getFreeHead(){
    return freeHead;
}
Chunk* Allocator::getOccHead(){
    return occHead;
}
Chunk* Allocator::getTrueHead(){
    Chunk* trueHead = nullptr;
    if(occHead == nullptr){
        trueHead = freeHead;
    }
    else if(freeHead == nullptr){
        trueHead = occHead;

    }
    else if(occHead->startIndex == 0){
        trueHead = occHead;
    }
    else{
        trueHead = freeHead;
    }
    return trueHead;
}
size_t Allocator::getFreeMemory(){
    return freeMemory;
}
size_t Allocator::getMemoryTotal(){
    return memorySize;
}
void* Allocator::getMemAddress(size_t index){
    return &memoryPool[index];
}
void Allocator::printChunks(){
    cout << "\nTotal Memory: " << memorySize << endl;
    switch(occHead != nullptr){
        case true: cout << "occHead: " << occHead->startLoc << endl; break;
        default: cout << "occHead: " << 0 << endl; break;
    }
    switch(freeHead != nullptr){
        case true: cout << "freeHead: " << freeHead->startLoc << endl << endl; break;
        default: cout << "freeHead: " << 0 << endl << endl; break;
    }

    Chunk* currentChunk = getTrueHead();

    while(currentChunk != nullptr){

        // absnext , absprev, Next, and Prev are both Chunk* so they are converted to normal so we can compare to Ptr
        printf("Ptr: %-14p\n", currentChunk->startLoc);
        printf("{ Free:  %-5s", currentChunk->Free ? "true" : "false");
        printf("| AbsNext: %-14p", (currentChunk->AbsNext != nullptr) ? currentChunk->AbsNext->startLoc : nullptr);
        printf("| Size: %-*zu", (int)round(log10(memorySize)), currentChunk->chunkSize);
        printf("| startIndex: %-*d", (int)round(log10(memorySize))-1, currentChunk->startIndex);
        printf("| AbsPrev: %-14p", (currentChunk->AbsPrev != nullptr) ? currentChunk->AbsPrev->startLoc : nullptr);
        printf("| next: %-14p", (currentChunk->next != nullptr) ? currentChunk->next->startLoc : nullptr);
        printf("| prev: %-14p", (currentChunk->prev != nullptr) ? currentChunk->prev->startLoc : nullptr);
        std::cout << "}\n" << endl;
        currentChunk = currentChunk->AbsNext;
        

    }
    cout << "ENDOFMEMORY" << endl;
}

Chunk* Allocator::merge(Chunk *newFree){
   // Check in front
    if(newFree->AbsNext != nullptr && newFree->AbsNext->Free){
        Chunk* nextFree = newFree->AbsNext;
        //Remove nextFree from freeList
        newFree->next = nextFree->next;
        if(nextFree->next != nullptr){
            nextFree->next->prev = newFree;
            }
        if(nextFree == freeHead){
            freeHead = nextFree->next;
            }
        //correct abslist
        newFree->AbsNext = nextFree->AbsNext;
        if(nextFree->AbsNext != nullptr){
            nextFree->AbsNext->AbsPrev = newFree;
            }
        newFree->chunkSize += nextFree->chunkSize;
        delete nextFree;
        }
    // Check behind
    if(newFree->AbsPrev != nullptr && newFree->AbsPrev->Free){
        Chunk* prevFree = newFree->AbsPrev;
        //Remove newFree from freeList
        prevFree->next = newFree->next;
        if(newFree->next != nullptr){
            newFree->next->prev = prevFree;
        }
        //correct abslist
        prevFree->AbsNext = newFree->AbsNext;
        if(newFree->AbsNext != nullptr){
            newFree->AbsNext->AbsPrev = prevFree;
        }
        prevFree->chunkSize += newFree->chunkSize;

        delete newFree;
        return prevFree;
    }
    return newFree;
       
}

void Allocator::insert_referece(Chunk *toInsert, Chunk *refChunk){
    // Updates the next, prev, and free vars of a chunk
    // ref chunk is the closest chunk behind or after toInsert of the opposite type

    if(toInsert == nullptr){
        return;
    }
    // Ensure surrounding ptrs point to toInsert
    if(toInsert->AbsPrev != nullptr){
        toInsert->AbsPrev->AbsNext = toInsert;
    }
    if(toInsert->AbsNext != nullptr){
        toInsert->AbsNext->AbsPrev = toInsert;
    }

    if(refChunk != nullptr && refChunk->Free != toInsert->Free){
        bool refBeforeToInsert = refChunk->startIndex < toInsert->startIndex;

        // if the reference chunk occurs before toInsert
        bool refHasNext = !(refChunk->next == nullptr);
        bool nextAfterToInsert = (refHasNext) && (refChunk->next->startIndex > toInsert->startIndex);
        if( refBeforeToInsert && (!refHasNext || nextAfterToInsert) ){
            // In case newFree was a head of some list
            if(toInsert == freeHead){
                freeHead = toInsert->next;
            }
            else if(toInsert == occHead){
                occHead = toInsert->next;
            }

            toInsert->orphan();
            toInsert->prev = refChunk;
            if(refChunk->next != nullptr){
                toInsert->next = refChunk->next;
                refChunk->next->prev = toInsert;
            }
            refChunk->next = toInsert;
            toInsert->Free = !toInsert->Free;
            
        } 

        // if the reference chunk occurs after toInsert
        bool refHasPrev = !(refChunk->prev == nullptr);
        bool prevBeforeToInsert = (refHasPrev) && (refChunk->prev->startIndex < toInsert->startIndex);
        if( !refBeforeToInsert && (!refHasPrev || prevBeforeToInsert) ){
            // In case newFree was a head of some list
            if(toInsert == freeHead){
                freeHead = toInsert->next;
            }
            else if(toInsert == occHead){
                occHead = toInsert->next;
            }
            toInsert->orphan();
            toInsert->next = refChunk;
            if(refChunk->prev != nullptr){
                toInsert->prev = refChunk->prev;
                refChunk->prev->next = toInsert;
            }
            refChunk->prev = toInsert;
            toInsert->Free = !toInsert->Free;
        }
        // In case newFree is now a head of some list
        if( (toInsert->Free) && (toInsert->startIndex < freeHead->startIndex) ){
            freeHead = toInsert;
        }
        if( (!toInsert->Free) && (toInsert->startIndex < occHead->startIndex) ){
            occHead = toInsert;
        }
    }
}

void Allocator::insert(Chunk *toInsert){
    // Must have the aboslute positions (of toInsert) established before running

    // toInsert's free value should be the opposite of the list its trying to be inserted into 
    // ex: toInsert needs to be inserted into the free list so it is assigned the occupied free state.

    if(toInsert == nullptr){
        return;
    }

    // In case newFree was a head of some list
    if(toInsert == freeHead){
        freeHead = toInsert->next;
    }
    else if(toInsert == occHead){
        occHead = toInsert->next;
    }

    // Ensure surrounding ptrs point to toInsert
    if(toInsert->AbsPrev != nullptr){
        toInsert->AbsPrev->AbsNext = toInsert;
    }
    if(toInsert->AbsNext != nullptr){
        toInsert->AbsNext->AbsPrev = toInsert;
    }

    // In case there is no header for the list toInsert is being inserted into
    if( (!toInsert->Free) && (freeHead == nullptr) ){
        freeHead = toInsert;
        // Just in case
        toInsert->orphan();
        // Update free state
        toInsert->Free = !toInsert->Free;
        return;
    }
    else if( (toInsert->Free) && (occHead == nullptr) ){
        occHead = toInsert;
        // Just in case
        toInsert->orphan();
        // Update free state
        toInsert->Free = !toInsert->Free;
        return;
    }

    // Insert the chunk into the opposite type list 

    Chunk* refChunk = nullptr;
    // Find a reference chunk of the opposite type adjacently
    // Check behind
    if(toInsert->AbsPrev != nullptr){
        if(toInsert->AbsPrev->Free != toInsert->Free){
            refChunk = toInsert->AbsPrev;
        }
        else if( (toInsert->AbsPrev->AbsPrev != nullptr) && (toInsert->AbsPrev->AbsPrev->Free != toInsert->Free) ){
            refChunk = toInsert->AbsPrev->AbsPrev;
        }
    }
    // Check front
    if(toInsert->AbsNext != nullptr){
        if(toInsert->AbsNext->Free != toInsert->Free){
            refChunk = toInsert->AbsNext;
        }
        else if( (toInsert->AbsNext->AbsNext != nullptr) && (toInsert->AbsNext->AbsNext->Free != toInsert->Free) ){
            refChunk = toInsert->AbsNext->AbsNext;
        }
    }
    // Test iterating with AbsNext/AbsPrev rather than next (might be faster due to the compiler)
    // This will only occur if youre freeing an occupied chunk
    if(refChunk == nullptr){
        // If the free head occurs after newFree we know it's an adjacent free chunk in the free list
        refChunk = freeHead;
        if(freeHead->startIndex < toInsert->startIndex){ 
            while( (refChunk->next != nullptr) && (refChunk->next->startIndex < toInsert->startIndex) ){
                refChunk = refChunk->next;
            }
        }
    }
    if(refChunk == nullptr){
        cout << "Was unable to insert chunk {" << toInsert << "}" << endl;
        return;
    }
    // Insert toInsert via refChunk
    insert_referece(toInsert, refChunk);
}

void* Allocator::malloc(size_t size){
    // Search through the free list and determine if there is a large enough free block to house the new occupied chunk
    // if there is enough free storage in the memory pool but no properly sized free block call defragment() (to be implemented)
    // if not
    // return nullptr
    // otherwise
    // Create a new occupied chunk with a chunksize of Size
    // and update freeCurrent's variable to simulate the creation of the occupied chunk
    // if there is no occHead then newChunk is the new occHead 
    // otherwise
    // Update the chunk pointers of the chunks adjacent to freeCurrent to insert newChunk into the occupied list
    // Update freeCurrent's absprev to point to newChunk
    // Update newChunk's pointers
    // check if freeCurrent has no more available space, if so, remove it from the free list (check if it is the freeHead)
    // return the address of the newChunk's start location (points to start of its position in the memoryPool)
    // we return the adress of the startlocation because defragment() could potentially deassociate previously created
    // pointers

    if(size <= 0){
        return nullptr;
    }

    // No point in checking anything if there isnt theoretically enough space
    if(freeMemory < size){
        cout << "Malloc failed for pointer of size (" << size  <<"): lack of free memory" << endl;
        return nullptr;
    }

    Chunk* freeCurrent = freeHead;
    while(freeCurrent != nullptr){
        if(freeCurrent->chunkSize >= size){
            break;
        }
        freeCurrent = freeCurrent->next;
    }
    if(freeCurrent == nullptr){
        return nullptr;
    }

    Chunk* newChunk = new Chunk(freeCurrent->startIndex, size, true);
    newChunk->startLoc = &memoryPool[newChunk->startIndex];
    freeCurrent->startIndex += newChunk->chunkSize;
    freeCurrent->startLoc = &memoryPool[freeCurrent->startIndex];
    freeCurrent->chunkSize -= newChunk->chunkSize;
    
    newChunk->AbsNext = freeCurrent;
    newChunk->AbsPrev = freeCurrent->AbsPrev;
    
    if (freeCurrent->chunkSize == 0){
        newChunk->AbsNext = freeCurrent->AbsNext;
        if(freeCurrent->prev != nullptr){
            freeCurrent->prev->next = freeCurrent->next;
        }
        if(freeCurrent == freeHead){
            freeHead = freeCurrent->next;
        }
        if(freeCurrent->next != nullptr){
            freeCurrent->next->prev = freeCurrent->prev;
        }
        delete freeCurrent;
    }

    this->insert(newChunk);
    // Update freeMemory tracker
    freeMemory -= size;

    return (newChunk->startLoc);

}

void Allocator::free(void* ptr){
    // Search through occupied memory list  and see if ptr exists
    // Remove the chunk associated with ptr from the occupied list (account for if it is the occHead)
    // Check the freestaet of adjacent chunks and merge them to the left-most free chunk
    // return if an adjacent free chunk is found
    // otherwise
    // if the freeHead has a larger start index than newFree then we know that newFree is the new freeHead
    // and we can just reference our pointer updates off of the old freeHead
    // otherwise
    // we need to search through the free list until either the next free chunk is a nullptr
    // or it has a larger start index than newChunk
    // We can then use this found free chunk to update next and prev
    Chunk* target = getTrueHead();
    Chunk* prevFree = nullptr;
    // keep track of previous occupied chunk so there is no need to maintain a prev variable
    //Chunk* prevOccChunk = nullptr;
    
    while(target != nullptr){
        if(target->startLoc == ptr){
            break;
        }
        if(target->Free){
            prevFree = target;
        }
        target = target->AbsNext;
    }
    if(target == nullptr){
        return;
    }
    // Since we know the ptr exists we can update freeMemory here
    freeMemory += target->chunkSize;
    if(prevFree == nullptr){
        if(freeHead == nullptr){
            this->insert(target);
        }
        else{
            this->insert_referece(target, freeHead);
        }
    }
    else{
        this->insert_referece(target, prevFree);
    }
    this->merge(target);
}

void* Allocator::calloc(size_t number, size_t size){
    void* arr = (Allocator::malloc(number*size));
    memset(arr, 0, number*size);
    return arr;
}

void* Allocator::realloc(void* ptr, size_t size){
    if (size == 0){
        return nullptr;
    }
    if (ptr == nullptr){
        return nullptr;
    }

    Chunk* target = getTrueHead();
    Chunk* prevFree = nullptr;
    while (target->startLoc != ptr){
        target = target->AbsNext;
        if(target->Free){
            prevFree = target;
        }
        if (target == ptr){
            break;
        }
    }
    if(target == nullptr){
        return nullptr;
    }
    
    // Check if its even possible to perform the new reallocation
    if( (size > target->chunkSize) && (freeMemory < size - target->chunkSize) ){
        cout << "Reallocation failed for " << ptr << ": lack of free memory" << endl;
        return target->startLoc;
    }
    if(target->chunkSize == size){
        return target->startLoc;
    }
    else if (target->chunkSize > size){
        // We know the pointers exist so we can update freeMemory
        freeMemory -= (size - target->chunkSize);
        Chunk* newFreeChunk = new Chunk(target->startIndex + size, target->chunkSize-size, false);
        newFreeChunk->AbsNext = target->AbsNext;
        newFreeChunk->AbsPrev = target;
        newFreeChunk->startLoc = &memoryPool[newFreeChunk->startIndex];

        if(prevFree == nullptr){
            if(freeHead == nullptr){
                this->insert(newFreeChunk);
            }
            else{
                this->insert_referece(newFreeChunk, freeHead);
            }
        }
        else{
            this->insert_referece(newFreeChunk, prevFree);
        }

        this->merge(newFreeChunk);
        target->chunkSize = size;
        return target->startLoc;
    }
    else {
        // Added min function since we dont want to copy more than necessary
        int dataSize = min(target->chunkSize, size);
        uint8_t* savedData = new uint8_t[dataSize]; // Have to use dynamic allocaiton since min() is processed at runtime
        memcpy(savedData, ptr, dataSize);
        Allocator::free(target->startLoc);
        void* newBlock = malloc(size);
        if (newBlock == nullptr){
            return nullptr;
        }
        memcpy(newBlock, savedData, dataSize);
        delete[] savedData;
        return newBlock;    
    }
}



