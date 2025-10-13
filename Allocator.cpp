#include "Allocator.hpp"
#include <cstdio>
#include <cmath> 
Chunk* Allocator::getFreeHead(){
    return freeHead;
};
Chunk* Allocator::getOccHead(){
    return occHead;
}
void* Allocator::getMemAddress(size_t index){
    return &memoryPool[index];
}
void Allocator::printChunks(){
    cout<< "\nTotal Memory: " << memorySize << endl;
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
        // Left-justify and pad with spaces to a width of 10
        printf("{Ptr: %-10p", currentChunk->startLoc);
        printf("| FreeState:  %-6s", currentChunk->Free ? "true" : "false");
        printf("| Size: %-*d", (int)round(log10(memorySize))+1, currentChunk->chunkSize);
        printf("| startIndex: %-*d", (int)round(log10(memorySize))+1, currentChunk->startIndex);
        printf("| AbsNext: %-10p", (currentChunk->AbsNext != nullptr) ? currentChunk->AbsNext->startLoc : nullptr);
        printf("| AbsPrev: %-10p", (currentChunk->AbsPrev != nullptr) ? currentChunk->AbsPrev->startLoc : nullptr);
        printf("| next: %-10p", (currentChunk->next != nullptr) ? currentChunk->next->startLoc : nullptr);
        printf("| prev: %-10p", (currentChunk->prev != nullptr) ? currentChunk->prev->startLoc : nullptr);
        std::cout << "}" << endl;
        currentChunk = currentChunk->AbsNext;
        

    }
    cout << "ENDOFMEMORY" << endl;
};

void *Allocator::malloc(size_t size)
{
    // Input sanitization
    if(size <= 0){
        return nullptr;
    }
    // Go to first free chunk (that is able to hold size)
    Chunk* freeCurrent = freeHead;
    
    while(freeCurrent != nullptr){
        if(freeCurrent->chunkSize >= size){
            break;
        }
        freeCurrent = freeCurrent->next;
    }
    if(freeCurrent == nullptr){
        // have some way to know total free mem and then auto perform defragment
        return nullptr;
    }
    // create new memory chunk 
    Chunk* newChunk = new Chunk(freeHead->startIndex, size, false);
    newChunk->startLoc = &memoryPool[newChunk->startIndex];
    // Update the free-chunk-you-are-taking-memory-from's size, startloc, and startindex
    freeCurrent->startIndex += newChunk->chunkSize;
    freeCurrent->startLoc = &memoryPool[freeCurrent->startIndex];
    freeCurrent->chunkSize -= newChunk->chunkSize;
    // if occHead is empty put new occ chunk at occ head and set occ absnext to the free block it was created from vice versa
    if(occHead == nullptr){
        occHead = newChunk;
        newChunk->AbsNext = freeCurrent;
        newChunk->AbsPrev = freeCurrent->AbsPrev;
        freeCurrent->AbsPrev = newChunk;
    }
    else{
        // set the free-chunk-you-are-recieving-memory-from's previous chunk's next to the new occ chunk 
        // (both abs and normal since we are assuming no two free memory blocks are right next to eachother)
        freeCurrent->AbsPrev->next = newChunk;
        freeCurrent->AbsPrev->AbsNext = newChunk;
        newChunk->AbsPrev = freeCurrent->AbsPrev;
        freeCurrent->AbsPrev = newChunk;
        // set new chunk's absnext to the free chunk you are taking memory from and its next to the freechunk's absnext
        newChunk->AbsNext = freeCurrent;
        newChunk->next = freeCurrent->AbsNext; // (might be a nullptr but not point in confirming before assign)
    }
    // Check if free current should be deleted
    if (freeCurrent->chunkSize == 0){
        if (freeCurrent == freeHead){
            newChunk->AbsNext = nullptr;
            freeHead = freeCurrent->next;
        } else {
            // if freeCurrent is not freeHead
            freeCurrent->prev->AbsNext = freeCurrent->AbsNext;
            freeCurrent->next->prev = freeCurrent->prev;
        }
        delete freeCurrent;
    }

    return &(newChunk->startLoc);

}
// works in tandem with free
void Allocator::deleteChunk(Chunk* toRemove){
    // The chunk to be deleted is assumed to either be free or has a free block (cant be both, the other is reserved to be freed) before it
    // has not had its freestate changed since after it was decided to be deleted

     // make delete function that takes a chunk, adds its chunksize to the previous chunk
    // and sets its prev's next to the chunk's next and vice versa for the chunk's next
    // also update the adjacent chunk's abs position pointers and then delete the inputted chunk

    // if toRemove is the free block and the previous is going to be freed
    toRemove->AbsPrev->chunkSize += toRemove->chunkSize;
    if(toRemove->Free){
        if(toRemove->prev != nullptr){
            toRemove->prev->next = toRemove->AbsPrev;
        }
        if(toRemove->next != nullptr){
            toRemove->AbsPrev->next = toRemove->next;
        }
        else{
            toRemove->AbsPrev->next = nullptr;
        }
    }
    else{
        // if toRemove is not the free block and absprev is free
        if(toRemove->prev != nullptr){
            toRemove->prev->next = toRemove->next;
        }
        if(toRemove->next != nullptr){
            toRemove->next->prev = toRemove->prev;
        }
        else{
            toRemove->prev->next = nullptr;
        }
    }
    if(toRemove->AbsNext != nullptr){
            toRemove->AbsNext->AbsPrev = toRemove->AbsPrev;
            toRemove->AbsPrev->AbsNext = toRemove->AbsNext;
        }
        else{
            toRemove->AbsPrev->AbsNext = nullptr;
    }
    // if toRemove is an occupied chunk then we need to check if its the occHead and change the occHead
    // if toRemove is free need to check if its the freehead and change it to the absprev
    // also need to check if absprev of toRemove is occHead
    // update absprev to be free just in case

    if(toRemove == occHead){
        if(toRemove->next != nullptr){
           occHead = toRemove->next;
        }
    }
    else{
        occHead = nullptr;
    }
    if(toRemove == freeHead){
        freeHead = toRemove->AbsPrev;
    }
    
    // just in case
    toRemove->AbsPrev->Free = true;
    delete toRemove;

}

