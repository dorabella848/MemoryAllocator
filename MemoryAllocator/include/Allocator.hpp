#include <new>
#include <cstdint>
struct Chunk;

template <typename T>
class Allocator {
    T value;
    private:
        int memorySize;
        int freeMemory;
        uint8_t* memoryPool;
        Chunk* occHead = nullptr;
        Chunk* freeHead = nullptr;
        
    public:
        Allocator(const T& value, std::size_t numBytes);
        ~Allocator();
        Chunk* getFreeHead();
        Chunk* getOccHead();
        int getFreeMemory();
        int getMemoryTotal();
        T* getMemAddress(std::size_t index);
        void printChunks();
        T* malloc(std::size_t size);
        void free(T* ptr);
        T* calloc(std::size_t number, std::size_t size);
        T* realloc(T* ptr, std::size_t size);
};