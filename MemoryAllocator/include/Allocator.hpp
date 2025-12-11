#include "Chunk.hpp"
#include <new>
#include <cstdint>
#include <cstdio>
#include <cmath> 
#include <cstring>
#include <iostream>
#include <stdexcept>

#pragma once
template <typename T>
class Allocator {
    private:
        int memorySize;
        int freeMemory;
        uint8_t* memoryPool;
        Chunk<T>* occHead = nullptr;
        Chunk<T>* freeHead = nullptr;
        
    public:
        using value_type = T;
        Allocator();
        ~Allocator();
        Allocator(std::size_t size);
        // defailt copy constructor
        Allocator(const Allocator& other);
        // changed type copy constrcu
        template <typename U>
        Allocator(const Allocator<U>& other);

        Chunk<T>* getFreeHead();

        Chunk<T>* getOccHead();

        int getFreeMemory();

        int getMemoryTotal();

        T* getMemAddress(size_t index);

        void printChunks();

        T* allocate(size_t size);

        void deallocate(T* ptr);
        // STL-required overload
        void deallocate(T* p, std::size_t n);

        T* calloc(size_t number, size_t size);
        T* reallocate(T* ptr, size_t size);
};
#include "Allocator.tpp"
