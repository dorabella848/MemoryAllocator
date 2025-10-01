#include "Allocator.hpp"
#include <list>

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
    cout << "{";
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
        cout << "FreeState: " << currentChunk->Free << ", Size: " << currentChunk->chunkSize << ", startIndex: " << currentChunk->startIndex << " ||| ";
        currentChunk = currentChunk->AbsNext;
        

    }
    cout << "ENDOFMEMORY}" << endl;
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
        freeCurrent->AbsPrev = newChunk;
    }
    else{
        // set the free-chunk-you-are-recieving-memory-from's previous chunk's next to the new occ chunk 
        // (both abs and normal since we are assuming no two free memory blocks are right next to eachother)
        freeCurrent->AbsPrev->next = newChunk;
        freeCurrent->AbsPrev->AbsNext = newChunk;
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
//void Allocator::deleteChunk(Chunk* toRemove, bool freeState);
void Allocator::combineChunks(Chunk* hostChunk, Chunk* toRemove, bool stateDefined){
    // State defined means that the hostChunk is not the newly freed block
    // The host chunk is assumed to be adjacent to the to be reomved chunk and occurs before the toRemove chunk
    
    // find if there is a previous chunk of hostchunk's freestate to update the prev chunk's non-absolute pointer if it isnt already good
    if(!stateDefined){
        Chunk* prevFreeChunk = hostChunk->AbsPrev;
        while(prevFreeChunk != nullptr){
            if(prevFreeChunk->Free){
                prevFreeChunk->next = hostChunk;
            }
        }
    }
    

    hostChunk->next = toRemove->next;
    hostChunk->AbsNext = toRemove->AbsNext;
    hostChunk->chunkSize += toRemove->chunkSize;
    
    if(toRemove->AbsNext != nullptr){
        toRemove->AbsNext->AbsPrev = hostChunk;
    }
    delete toRemove;
}
void Allocator::free(void* ptr) {
    // to make a block free you must update its free state and its next and prev vars
    // find the block you want to remove using a loop
    // also find the closest memory block to the found block using absprev
    // the newly-freed-block's next is the prevfreechunk's next and its prevfreechunk's next's prev is the new free block
    // update prev free block's next to be new free

    //
    //
    //
    // REMEMBER TO COMBINE THE FREE CHUNKS IF THEY ARE NEXT TO EACHOTHER (check the old code that is commented out)
    // basically if prev free chunk start index plus prevfreechunk chunk size == newFree start index
    //
    //
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
                if(nextFreeChunk->startIndex == (newFree->startIndex + newFree->chunkSize))
                {
                    newFree->next = nextFreeChunk->next;
                    newFree->AbsNext = nextFreeChunk->AbsNext;
                    newFree->chunkSize += nextFreeChunk->chunkSize;
                    
                    if(nextFreeChunk->AbsNext != nullptr){
                        nextFreeChunk->AbsNext->AbsPrev = newFree;
                    }
                }
                else{
                    newFree->next = nextFreeChunk;
                }
                
            }
            else{
                newFree->next = nullptr;
            }
            // Check if prevFreeChunk is adjacent to newFree
            // might want to do this before deleting free chunk after newfree to remove redundant logic
            if(prevFreeChunk->startIndex+prevFreeChunk->chunkSize == newFree->startIndex){

                prevFreeChunk->next = newFree->next;
                prevFreeChunk->AbsNext = newFree->AbsNext;
                newFree->AbsNext->AbsPrev = prevFreeChunk;
                prevFreeChunk->chunkSize += newFree->chunkSize;
                delete newFree;
                
            }
            return;
        }
        prevFreeChunk = prevFreeChunk->AbsPrev;
    }
    
    // the free head occurs after the current (confirmed by previous lines (wouldnt have gotten here otherwise))
    // remember to check if they are adjacent in memoryPool
    if(newFree->startIndex+newFree->chunkSize == freeHead->startIndex){
        newFree->next = freeHead->next;
        newFree->AbsNext = freeHead->AbsNext;
        if(freeHead->AbsNext != nullptr){
            freeHead->AbsNext->AbsPrev = newFree;
        }
        newFree->chunkSize += freeHead->chunkSize;
        delete freeHead;
        freeHead = newFree;
                
    }
    else{
        newFree->next = freeHead;
        freeHead->prev = newFree;
        freeHead = newFree;
    }
    // Chunk* prevFreeChunk = nullptr;
    // while(prevFreeChunk != nullptr )

    // while(newFree != nullptr){
    //     if(newFree->startLoc == ptr){

    //         if(prevChunk == nullptr){
    //             occHead = newFree->next;
    //         }
    //         else{
    //             prevChunk->next = newFree->next;
    //         }
    //         // Update free state
    //         newFree->Free = true;
    //         newFree->next = nullptr;
    //         // handle if freecurrent should be free head
    //         if(freeHead == nullptr){
    //             freeHead = newFree;
    //         }
    //         else{
    //             while(freeCurrent != nullptr){
    //                 // check if freeCurrent has a higher index than newFree
    //                 // if it does then the newFree's next should point
    //                 // to the prevFreeChunk's next and the prevFreeChunk's next
    //                 // should be updated to the newFree
    //                 if ((*freeCurrent).startIndex > (*newFree).startIndex){
    //                     if(freeCurrent == freeHead){
    //                         newFree->next = freeCurrent;
    //                         freeHead = newFree;
    //                     }

    //                     // Check if the next free chunk is free
    //                     if( (newFree->startIndex + newFree->chunkSize == (*(freeCurrent)).startIndex) ){
    //                         // if it is then add its chunk size to the
    //                         // current free chunk and delete the next chunk
    //                         (*newFree).chunkSize += (*freeCurrent).chunkSize;
    //                         newFree->next = freeCurrent->next;
    //                         delete freeCurrent;                        
    //                     }
    //                     // repeat for looking behind (delete current chunk add size to prev chunk)
    //                     if( prevFreeChunk != nullptr && prevFreeChunk->startIndex + prevFreeChunk->chunkSize == newFree->startIndex){
    //                         (*prevFreeChunk).chunkSize += (*newFree).chunkSize;
    //                         prevFreeChunk->next = newFree->next;
    //                         delete newFree;
    //                     }
    //                     break;
    //                 }
    //                 prevFreeChunk = freeCurrent;
    //                 freeCurrent = freeCurrent->next;
    //             }
    //             // Handle if free chunk should be at then end of the free list
    //             if(freeCurrent == nullptr){
    //                 // check behind
    //                 if ((*prevFreeChunk).startIndex + prevFreeChunk->chunkSize == newFree->startIndex){
    //                     (*prevFreeChunk).chunkSize += (*newFree).chunkSize;
    //                     delete newFree;
    //                 }
    //                 else{
    //                     prevFreeChunk->next = newFree;
    //                 }
    //             }
    //         };
    //         // If the program finds the position of the current occupied chunk (newFree) no need to keep checking
    //         break;
    //     };
    //     prevChunk = newFree;
    //     newFree = newFree->next;
    // };
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



