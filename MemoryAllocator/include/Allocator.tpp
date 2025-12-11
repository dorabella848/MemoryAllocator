#pragma once
template <typename T>
inline Allocator<T>::Allocator()
{
    memorySize = 100000;
    freeMemory = memorySize;
    memoryPool = new uint8_t[memorySize];
    freeHead = new Chunk<T>(0, memorySize, true);
    (*freeHead).startLoc = reinterpret_cast<T*>(&memoryPool[0]);
};

template <typename T>
inline Allocator<T>::~Allocator(){
    delete[] memoryPool;
    // Delete all free and occupied chunks
    Chunk<T>* currentChunk = nullptr;
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

    Chunk<T>* occCurrent = occHead;
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

template <typename T>
inline Allocator<T>::Allocator(std::size_t size){
    memorySize = size;
    freeMemory = memorySize;
    memoryPool = new uint8_t[memorySize];
    freeHead = new Chunk<T>(0, memorySize, true);
    (*freeHead).startLoc = reinterpret_cast<T*>(&memoryPool[0]);
};

template <typename T>
inline Allocator<T>::Allocator(const Allocator& other) {
    memorySize = other.memorySize;
    freeMemory = other.freeMemory;

    // Allocate a new pool
    memoryPool = new uint8_t[memorySize];
    std::copy(other.memoryPool, other.memoryPool + other.memorySize, memoryPool);

    // Create a new freeHead (don’t just copy pointer!)
    freeHead = new Chunk<T>(0, memorySize, true);
    freeHead->startLoc = reinterpret_cast<T*>(&memoryPool[0]);
};

template <typename T>
template <typename U>
inline Allocator<T>::Allocator(const Allocator<U>& other){
    memorySize = other.memorySize;
    freeMemory = other.freeMemory;

    memoryPool = new uint8_t[memorySize];
    std::copy(other.memoryPool, other.memoryPool + other.memorySize, memoryPool);

    freeHead = new Chunk<T>(0, memorySize, true);
    freeHead->startLoc = reinterpret_cast<T*>(&memoryPool[0]);

};

template <typename T>
inline Chunk<T>* Allocator<T>::getFreeHead(){ return freeHead; };

template <typename T>
inline Chunk<T>* Allocator<T>::getOccHead(){ return occHead; };

template <typename T>
inline int Allocator<T>::getFreeMemory(){ return freeMemory; };

template <typename T>
inline int Allocator<T>::getMemoryTotal(){ return memorySize; };

template <typename T>
inline T* Allocator<T>::getMemAddress(size_t index){ return reinterpret_cast<T*>(&memoryPool[index]); };

template <typename T>
inline void Allocator<T>::printChunks(){
    std::cout << "\nTotal Memory: " << memorySize << std::endl;
    switch(occHead != nullptr){
        case true: std::cout << "occHead: " << occHead->startLoc << std::endl; break;
        default: std::cout << "occHead: " << 0 << std::endl; break;
    }
    switch(freeHead != nullptr){
        case true: std::cout << "freeHead: " << freeHead->startLoc << std::endl << std::endl; break;
        default: std::cout << "freeHead: " << 0 << std::endl << std::endl; break;
    }

    Chunk<T>* currentChunk = nullptr;
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
        std::cout << "}\n" << std::endl;
        currentChunk = currentChunk->AbsNext;
        

    }
    std::cout << "ENDOFMEMORY" << std::endl;
};

template <typename T>
inline T* Allocator<T>::allocate(size_t size){
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
        std::cout << "Allocation failed for pointer of size (" << size  <<"): lack of free memory" << std::endl;
        return nullptr;
    }

    Chunk<T>* freeCurrent = freeHead;
    while(freeCurrent != nullptr){
        if(freeCurrent->chunkSize >= size){
            break;
        }
        freeCurrent = freeCurrent->next;
    }
    if(freeCurrent == nullptr){
        return nullptr;
    }

    Chunk<T>* newChunk = new Chunk<T>(freeCurrent->startIndex, size, false);
    newChunk->startLoc = reinterpret_cast<T*>(&memoryPool[newChunk->startIndex]);
    freeCurrent->startIndex += newChunk->chunkSize;
    freeCurrent->startLoc = reinterpret_cast<T*>(&memoryPool[freeCurrent->startIndex]);
    freeCurrent->chunkSize -= newChunk->chunkSize;
    
    if(occHead == nullptr){
        occHead = newChunk;
        newChunk->AbsNext = freeCurrent;
        newChunk->AbsPrev = freeCurrent->AbsPrev;
        freeCurrent->AbsPrev = newChunk;
    }
    else{
        if(freeCurrent->AbsPrev != nullptr){
            // Insert newChunk into the occupied list by using freeCurrent as the reference
            freeCurrent->AbsPrev->next = newChunk;
            freeCurrent->AbsPrev->AbsNext = newChunk;
        }
        if(freeCurrent->AbsNext != nullptr){
            freeCurrent->AbsNext->prev = newChunk;
        }
        newChunk->AbsPrev = freeCurrent->AbsPrev;
        newChunk->prev = freeCurrent->AbsPrev;
        freeCurrent->AbsPrev = newChunk;
        newChunk->AbsNext = freeCurrent;
        newChunk->next = freeCurrent->AbsNext;
        if(freeCurrent->AbsNext == occHead){
            occHead = newChunk;
        }
        
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

    return (newChunk->startLoc);

};

