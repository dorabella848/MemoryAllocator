# MemoryAllocator
This document describes the design of the C++ Dynamic Memory Allocator. 

This document is aimed at the following audiences:
 * software engineers
 * project managers 

 This document assumes that you understand the basics of memory allocation.

## Data Structures
There is one main data structure used in the **Memory Allocator**:
* **Chunk**--> used to represent blocks of **allocated** memory

### Structure: Chunk
The **Chunk** structure is used to represent blocks of memory within the memory pool. These Chunks can be either free or occupied (actively storing allocated memory). In the allocator, chunks are stored in three doubly linked lists: free list, occupied (occ) list, and absolute (abs) list.

 The free list is comprised of a contigious Chunk of free memory that can be used for allocation. The occ list is comprised of a series of occupied Chunks. The abs list is comprised of both the occupied and free chunks, representing the memory pool as a whole. 

 //Diagram//

## Memory Allocation 
To allocate memory, the program calls a reimplemented malloc() function. The function allocates the memory, taking away the needed memory size from the free list, adding the new occupied memory Chunk to the occ list, and updating the abs list to match. calloc() can also be used with the allocator. The function allocates memory the same way as the original C memory allocation function, but is adapted for the doubly-linked lists used in this allocator.  

When memory needs to be moved, or reallocated, the realloc() function is called. The function searches for space to reallocate the memory, reallocates the memory, and updates the free, occ, and abs list to match. 

//Flowchart//

## Freeing Memory 
When memory needs to be freed, the reimplemented free() function is called. The function adds the freed memory back to the free Chunk, removes the newly freed Chunk from the occ list, and updates the abs list to match what is stored in the memory pool.


//Flowchart//

## Limitations
realloc() may have less capabilities in some scenarios due to the discontinued use of double pointers to track and manage chunks in compliance with c++ stl. 