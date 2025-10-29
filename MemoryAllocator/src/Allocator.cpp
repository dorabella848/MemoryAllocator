#include "Allocator.hpp"
#include <cstdio>
#include <cmath> 
#include <cstring>
#include <iostream>
#include <stdexcept> // logic_error
#include <algorithm> // min()
using namespace std;

Chunk* Allocator::getFreeHead(){
    return freeHead;
}
Chunk* Allocator::getOccHead(){
    return occHead;
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
};

void** Allocator::malloc(size_t size)
{
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
        // If no single free chunk had enough space but we have enough total free memory then fragmentation
        // is the limitation
        // This could lead to an inf loop if freeMemory is not properly tracked
        if(freeMemory > size){
            defragment();
            return malloc(size);
        }
        else{
            return nullptr;
        }
    }

    Chunk* newChunk = new Chunk(freeCurrent->startIndex, size, false);
    newChunk->startLoc = &memoryPool[newChunk->startIndex];
    freeCurrent->startIndex += newChunk->chunkSize;
    freeCurrent->startLoc = &memoryPool[freeCurrent->startIndex];
    freeCurrent->chunkSize -= newChunk->chunkSize;
    
    if(occHead == nullptr){
        occHead = newChunk;
        newChunk->AbsNext = freeCurrent;
        newChunk->AbsPrev = freeCurrent->AbsPrev;
        freeCurrent->AbsPrev = newChunk;
    }
    else{
        if(freeCurrent->AbsPrev->next != nullptr){
            freeCurrent->AbsPrev->next->prev = newChunk;
        }
        // Insert newChunk into the occupied list by using freeCurrent as the reference
        freeCurrent->AbsPrev->next = newChunk;
        freeCurrent->AbsPrev->AbsNext = newChunk;
        newChunk->AbsPrev = freeCurrent->AbsPrev;
        newChunk->prev = freeCurrent->AbsPrev;
        freeCurrent->AbsPrev = newChunk;
        newChunk->AbsNext = freeCurrent;
        newChunk->next = freeCurrent->AbsNext;
    }

    if (freeCurrent->chunkSize == 0){
        newChunk->AbsNext = freeCurrent->AbsNext;
        if(freeCurrent->AbsNext != nullptr){
            freeCurrent->AbsNext->AbsPrev = newChunk;
        }
        if(freeCurrent->prev != nullptr){
            freeCurrent->prev->next = freeCurrent->next;
        }
        if(freeCurrent == freeHead){
            freeHead = freeCurrent->next;
            if(freeCurrent->next != nullptr){
                freeCurrent->next->prev = nullptr;
            }
        }
        else {
            if(freeCurrent->next != nullptr){
                freeCurrent->next->prev = freeCurrent->prev;
            }
        }
        delete freeCurrent;
    }
    // Update freeMemory tracker
    freeMemory -= size;

    return &(newChunk->startLoc);

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
    Chunk* newFree = occHead;
    // keep track of previous occupied chunk so there is no need to maintain a prev variable
    //Chunk* prevOccChunk = nullptr;
    
    while(newFree != nullptr){
        if(newFree->startLoc == ptr){
            break;
        }
        newFree = newFree->next;
    }
    if(newFree == nullptr){
        return;
    }
    // Since we know the ptr exists we can update freeMemory here
    freeMemory += newFree->chunkSize;

    if(newFree == occHead){
        if(newFree->next != nullptr){
            occHead = newFree->next;
        }
        else{
            occHead = nullptr;
        }
    }

    if(newFree->prev != nullptr){
        newFree->prev->next = newFree->next;
    }
    if(newFree->next != nullptr){
        newFree->next->prev = newFree->prev;
    }
    newFree->Free = true;
    // check if adjacent chunks are free and merge if they are
    if( (newFree->AbsNext != nullptr && newFree->AbsNext->Free) || (newFree->AbsPrev != nullptr && newFree->AbsPrev->Free) ){
        // Check in front
        if(newFree->AbsNext != nullptr && newFree->AbsNext->Free){
            Chunk* nextFree = newFree->AbsNext;
            newFree->prev = nextFree->prev;
            newFree->next = nextFree->next;
            if(nextFree->prev != nullptr){
                nextFree->prev->next = newFree;
            }
            if(nextFree->next != nullptr){
                nextFree->next->prev = newFree;
            }

            if(nextFree->AbsNext != nullptr){
                nextFree->AbsNext->AbsPrev = newFree;
            }
            newFree->AbsNext = nextFree->AbsNext;
            newFree->chunkSize += nextFree->chunkSize;
            if(nextFree == freeHead){
                freeHead = newFree;
            }
            delete nextFree;

        }
        // Check behind
        if(newFree->AbsPrev != nullptr && newFree->AbsPrev->Free){
            Chunk* prevFree = newFree->AbsPrev;
            newFree->AbsPrev->AbsNext = newFree->AbsNext; 
            if(newFree->AbsNext != nullptr){
                newFree->AbsNext->AbsPrev = newFree->AbsPrev;
            }
            prevFree->chunkSize += newFree->chunkSize;
            
            delete newFree;
        }
        return;
    }

    if(freeHead == nullptr){
        freeHead = newFree;
        return;
    }
    if(freeHead->startIndex > newFree->startIndex){
        newFree->prev = nullptr;
        newFree->next = freeHead;
        freeHead->prev = newFree;
        freeHead = newFree;
        return;
    }

    Chunk* prevFreeChunk = freeHead;
    while(prevFreeChunk->next != nullptr && prevFreeChunk->next->startIndex < newFree->startIndex){
        prevFreeChunk = prevFreeChunk->next;
    }

    newFree->next = prevFreeChunk->next;
    newFree->prev = prevFreeChunk;
    if(prevFreeChunk->next != nullptr){
        prevFreeChunk->next->prev = newFree;
    }
    prevFreeChunk->next = newFree;
}

