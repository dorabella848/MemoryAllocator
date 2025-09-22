#include "Allocator.hpp"
#include <list>

Chunk* MemoryAllocator::getFreeHead(){
    return freeHead;
};

void MemoryAllocator::printChunks(){
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

void *MemoryAllocator::malloc(size_t size)
{

    // Start at the beginning and traverse to end, if no space is found: defragment
    Chunk* freeCurrent = freeHead;
    Chunk* occCurrent = occHead;
    Chunk* prevOccChunk;
    Chunk* prevFreeChunk;


    while(freeCurrent != nullptr){
        if((*freeCurrent).chunkSize >= size){
            // Allocate required amount of bytes and if there is left over space break off and form a single free block
            // current pos is (*current).startIndex
            // Create occChunk
            Chunk* newChunk = new Chunk((*freeCurrent).startIndex, size, false);
            newChunk->startLoc = &memoryPool[newChunk->startIndex];
            (*freeCurrent).startIndex += size;
            (*freeCurrent).chunkSize -= size;
            
            if ((*freeCurrent).chunkSize == 0){
                if (freeCurrent == freeHead){
                    freeHead = freeCurrent->next;
                }
                else{
                    prevFreeChunk->next = freeCurrent->next;
                }
                delete(freeCurrent);
            
            };
            // Handle if occHead is nullptr
            if(occHead == nullptr){
                occHead = newChunk;
                return newChunk->startLoc;
            };
            while(occCurrent != nullptr){
                if ((*occCurrent).startIndex > (*newChunk).startIndex){
                    
                    if (occCurrent == occHead){
                        newChunk->next = occHead;
                        occHead = newChunk;
                    }
                    else{
                        prevOccChunk->next = newChunk;
                        newChunk->next = occCurrent;
                    }

                    return newChunk->startLoc;
                };
                
                prevOccChunk = occCurrent;
                occCurrent = occCurrent->next;
            };
            // Handle if the new chunk is at the end of all occupied chunks
            prevOccChunk->next = newChunk;
            return newChunk->startLoc;
        
        };
            
        prevFreeChunk = freeCurrent;
        freeCurrent = freeCurrent->next;
    };
    // Check if there would be enough enough memory after defragmentation
    // if there is defragment and call (and return) malloc


    // No available space
    return nullptr;
};

void MemoryAllocator::free(void* ptr) {
    Chunk* occCurrent = occHead;
    Chunk* prevChunk = nullptr;
    Chunk* freeCurrent = freeHead;
    Chunk* prevFreeChunk = nullptr;

    while(occCurrent != nullptr){
        if(occCurrent->startLoc == ptr){
            if(prevChunk == nullptr){
                occHead = occCurrent->next;
            }
            else{
                prevChunk->next = occCurrent->next;
            }
            // add occCurent to free list and set it's free var to true
            //freeCurrent->next = occCurrent;
            (*occCurrent).Free = true;
            // handle if freecurrent should be free head
            if(freeHead == nullptr){
                freeHead = occCurrent;
            }
            else{
                while(freeCurrent != nullptr){
                    // check if freeCurrent has a higher index than occCurrent
                    // if it does then the occCurrent's next should point
                    // to the prevFreeChunk's next and the prevFreeChunk's next
                    // should be updated to the occCurrent
                    if ((*freeCurrent).startIndex > (*occCurrent).startIndex){

                        if(freeCurrent == freeHead){
                            occCurrent->next = freeCurrent;
                            freeHead = occCurrent;
                            break;
                        }

                        // Check if the next free chunk is free
                        if( (occCurrent->startIndex + occCurrent->chunkSize == (*(freeCurrent)).startIndex) ){
                            // if it is then add its chunk size to the
                            // current free chunk and delete the next chunk
                            (*occCurrent).chunkSize += (*freeCurrent).chunkSize;
                            occCurrent->next = freeCurrent->next;
                            delete freeCurrent;                        
                        }
                        // repeat for looking behind (delete current chunk add size to prev chunk)
                        if( prevFreeChunk != nullptr && prevFreeChunk->startIndex + prevFreeChunk->chunkSize == occCurrent->startIndex){
                            (*prevFreeChunk).chunkSize += (*occCurrent).chunkSize;
                            prevFreeChunk->next = occCurrent->next;
                            delete occCurrent;
                        }
                        break;
                    }
                    prevFreeChunk = freeCurrent;
                    freeCurrent = freeCurrent->next;
                    // Handle if free chunk should be at then end of the free list
                    
                    if(freeCurrent == nullptr){
                        // check behind
                        if ((*prevFreeChunk).startIndex + prevFreeChunk->chunkSize == occCurrent->startIndex){
                            (*prevFreeChunk).chunkSize += (*occCurrent).chunkSize;
                            delete occCurrent;
                        }
                        else{
                            prevFreeChunk->next = occCurrent;
                        };
                        
                        
                    }
                }
            };
            // If the program finds the position of the current occupied chunk no need to keep checking
            break;
        };
        prevChunk = occCurrent;
        occCurrent = occCurrent->next;
    };
};

void MemoryAllocator::defragment(){
    // Move memory around to make one free block

    // Make the last free block the tail end of the big free block
    // Move memory blocks from left to right to the front of the free block tail
    Chunk* occCurrent = occHead;
    Chunk* freeCurrent = freeHead;
    Chunk* freeTailChunk;



    // to remove a free chunk move all subsequent chunks up to the next free chunk
    // Get starting location of the first chunk
    // iterate through list and count chunk sizes
    // use memmove to move the memory to the start loc of the free chunk by chunk total size from the location
    // of the next chunk
    // Update the affected chunks memory location and memory pool index
    // repeat for each memory block
    // if the current free block is the last in the free list then stop
    size_t totalSize = 0;
    while(freeCurrent->next != nullptr){

        
        void* occStartLoc = nullptr;
        while(occCurrent->startIndex < freeCurrent->next->startIndex && occCurrent->startIndex > freeCurrent->startIndex){
            totalSize += occCurrent->chunkSize;
            
            // Gets first occupied chunk start location
            if(occStartLoc == nullptr){
                occStartLoc = occCurrent->startLoc;
            }
            // Update chunk locations
            occCurrent->startIndex -= freeCurrent->chunkSize;
            occCurrent->startLoc = &memoryPool[occCurrent->startIndex];

            occCurrent = occCurrent->next;
        }
        memmove(freeCurrent->startLoc, occStartLoc, totalSize);

        freeCurrent = freeCurrent->next;
    };
    
    // Update free list
    //update tail end chunk: should begin at tail chunk start index - (total free chunksize - tail free chunk size)
    freeTailChunk = freeCurrent->next;
    freeTailChunk->startIndex -= totalSize - (freeTailChunk->chunkSize);
    freeTailChunk->startLoc = &memoryPool[freeTailChunk->startIndex];
    // delete all free chunks and set tail free chunk to freeHead
    
    freeHead = freeTailChunk;
};



