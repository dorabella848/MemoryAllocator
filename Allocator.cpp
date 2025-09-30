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
    // iterate through the free list and occ list
    // if the free chunk is at an earlier index then ouput that and go to next
    // free chunk 
    // if its a nullptr then skip it
    // if theyre both nullptrs then end
    Chunk* occCurrent = occHead;
    Chunk* freeCurrent = freeHead;
    //cout << occHead << endl << freeHead << endl;
    cout << "{";
    while(freeCurrent != nullptr || occCurrent != nullptr){
        if( freeCurrent != nullptr && ((occCurrent == nullptr) || (freeCurrent->startIndex < occCurrent->startIndex)) ){
            cout << "Free, Size: " << freeCurrent->chunkSize << ", startIndex: " << freeCurrent->startIndex << " ||| ";
            freeCurrent = freeCurrent->next;
        }
        else if( occCurrent != nullptr && ((freeCurrent == nullptr) || (freeCurrent->startIndex > occCurrent->startIndex)) ){
            cout << "Occupied, Size: " << occCurrent->chunkSize << ", startIndex: " << occCurrent->startIndex << " ||| ";
            occCurrent = occCurrent->next;
        }
    }
    cout << "ENDOFMEMORY}";
};

void *Allocator::malloc(size_t size)
{
    // Input sanitization
    if(size <= 0){
        return nullptr;
    }

    Chunk* freeCurrent = freeHead;
    Chunk* prevFreeChunk = nullptr;
    
    while(freeCurrent != nullptr){
        if(freeCurrent->chunkSize >= size){

            // Create a new chunk for the allocated memory
            Chunk* newChunk = new Chunk(freeCurrent->startIndex, size, false);
            newChunk->startLoc = &memoryPool[newChunk->startIndex];
            
            // Update the existing free chunk
            freeCurrent->startIndex += size;
            freeCurrent->chunkSize -= size;
            
            if (freeCurrent->chunkSize == 0){
                if (prevFreeChunk == nullptr){
                    freeHead = freeCurrent->next;
                } else {
                    prevFreeChunk->next = freeCurrent->next;
                }
                delete freeCurrent;
            }

            // Now, insert the new occupied chunk into the occupied list
            Chunk* occCurrent = occHead;
            Chunk* prevOccChunk = nullptr;
            
            // Special case for an empty occupied list
            if (occHead == nullptr) {
                occHead = newChunk;
                return &(newChunk->startLoc);
            }

            // Insert in a sorted manner into the occupied list
            while (occCurrent != nullptr && occCurrent->startIndex < newChunk->startIndex) {
                prevOccChunk = occCurrent;
                occCurrent = occCurrent->next;
            }

            if (prevOccChunk == nullptr) { // Insert at the head
                newChunk->next = occHead;
                occHead = newChunk;
            } else { // Insert in the middle or at the end
                prevOccChunk->next = newChunk;
                newChunk->next = occCurrent;
            }

            return &(newChunk->startLoc);
        }

        prevFreeChunk = freeCurrent;
        freeCurrent = freeCurrent->next;
    }

    // No available space found after traversing the free list
    return nullptr;
}


void Allocator::free(void* ptr) {
    Chunk* newFree = occHead;
    Chunk* prevChunk = nullptr;
    Chunk* freeCurrent = freeHead;
    Chunk* prevFreeChunk = nullptr;

    while(newFree != nullptr){
        if(newFree->startLoc == ptr){

            if(prevChunk == nullptr){
                occHead = newFree->next;
            }
            else{
                prevChunk->next = newFree->next;
            }
            // Update free state
            newFree->Free = true;
            newFree->next = nullptr;
            // handle if freecurrent should be free head
            if(freeHead == nullptr){
                freeHead = newFree;
            }
            else{
                while(freeCurrent != nullptr){
                    // check if freeCurrent has a higher index than newFree
                    // if it does then the newFree's next should point
                    // to the prevFreeChunk's next and the prevFreeChunk's next
                    // should be updated to the newFree
                    if ((*freeCurrent).startIndex > (*newFree).startIndex){
                        if(freeCurrent == freeHead){
                            newFree->next = freeCurrent;
                            freeHead = newFree;
                        }

                        // Check if the next free chunk is free
                        if( (newFree->startIndex + newFree->chunkSize == (*(freeCurrent)).startIndex) ){
                            // if it is then add its chunk size to the
                            // current free chunk and delete the next chunk
                            (*newFree).chunkSize += (*freeCurrent).chunkSize;
                            newFree->next = freeCurrent->next;
                            delete freeCurrent;                        
                        }
                        // repeat for looking behind (delete current chunk add size to prev chunk)
                        if( prevFreeChunk != nullptr && prevFreeChunk->startIndex + prevFreeChunk->chunkSize == newFree->startIndex){
                            (*prevFreeChunk).chunkSize += (*newFree).chunkSize;
                            prevFreeChunk->next = newFree->next;
                            delete newFree;
                        }
                        break;
                    }
                    prevFreeChunk = freeCurrent;
                    freeCurrent = freeCurrent->next;
                }
                // Handle if free chunk should be at then end of the free list
                if(freeCurrent == nullptr){
                    // check behind
                    if ((*prevFreeChunk).startIndex + prevFreeChunk->chunkSize == newFree->startIndex){
                        (*prevFreeChunk).chunkSize += (*newFree).chunkSize;
                        delete newFree;
                    }
                    else{
                        prevFreeChunk->next = newFree;
                    }
                }
            };
            // If the program finds the position of the current occupied chunk (newFree) no need to keep checking
            break;
        };
        prevChunk = newFree;
        newFree = newFree->next;
    };
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