void Allocator::defragment(){
    if(freeMemory <= 0){
        throw logic_error("Defragment was called when no Free memory exists");
    }
    // Go through every freeChunk that is not the last free chunk
    // and shift all the occupied chunk to the left (add the free chunk's size to the next free chunk)
    // repeat until all free chunks are moved to the end
    Chunk* occCurrent = occHead;
    Chunk* freeCurrent = freeHead;
    Chunk* prevFreeChunk = nullptr;
    // The total number of occupied bytes to be moved after each free chunk relocation
    int totalMove = 0;
    // The total number of free bytes moved to the final free chunk
    int totalFree = 0;
    while(freeCurrent->next != nullptr){
        
        void* occStartLoc = nullptr;
        // totalFree must be tracked because we delete free chunks as we pass over them
        totalFree += freeCurrent->chunkSize;

        while(occCurrent->startIndex < freeCurrent->startIndex){
            occCurrent = occCurrent->next;
        }

        while(occCurrent != nullptr && occCurrent->startIndex < freeCurrent->next->startIndex){
            // Get the total number of occupied chunks in between two free chunks
            // to figure out how many bytes to move over into the to be moved free chunk
            totalMove += occCurrent->chunkSize;
            if(occStartLoc == nullptr){
                occStartLoc = occCurrent->startLoc;
            }
            occCurrent->startIndex -= freeCurrent->chunkSize;
            occCurrent->startLoc = &memoryPool[occCurrent->startIndex];
            occCurrent = occCurrent->next;
        }
        // Move the occupied bytes over by the total number of free bytes we have moved to the end
        memmove(freeCurrent->startLoc, occStartLoc, totalMove);
        prevFreeChunk = freeCurrent;
        freeCurrent = freeCurrent->next;
        if(prevFreeChunk->AbsPrev != nullptr){
            prevFreeChunk->AbsPrev->AbsNext = prevFreeChunk->AbsNext;
        }
        prevFreeChunk->AbsNext->AbsPrev = prevFreeChunk->AbsPrev;
        delete prevFreeChunk;
    };

    // Update the final free chunk which is at the end of the memory pool (it's also the freeHead)
    
    freeCurrent->startIndex -= totalFree;
    freeCurrent->chunkSize += totalFree;
    freeCurrent->startLoc = &memoryPool[freeCurrent->startIndex];
    freeCurrent->prev = nullptr;
    freeHead = freeCurrent;
    
    if(totalFree == 0 || totalMove == 0){
        throw logic_error("Fatal error: Defragment was called but nothing was moved");
    }
    
};

