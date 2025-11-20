# MemoryAllocator
This document describes the design of the C++ Dynamic Memory Allocator. It is compliant with, but not dependent on STL.

This document is aimed at the following audiences:
 * software engineers
 * project managers 

 This document assumes that you understand the basics of memory allocation.

 Goals:
  - efficiency
  - not dependent on STL
  - compliant with STL

Non-Goals
    - using less memory compared to existing allocation functions

## Data Structures
There is one main data structure used in the **Memory Allocator**:
* **Chunk**--> used to represent blocks of **allocated** memory

### Structure: Chunk
The **Chunk** structure is used to represent blocks of memory within the memory pool. These Chunks can be either free or occupied (actively storing allocated memory). In the allocator, chunks are stored in three doubly linked lists: free list, occupied (occ) list, and absolute (abs) list. 

 The free list is comprised of a contiguous Chunk of free memory that can be used for allocation. This chunk can be split when memory needs to be allocated. The occ list is comprised of a series of occupied Chunks. These chunks are not soretd in any particular order; typically, they will appear in the order they were allocated. 
 
  The abs list is comprised of both the occupied and free chunks. Ideally, in a non-fragmented memory pool, it consists of a series of occupied chunks (typically sorted in order of allocation) with one contiguous free chunk at the end (all free memeory is included in this chunk). It functions as an easily traversable representation of the overall memory pool. The abs list is primarily used to check for fragmentation in the memory pool. 

  Doubly linked lists allow for two-direction traversal, which is helpful when accounting for edge cases in which the previous object in a list needs to be referenced. For example, when maintaining one contiguous chunk of free memory in the free list, it may be necessary to see if there is another free chunk before the contiguous chunk so that the two can be combined. 

 <img width="1387" height="601" alt="chunkListDiagram" src="https://github.com/user-attachments/assets/ead3c2f7-caa6-4937-b2c8-d1447e859a6b" />


## Memory Allocation 
To allocate memory, the program calls a reimplemented malloc() function. The function allocates the memory by taking away the needed memory size from the free list, selecting a free chunk that is large enough to hold the newly allocated memory. The fundtion adds the new occupied memory Chunk to the occ list, and updates the abs list to match. calloc() can also be used with the allocator. The function allocates memory the same way as the original C memory allocation function, but is adapted for the doubly-linked lists used in this allocator.  

When memory needs to be moved, or reallocated, the realloc() function is called. The function searches for space to reallocate the memory, reallocates the memory, and updates the free, occ, and abs list to match. 

<img width="853" height="1462" alt="memoryAllocationFlowchart" src="https://github.com/user-attachments/assets/776c378f-1806-41ea-9ddf-fe65355fcc23" />


## Freeing Memory 
When memory needs to be freed, the reimplemented free() function is called. The function adds the freed memory back to the free Chunk, removes the newly freed Chunk from the occ list, and updates the abs list to match what is stored in the memory pool.

<img width="883" height="1455" alt="freeMemoryFlowchart" src="https://github.com/user-attachments/assets/0f267920-7a20-46ba-9572-257b0a5bd0c5" />


## Defragmenting Memory
When memory needs to be defragmented, the defragment() function is called. The function moves all free chunks to the contiguous free chunk at the end of the list, ensuring that the memory is not fragmented. 

<img width="685" height="1288" alt="defragmentFlowchart" src="https://github.com/user-attachments/assets/66250a5b-f9ab-40a6-a5b9-0c0d56d36c60" />


## Limitations

realloc() may have less capabilities in some scenarios due to the discontinued use of double pointers to track and manage chunks in compliance with c++ stl. 