void Allocator::free(void* ptr) {
    Chunk* newFree = occHead;
    while(newFree != nullptr){
        if(newFree->startLoc == ptr){
            break;
        }
        newFree = newFree->next;
    }
    if(newFree == nullptr){
        return;
    }
    if(newFree->AbsNext != nullptr && newFree->AbsNext->Free){
        deleteChunk(newFree->AbsNext);
        if(newFree->AbsPrev != nullptr && newFree->AbsPrev->Free){
            deleteChunk(newFree);
        }
        return;
    }
    if(newFree->AbsPrev != nullptr && newFree->AbsPrev->Free){
        deleteChunk(newFree);
        return;
    }
    // found new free from prev code
    newFree->Free = true;
    // in case free head is nullptr
    if(freeHead == nullptr){
        freeHead = newFree;
        newFree->next = nullptr;
        newFree->prev = nullptr;
        return;
    }
    Chunk* prevFreeChunk = newFree->AbsPrev;
    // In case there is a previous free chunk
    while(prevFreeChunk != nullptr){
        if(prevFreeChunk->Free){
            Chunk* nextFreeChunk = prevFreeChunk->next;
            prevFreeChunk->next = newFree;
            newFree->prev = prevFreeChunk;
            // update the free chunk state otherwise the new free block is the free head and its next is the prev free head
            if(nextFreeChunk != nullptr){
                nextFreeChunk->prev = newFree;
                newFree->next = nextFreeChunk;
                
            }
            else{
                newFree->next = nullptr;
            }
            return;
        }
        prevFreeChunk = prevFreeChunk->AbsPrev;
    }
    
    // the free head occurs after the current (confirmed by previous lines (wouldnt have gotten here otherwise))
    newFree->next = freeHead;
    freeHead->prev = newFree;
    freeHead = newFree;
};

void Allocator::defragment(){
    // Move memory around to make one free block

    // Make the last free block the tail end of the big free block
    // Move memory blocks from left to right to the front of the free block tail
    Chunk* occCurrent = occHead;
    Chunk* freeCurrent = freeHead;
    Chunk* prevFreeChunk = nullptr;
    // to remove a free chunk move all subsequent chunks up to the next free chunk
    // Get starting location of the first chunk
    // iterate through list and count chunk sizes
    // use memmove to move the memory to the start loc of the free chunk by chunk total size from the location
    // of the next chunk
    // Update the affected chunks memory location and memory pool index
    // repeat for each memory block
    // if the current free block is the last in the free list then stop
    int totalMove = 0;
    int totalFree = 0;
    while(freeCurrent->next != nullptr){
        
        void* occStartLoc = nullptr;
        totalMove = 0;
        totalFree += freeCurrent->chunkSize;

        // Catch occCurrent up to current freeCurrent index
        while(occCurrent->startIndex < freeCurrent->startIndex){
            occCurrent = occCurrent->next;
        }

        while(occCurrent != nullptr && occCurrent->startIndex < freeCurrent->next->startIndex){
            totalMove += occCurrent->chunkSize;
            // Gets first occupied chunk start location
            if(occStartLoc == nullptr){
                occStartLoc = occCurrent->startLoc;
            }
            // Update chunk locations
            occCurrent->startIndex -= freeCurrent->chunkSize;
            occCurrent->startLoc = &memoryPool[occCurrent->startIndex];
            occCurrent = occCurrent->next;
        }
        memmove(freeCurrent->startLoc, occStartLoc, totalMove);
        prevFreeChunk = freeCurrent;
        freeCurrent = freeCurrent->next;
        delete prevFreeChunk;
    };

    // Update free list
    freeCurrent->startIndex -= totalFree;
    freeCurrent->chunkSize += totalFree;
    freeCurrent->startLoc = &memoryPool[freeCurrent->startIndex];
    // delete all free chunks and set tail free chunk to freeHead
    
    freeHead = freeCurrent;
};