void** Allocator::calloc(size_t number, size_t size){
    void** arr = (Allocator::malloc(number*size));
    memset(*arr, 0, number*size);
    return arr;
}

void** Allocator::realloc(void* ptr, size_t size){
    Chunk* target = occHead;
    if (size == 0){
        return nullptr;
    }
    if (ptr == nullptr){
        return nullptr;
    }
    while (target->startLoc != ptr){
        target = target->next;
        if (target == ptr){
            break;
        }
    }
    if(target == nullptr){
        return nullptr;
    }
    // Check if its even possible to perform the new reallocation
    if(freeMemory < size - target->chunkSize){
        cout << "Reallocation failed for " << ptr << ": lack of free memory" << endl;
        return &(target->startLoc);
    }
    // We know the pointers exist so we can update freeMemory
    freeMemory += (target->chunkSize - size);

    if (target->chunkSize >= size){
        // if there is a free chunk to the right of target there is no need to create a new free chunk
        if(target->AbsNext != nullptr && target->AbsNext->Free){
            target->AbsNext->chunkSize += target->chunkSize-size;
            target->AbsNext->startIndex -= target->chunkSize-size;
            target->AbsNext->startLoc = &memoryPool[target->AbsNext->startIndex];
            target->chunkSize = size;
            return &(target->startLoc);
        }

        Chunk* newFreeChunk = new Chunk(target->startIndex + target->chunkSize, target->chunkSize-size, true);
        target->chunkSize = size;
        newFreeChunk->startLoc = &memoryPool[newFreeChunk->startIndex];
        // insert newFreeChunk into the abs list
        newFreeChunk->AbsPrev = target;
        newFreeChunk->AbsNext = target->AbsNext;
        if(target->AbsNext != nullptr){
            target->AbsNext->AbsPrev = newFreeChunk;
        }
        if(target->AbsPrev != nullptr){
            target->AbsPrev->AbsNext = newFreeChunk;
        }
        target->AbsNext = newFreeChunk;
        // no need to update next and prev if there is no other free chunks
        if(freeHead == nullptr){
            freeHead = newFreeChunk;
            return &(target->startLoc);
        }
        if(freeHead->startIndex > newFreeChunk->startIndex){
            newFreeChunk->prev = nullptr;
            newFreeChunk->next = freeHead;
            freeHead->prev = newFreeChunk;
            freeHead = newFreeChunk;
            return &(target->startLoc);
        }
        Chunk* prevFreeChunk = freeHead;
        // Logic used from free() to find the closest free chunk
        while(prevFreeChunk->next != nullptr && prevFreeChunk->next->startIndex < target->startIndex){
            prevFreeChunk = prevFreeChunk->next;
        }

        newFreeChunk->next = prevFreeChunk->next;
        newFreeChunk->prev = prevFreeChunk->prev;
        if(prevFreeChunk->next != nullptr){
            prevFreeChunk->next->prev = newFreeChunk;
        }
        prevFreeChunk->next = newFreeChunk;

        return &(target->startLoc);
    }
    else {
        // Added min function since we dont want to copy more than necessary
        int dataSize = min(target->chunkSize, size);
        uint8_t* savedData = new uint8_t[dataSize]; // Have to use dynamic allocaiton since min() is processed at runtime
        memcpy(savedData, ptr, dataSize);
        Allocator::free(target->startLoc);
        void** newBlock = malloc(size);
        if (*newBlock == nullptr){
            return nullptr;
        }
        memcpy(*newBlock, savedData, dataSize);
        delete savedData;
        return newBlock;    
    }
}



