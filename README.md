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







