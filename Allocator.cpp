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
        printf("Ptr: %-14p\n", currentChunk->startLoc);
        printf("{ Free:  %-5s", currentChunk->Free ? "true" : "false");
        printf("| AbsNext: %-14p", (currentChunk->AbsNext != nullptr) ? currentChunk->AbsNext->startLoc : nullptr);
        printf("| Size: %-*d", (int)round(log10(memorySize)), currentChunk->chunkSize);
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
        if(freeCurrent->AbsPrev->next != nullptr){
            freeCurrent->AbsPrev->next->prev = newChunk;
        }
        freeCurrent->AbsPrev->next = newChunk;
        freeCurrent->AbsPrev->AbsNext = newChunk;
        newChunk->AbsPrev = freeCurrent->AbsPrev;
        newChunk->prev = freeCurrent->AbsPrev;
        freeCurrent->AbsPrev = newChunk;
        // set new chunk's absnext to the free chunk you are taking memory from and its next to the freechunk's absnext
        newChunk->AbsNext = freeCurrent;
        newChunk->next = freeCurrent->AbsNext; // (might be a nullptr but no point in confirming before assign)
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

void Allocator::free(void* ptr){
    // Remove the connections to the block being freed by updating its prev's next and its next's prev
    // set the chunk to free
    // if there is a free chunk next to it then use that to update the freed chunk's next and prev vars
    // otherwise
    // check if the freeHead is a nullptr, if it is then the new chunk is the free head and its next and prev should
    // be nullptrs
    // otherwise
    // use a loop through the free list to find the closest free chunk before the new free's location
    // if the first free block is after newFree then newFree is new freeHead and newFree's prev is nullptr
    // (this block will be newFree's next if that is the case)
    // once the closest behind free block is found (excluded if first was after newFree)
    // then updated next and prev values (next is going to be prevFree's next)
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
    if(newFree == occHead){
        if(newFree->next != nullptr){
            occHead = newFree->next;
        }
        else{
            occHead = nullptr;
        }
    }

    // Update the previous and next occupied chunks' pointers
    if(newFree->prev != nullptr){
        newFree->prev->next = newFree->next;
    }
    if(newFree->next != nullptr){
        newFree->next->prev = newFree->prev;
    }
    newFree->Free = true;

    // Check for adjacent free chunks
    if( (newFree->AbsNext != nullptr && newFree->AbsNext->Free) || (newFree->AbsPrev != nullptr && newFree->AbsPrev->Free) ){
        if(newFree->AbsNext != nullptr && newFree->AbsNext->Free){
            Chunk* nextFree = newFree->AbsNext; // newFree->absnext is going to be deleted since no two free chunks
            newFree->prev = nextFree->prev;     // are to be adjacent
            newFree->next = nextFree->next;
            if(nextFree->prev != nullptr){
                nextFree->prev->next = newFree;
            }
            if(nextFree->next != nullptr){
                nextFree->next->prev = newFree;
            }

            // Update adjacent chunk abs pointers
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
        if(newFree->AbsPrev != nullptr && newFree->AbsPrev->Free){
            Chunk* prevFree = newFree->AbsPrev; // newFree->absnext is going to be deleted since no two free chunks
            newFree->AbsPrev->AbsNext = newFree->AbsNext; 
            if(newFree->AbsNext != nullptr){
                newFree->AbsNext->AbsPrev = newFree->AbsPrev;
            }
            prevFree->chunkSize += newFree->chunkSize;
            
            delete newFree;
        }
        return;
    }
    // if the freeHead is after newFree then newFree is the new freeHead (update pointers appropiately)
    // otherwise
    // go until 
    // prevFreeChunk's next is a nullptr
    // or
    // prevFreeChunk-next's startIndex is after newFree
    // if the next is a nullptr, then newFree is the last free block (prev is prevFreeCHunk, next is nullptr)
    // otherwise newFree is between 2 free blocks 
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

    // Find closest previous free chunk
    Chunk* prevFreeChunk = freeHead;
    while(prevFreeChunk->next != nullptr && prevFreeChunk->next->startIndex > newFree->startIndex){
        prevFreeChunk = prevFreeChunk->next;
    }

    newFree->next = prevFreeChunk->next;
    newFree->prev = prevFreeChunk->prev;
    if(prevFreeChunk->next != nullptr){
        prevFreeChunk->next->prev = newFree;
    }
    prevFreeChunk->next = newFree;
}

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
        if(prevFreeChunk->AbsPrev != nullptr){
            prevFreeChunk->AbsPrev->AbsNext = prevFreeChunk->AbsNext;
        }
        prevFreeChunk->AbsNext->AbsPrev = prevFreeChunk->AbsPrev;
        delete prevFreeChunk;
    };

    // Update free list
    freeCurrent->startIndex -= totalFree;
    freeCurrent->chunkSize += totalFree;
    freeCurrent->startLoc = &memoryPool[freeCurrent->startIndex];
    freeCurrent->prev = nullptr;
    // delete all free chunks and set tail free chunk to freeHead
    
    freeHead = freeCurrent;
};

void** Allocator::calloc(size_t number, size_t size){
    void** arr = (Allocator::malloc(number*size));
    memset(*arr, 0, number*size);
    return arr;
    // Initialize all bytes to 0
}

void** Allocator::realloc(void* ptr, size_t size){
    //see if block can expand or shrink
    //find new contiguous memory block
    //copy contents over to new block, deallocate old memory
    //if works, return void ptr if not return null ptr
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
    if (target->chunkSize >= size){
        target->chunkSize = size;
        // This should create a new free block after target which has the chunk size of (target->chunksize-size)
        Chunk* newFreeChunk = new Chunk(target->startIndex + target->chunkSize, target->chunkSize-size, true);
        newFreeChunk->startLoc = &memoryPool[newFreeChunk->startIndex];
        // Find closest previous free chunk
        Chunk* prevFreeChunk = freeHead;
        while(prevFreeChunk->next != nullptr && prevFreeChunk->next->startIndex > target->startIndex){
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
        // Added min function since we dont want to copy more than necessary since it is a waste of time.
        // We may want to change chunkSize to size_t so we dont have to convert to int64_t here
        int dataSize = min((int64_t)target->chunkSize, (int64_t)size);
        uint8_t savedData[dataSize];
        memcpy(savedData, ptr, dataSize);
        Allocator::free(target->startLoc);
        void** newBlock = malloc(size);
        if (*newBlock == nullptr){
            return nullptr;
        }
        memcpy(*newBlock, savedData, dataSize);
        return newBlock;    
    }
}