template <typename T>
inline void Allocator<T>::deallocate(T* ptr){
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
    Chunk<T>* newFree = occHead;
    // keep track of previous occupied chunk so there is no need to maintain a prev variable
    //Chunk* prevOccChunk = nullptr;
    
    while(newFree != nullptr){
        if(newFree->startLoc == ptr){
            break;
        }
        newFree = newFree->AbsNext;
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
            Chunk<T>* nextFree = newFree->AbsNext;
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
            Chunk<T>* prevFree = newFree->AbsPrev;
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

    Chunk<T>* prevFreeChunk = freeHead;
    while(prevFreeChunk->next != nullptr && prevFreeChunk->next->startIndex < newFree->startIndex){
        prevFreeChunk = prevFreeChunk->next;
    }

    newFree->next = prevFreeChunk->next;
    newFree->prev = prevFreeChunk;
    if(prevFreeChunk->next != nullptr){
        prevFreeChunk->next->prev = newFree;
    }
    prevFreeChunk->next = newFree;
};

template <typename T>
inline void Allocator<T>::deallocate(T* p, std::size_t n) {
    deallocate(p);
};

template <typename T>
inline T* Allocator<T>::calloc(size_t number, size_t size){
    T* arr = (this->allocate(number*size));
    memset(arr, 0, number*size);
    return arr;
};

template <typename T>
inline T* Allocator<T>::reallocate(T* ptr, size_t size){
    Chunk<T>* target = occHead;
    // Maybe reimplement this to be a weird way to perform deallocate?
    if (size == 0){
        std::cout << "WARNING: Attempted to reallocate ptr {"<< ptr << "} to 0 bytes" << std::endl;
        return ptr;
    }
    if (ptr == nullptr){
        return nullptr;
    }
    while (target->startLoc != ptr){
        target = target->next;
        if (target->startLoc == ptr){
            break;
        }
    }
    if(target == nullptr){
        return nullptr;
    }
    // Check if its even possible to perform the new reallocation
    if( (size > target->chunkSize) && (freeMemory < size - target->chunkSize) ){
        std::cout << "Reallocation failed for " << ptr << ": lack of free memory" << std::endl;
        return target->startLoc;
    }
    if (target->chunkSize >= size){
        // We know the pointers exist so we can update freeMemory
        freeMemory -= (size - target->chunkSize);
        // if there is a free chunk to the right of target there is no need to create a new free chunk
        if(target->AbsNext != nullptr && target->AbsNext->Free){
            target->AbsNext->chunkSize += target->chunkSize-size;
            target->AbsNext->startIndex -= target->chunkSize-size;
            target->AbsNext->startLoc = reinterpret_cast<T*>(&memoryPool[target->AbsNext->startIndex]);
            target->chunkSize = size;
            return target->startLoc;
        }
        Chunk<T>* newFreeChunk = new Chunk<T>(target->startIndex + target->chunkSize, target->chunkSize-size, true);
        target->chunkSize = size;
        newFreeChunk->startLoc = reinterpret_cast<T*>(&memoryPool[newFreeChunk->startIndex]);
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
            return target->startLoc;
        }
        if(freeHead->startIndex > newFreeChunk->startIndex){
            newFreeChunk->prev = nullptr;
            newFreeChunk->next = freeHead;
            freeHead->prev = newFreeChunk;
            freeHead = newFreeChunk;
            return target->startLoc;
        }
        Chunk<T>* prevFreeChunk = freeHead;
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
        return target->startLoc;
    }
    else {
        // Implement the code that will check if theres free memory ahead of the chunk and see 
        // if there is any need to generate a new chunk rather than updating the old one

        // Maybe implement the code to see if it would be faster to move the chunks ahead of the target
        // rather than move the target itself 
        // (this may be the case whenever the extra needed space required to perform the reallocation in place
        // is less than the main chunk itself I.e moving the data of a 50 byte chunk would be easier than
        // a one million byte chunk)
        if(target->AbsNext->Free && (target->chunkSize + target->AbsNext->chunkSize) >= size){
            // Remove used space from next free chunk
            target->AbsNext->chunkSize -= (size - target->chunkSize);
            // Since this is performed in place we have to manually update freeMemory
            this->freeMemory -= (size - target->chunkSize);

            // Check if the free chunk used was exhausted
            Chunk<T>* freeChunk = target->AbsNext;
            if(freeChunk->chunkSize == 0){
                target->AbsNext = freeChunk->AbsNext;
                if(freeChunk->AbsNext != nullptr){
                    freeChunk->AbsNext->AbsPrev = target;
                }
                if(freeChunk->prev != nullptr){
                    freeChunk->prev->next = freeChunk->next;
                }
                if(freeChunk == freeHead){
                    freeHead = freeChunk->next;
                    if(freeChunk->next != nullptr){
                        freeChunk->next->prev = nullptr;
                    }
                }
                else {
                    if(freeChunk->next != nullptr){
                        freeChunk->next->prev = freeChunk->prev;
                    }
                }
                delete freeChunk;
            }
            else{
                // if the free block stays, then we have to update its positional pointers (otherwise we end up
                // deleting the next occupied chunk)
                target->AbsNext->startIndex += (size - target->chunkSize); 
                target->AbsNext->startLoc = getMemAddress(target->AbsNext->startIndex);
            }
            // Update target itself
            target->chunkSize = size;
            return target->startLoc;

        }
        else{
            int targetSize = target->chunkSize;
            this->deallocate(target->startLoc);
            T* newBlock = this->allocate(size);
            if(newBlock == nullptr){
                return nullptr;
            }
            memmove(newBlock, ptr, targetSize);
            return newBlock;  
        }  

    }
};