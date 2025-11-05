#include <benchmark/benchmark.h>
#include "Allocator.hpp"
#include "Chunk.hpp"

// static void Malloc_SingleByte(benchmark::State& state) {
//     Allocator alloc(100000000);
//     for (auto _ : state)
//         alloc.malloc(1);
// }
// BENCHMARK(Malloc_SingleByte)->Iterations(1000000);

static void Malloc_Complexity(benchmark::State& state) {
    Allocator* alloc = new Allocator(INT32_MAX);

    for (auto _ : state) {
        alloc->malloc(state.range(0));

        state.PauseTiming();
        if (alloc->getFreeMemory() < state.range(0)) {
            delete alloc;
            alloc = new Allocator(INT32_MAX);
        }
        state.ResumeTiming();
    }

    delete alloc; // cleanup
    state.SetComplexityN(state.range(0));
}
BENCHMARK(Malloc_Complexity)->Range(1, 1 << 16)->Complexity();





BENCHMARK_MAIN();
