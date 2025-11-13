# MemoryAllocator
A C++ based dynamic memory allocator 

## Design 
This section describes the design of the allocator. 

The Memory Allocator is a dynamic allocator that reimplements C dynamic allocation functions malloc(), calloc(), realloc(), free(), and defragment() in a C++ environment. 

### Allocator Objects
When an allocator is created (include/Allocator.hpp), a memory pool is used to hold/manage free and occupied memory.

#### Chunks
To represent memory and free space in the dynamic allocator, a dynamic "Chunk" structure is used (include/Chunk.hpp). A memory pool is used to store memory chunks. Two doubly linked lists are used to keep track of free and occupied chunks (free and occ). The free list should be one contiguous chunk of free memory, while the occupied list consists of chunks of occupied memory. The memory pool is set up so that it consists of multiple chunks of occupied memory with one contiguous free block at the end. Defragment() is used to maintain this. 
For example:

<img width="932" height="372" alt="chunkDiagram" src="https://github.com/user-attachments/assets/f867bf89-59da-43be-a9e1-ca2dfedbf507" />

### Allocator Functionality
This section describes how the allocator works and the reimplementation of functions used (malloc(), calloc(), realloc(), free(), and defragment())

#### malloc(size_t size)
malloc() is reimplemented for basic allocation of memory. When the function is called, the iterator moves through the memory pool until a large enough free block is found. A (size_t size) amount of memory is then allocated and the block is marked as occupied. The newly occupied block is removed from the free list. 

#### calloc(size_t number, size_t size)
calloc() is reimplemented in the same manner as its original function. It creates an array of elements and initializes them to 0, reserving spaces for the memory needed. 

#### realloc(void* ptr, size_t size)
realloc() is reimplemented for reallocation of a specific chunk of memory. When the function is called, the iterator moves through the occupied list, finds the pointer of the target memory chunk, and reallocates the chunk of memory to the closest free block in the memory pool. 

#### free(void* ptr)
free() is reimplemented to free allocated blocks. It searches through the occupied memory list to see if the desired pointer exists. When found, it removes the chunk associated with the pointer from the occupied list. The freed memory is then merged with the free block in the freelist and in the memory pool.

#### defragment()
defragment() is reimplemented to prevent fragmentation. The function goes through each free chunk (not including the last free chunk) and shifts all occupied chunks left. It then adds the sizes of the free chunks to each other to create one contiguous free block and a set of occupied chunks. It cannot be used in some cases due to the previous use of double pointers to track memory, which is not compliant with c++ stl. 

### Limitations
The allocator is currently limited in its space efficiency and currently cannot handle some cases of defragmentation. This occurs due to limits from compliance with c++ stl (double pointers were previosuly used to track memory, can no longer be used).







