#include "Allocator.cpp"
struct test123 {
    int* a = nullptr;
};
int main()
{
    
    uint8_t num1 = 14;
    int chunkSize = 4096;
    MemoryAllocator allocator123 = MemoryAllocator(8192);
    //int* test = static_cast<int*>(allocator123.malloc(chunkSize));
    int* test = (int*)allocator123.malloc(500);
    int* test1 = (int*)allocator123.malloc(500);
    allocator123.printChunks();
    //allocator123.free(test);
    //allocator123.printChunks();
    return 0;

    // TODO:


    // 9/15/25 Notes
    // Our program defines the chunks outside of the memory pool
    // what we need to do is define block metadeta which attaches our chunk information to the beginning of a memory address
    // the total memory required for the block is the size requested + the size of the chunk itself (not chunkSize)
    // probably have to reimplement everything

    

};