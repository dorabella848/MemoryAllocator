#include <benchmark/benchmark.h>
#include "Allocator.hpp"
#include <cstring>
// Continuously create chunks of a varied size, empty the memory pool when there is no free memory left
static void Malloc_Complexity(benchmark::State& state) {
    auto alloc = std::make_unique<Allocator<int>>(INT32_MAX/sizeof(int));

    for (auto _ : state) {
        alloc->allocate(state.range(0));

        state.PauseTiming();
        if (alloc->getFreeMemory() < state.range(0)*sizeof(int)) {
            alloc.reset();
            alloc = std::make_unique<Allocator<int>>(INT32_MAX/sizeof(int));
        }
        state.ResumeTiming();
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(Malloc_Complexity)->Range(1, 1 << 4)->Complexity();

// Allocate a psuedo-random sized chunk, clear memory pool if it returns nullptr, clear memory pool and attempt again
// Run for 1 million allocations
static void Malloc_Performance(benchmark::State& state){
    auto alloc = std::make_unique<Allocator<int>>(INT32_MAX/sizeof(int));
    int allocationSize = rand() % 1<<3;  
    void* ptr = nullptr;
    for (auto _ : state){
        ptr = alloc->allocate(allocationSize);
        if (ptr == nullptr) {
            state.PauseTiming();
            alloc.reset();
            alloc = std::make_unique<Allocator<int>>(INT32_MAX/sizeof(int));
            state.ResumeTiming();
        }
    }

}
BENCHMARK(Malloc_Performance)->Iterations(1000*1000)->Complexity();

// Allocate posToBeFreed number of chunks of size BlockSize and attempt to free the final chunk
static void Free_Complexity_Size(benchmark::State& state){
    Allocator<int> alloc(INT32_MAX/sizeof(int));
    int* finalChunk = nullptr;
    for(int i = 0; i < state.range(1); i++){
        finalChunk = alloc.allocate(state.range(0)); // Max num bytes needed is 2^12 * 2^4 = 2^16
    }

    for (auto _ : state) {
        alloc.deallocate(finalChunk);

        state.PauseTiming();
        finalChunk = alloc.allocate(state.range(0)); // undo free for next test
        state.ResumeTiming();
    }
    state.SetComplexityN(state.range(1));
}
//                                  BlockSize     posToBeFreed
BENCHMARK(Free_Complexity_Size)->Ranges({{1, 1 << 4}, {1, 1 << 6}})->Complexity();

// Allocate state.range(0) number of  chunks and attempt to free the final chunk
static void Free_Complexity(benchmark::State& state){
    Allocator<int> alloc(INT32_MAX/sizeof(int));
    int* finalChunk = nullptr;
    for(int i = 0; i < state.range(0); i++){
        finalChunk = alloc.allocate(1); // Max num bytes needed is 2^16
    }

    for (auto _ : state) {
        alloc.deallocate(finalChunk);

        state.PauseTiming();
        finalChunk = alloc.allocate(1); // undo free for next test
        state.ResumeTiming();
    }
    state.SetComplexityN(state.range(0));
}
//                                posToBeFreed
BENCHMARK(Free_Complexity)->Range(1, 1 << 16)->Complexity();

// Test when reallocating a chunk to a larger chunksize
static void ReallocMore_Complexity(benchmark::State& state){
    Allocator<int> alloc(INT32_MAX/sizeof(int));
    int* TestChunk = alloc.allocate(state.range(0)/4);
    int* fragmentationMaker = alloc.allocate(1);
    for (auto _ : state) {

        TestChunk = alloc.reallocate(TestChunk, state.range(0)/2); // Reallocate to *2 original size

        state.PauseTiming();
        alloc.deallocate(TestChunk);
        TestChunk = alloc.allocate(state.range(0));
        state.ResumeTiming();
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(ReallocMore_Complexity)->DenseRange(30000000, 30000000*2, (30000000/25))->Complexity()->UseRealTime();
BENCHMARK_MAIN();
