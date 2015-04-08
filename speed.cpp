#include "config.h"

#include <immintrin.h>
#include <x86intrin.h>

#include <cstdint>
#include <cstdlib>
#include <cassert>

#include <string>
#include <chrono>

// --------------------------------------------------

#include "popcnt-lookup.cpp"
#include "popcnt-bit-parallel-scalar.cpp"

#include "sse_operators.cpp"
#include "popcnt-sse-bit-parallel.cpp"
#include "popcnt-sse-lookup.cpp"

#include "popcnt-cpu.cpp"

// --------------------------------------------------


void print_help(const char* name);
bool is_name_valid(const std::string& name);


int main(int argc, char* argv[]) {

    // 1. parse arguments

    if (argc != 4) {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    const std::string name  = argv[1];
    const int size       = std::atoi(argv[2]);
    const int count      = std::atoi(argv[3]);

    if (!is_name_valid(name)) {
        printf("Unknown function name '%s'.\n", name.c_str());
        return EXIT_FAILURE;
    }

    if (size <= 0) {
        printf("Size must be greater than 0.\n");
        return EXIT_FAILURE;
    }

    if (count <= 0) {
        printf("Iteration count must be greater than 0.\n");
        return EXIT_FAILURE;
    }

    // 2. initialize memory

    uint8_t* data = static_cast<uint8_t*>(malloc(size));
    if (data == nullptr) {
        printf("allocation failed");
        return EXIT_FAILURE;
    }

    for (int i=0; i < size; i++) {
        data[i] = i;
    }


    // 3. run the test

    size_t n = 0;
    size_t k = count;

    std::printf("running %-30s...", name.c_str());
    std::fflush(stdout);

    const auto t1 = std::chrono::high_resolution_clock::now();

    if (name == "lookup-8") {
        
        while (k-- > 0) {
            n += popcnt_lookup_8bit(data, size);
        }

    } else if (name == "lookup-64") {
        
        while (k-- > 0) {
            n += popcnt_lookup_64bit(data, size);
        }

    } else if (name == "bit-parallel") {

        while (k-- > 0) {
            n += popcnt_parallel_64bit_naive(data, size);
        }

    } else if (name == "bit-parallel-optimized") {

        while (k-- > 0) {
            n += popcnt_parallel_64bit_optimized(data, size);
        }
    } else if (name == "sse-bit-parallel") {

        while (k-- > 0) {
            n += popcnt_SSE_bit_parallel(data, size);
        }
    } else if (name == "sse-lookup") {

        while (k-- > 0) {
            n += popcnt_SSE_lookup(data, size);
        }
    } else if (name == "cpu") {

        while (k-- > 0) {
            n += popcnt_cpu_64bit(data, size);
        }
    } else {
        assert(false && "wrong function name handling");
    }

    const auto t2 = std::chrono::high_resolution_clock::now();

    const std::chrono::duration<double> td = t2-t1;

    printf("reference result = %lu, time = %10.6f s\n", n, td.count());

    free(data);

    return EXIT_SUCCESS;
}


void print_help(const char* name) {
    std::printf("usage: %s function size iteration_count\n", name);
    std::puts("");
    std::puts("1. function - one of:");
    std::puts("   * see-lookup              - SSSE3 variant using pshufb instruction");
    std::puts("   * lookup-8                - lookup in std::uint8_t[256] LUT");
    std::puts("   * lookup-64               - lookup in std::uint64_t[256] LUT");
    std::puts("   * bit-parallel            - naive bit parallel method");
    std::puts("   * bit-parallel-optimized  - a bit better bit parallel");
    std::puts("   * sse-bit-parallel        - SSE implementation of bit-parallel-optimized");
#ifdef HAVE_POPCNT_INSTRUCTION
    std::puts("   * cpu                     - CPU instruction popcnt (64-bit variant)");
#endif
    std::puts("2. size - size of buffer in 16-bytes chunks");
    std::puts("3. iteration_count - as the name states");
}


bool is_name_valid(const std::string& name) {

    return (name == "lookup-8")
        || (name == "lookup-64")
        || (name == "bit-parallel")
        || (name == "bit-parallel-optimized")
        || (name == "sse-bit-parallel")
        || (name == "sse-lookup")
#ifdef HAVE_POPCNT_INSTRUCTION
        || (name == "cpu")
#endif
        ;
}

