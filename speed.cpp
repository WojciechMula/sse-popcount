#include "config.h"

#include <immintrin.h>
#include <x86intrin.h>

#include <cstdint>
#include <cstdlib>
#include <cassert>

#include <set>
#include <string>
#include <chrono>

// --------------------------------------------------

#include "popcnt-lookup.cpp"
#include "popcnt-bit-parallel-scalar.cpp"
#include "popcnt-harley-seal.cpp"

#include "sse_operators.cpp"
#include "popcnt-sse-bit-parallel.cpp"
#include "popcnt-sse-lookup.cpp"

#include "popcnt-cpu.cpp"

#if defined(HAVE_AVX2_INSTRUCTIONS)
#   include "popcnt-avx2-lookup.cpp"
#endif

// --------------------------------------------------


void print_help(const char* name);
bool is_name_valid(const std::string& name);


struct Result {
    uint64_t count;
    double   time;
};


template <typename FN>
Result run(FN function, const char* name, const uint8_t* data, size_t size, size_t iterations, double reference) {

    Result result;

    const auto t1 = std::chrono::high_resolution_clock::now();

    printf("%-30s... ", name);
    fflush(stdout);

    size_t n = 0;
    size_t k = iterations;
    while (k-- > 0) {
        n += function(data, size);
    }

    const auto t2 = std::chrono::high_resolution_clock::now();

    const std::chrono::duration<double> td = t2-t1;
    //printf("reference result = %lu, time = %0.6f s", n, td.count());
    printf("time = %0.6f s", td.count());

    if (reference > 0.0) {
        const auto speedup = reference/td.count();

        printf(" (speedup: %3.2f)", speedup);
    }

    putchar('\n');

    result.count = n; // to prevent compiler from optimizing out the loop
    result.time  = td.count();

    return result;
}


int main(int argc, char* argv[]) {

    // 1. parse arguments
    std::set<std::string> functions;

    if (argc < 3) {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    const int size  = std::atoi(argv[1]);
    const int count = std::atoi(argv[2]);

    if (size <= 0) {
        printf("Size must be greater than 0.\n");
        return EXIT_FAILURE;
    }

    if (count <= 0) {
        printf("Iteration count must be greater than 0.\n");
        return EXIT_FAILURE;
    }

    for (int i=3; i < argc; i++) {
        functions.insert(argv[i]);
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
    double time = 0.0;

    if (functions.empty() || functions.count("lookup-8")) {

        auto result = run(popcnt_lookup_8bit, "LUT (uint8_t[256])", data, size, count, time);
        n += result.count;
        if (time == 0.0) {
            time = result.time;
        }
    }

    if (functions.empty() || functions.count("lookup-64")) {

        auto result = run(popcnt_lookup_64bit, "LUT (uint64_t[256])", data, size, count, time);
        n += result.count;
        if (time == 0.0) {
            time = result.time;
        }
    }

    if (functions.empty() || functions.count("bit-parallel")) {

        auto result = run(popcnt_parallel_64bit_naive, "bit parallel", data, size, count, time);
        n += result.count;
        if (time == 0.0) {
            time = result.time;
        }
    }

    if (functions.empty() || functions.count("bit-parallel-optimized")) {

        auto result = run(popcnt_parallel_64bit_optimized, "bit parallel optimized", data, size, count, time);
        n += result.count;
        if (time == 0.0) {
            time = result.time;
        }
    }

    if (functions.empty() || functions.count("harley-seal")) {

        auto result = run(popcnt_harley_seal, "harley-seal", data, size, count, time);
        n += result.count;
        if (time == 0.0) {
            time = result.time;
        }
    }

    if (functions.empty() || functions.count("sse-bit-parallel")) {

        auto result = run(popcnt_SSE_bit_parallel, "bit parallel optimized - SSE", data, size, count, time);
        n += result.count;
        if (time == 0.0) {
            time = result.time;
        }
    }

    if (functions.empty() || functions.count("sse-lookup")) {

        auto result = run(popcnt_SSE_lookup, "SSSE3 [pshufb]", data, size, count, time);
        n += result.count;
        if (time == 0.0) {
            time = result.time;
        }
    }

#if defined(HAVE_AVX2_INSTRUCTIONS)
    if (functions.empty() || functions.count("avx2-lookup")) {

        auto result = run(popcnt_AVX2_lookup, "AVX2 [pshufb]", data, size, count, time);
        n += result.count;
        if (time == 0.0) {
            time = result.time;
        }
    }
#endif

#if defined(HAVE_POPCNT_INSTRUCTION)
    if (functions.empty() || functions.count("cpu")) {

        auto result = run(popcnt_cpu_64bit, "CPU popcnt", data, size, count, time);
        n += result.count;
        if (time == 0.0) {
            time = result.time;
        }
    }
#endif

    free(data);

    return EXIT_SUCCESS;
}



void print_help(const char* name) {
    std::printf("usage: %s buffer_size iteration_count [function(s)]\n", name);
    std::puts("");
    std::puts("1. buffer_size      - size of buffer in 16-bytes chunks");
    std::puts("2. iteration_count  - as the name states");

    std::puts("3. one or more functions (if not given all will run):");
    std::puts("   * sse-lookup              - SSSE3 variant using pshufb instruction");
#if defined(HAVE_AVX2_INSTRUCTIONS)
    std::puts("   * avx2-lookup             - AVX2 variant using pshufb instruction");
#endif
    std::puts("   * lookup-8                - lookup in std::uint8_t[256] LUT");
    std::puts("   * lookup-64               - lookup in std::uint64_t[256] LUT");
    std::puts("   * bit-parallel            - naive bit parallel method");
    std::puts("   * bit-parallel-optimized  - a bit better bit parallel");
    std::puts("   * harley-seal             - Harley-Seal popcount (4th iteration)");
    std::puts("   * sse-bit-parallel        - SSE implementation of bit-parallel-optimized");
#if defined(HAVE_POPCNT_INSTRUCTION)
    std::puts("   * cpu                     - CPU instruction popcnt (64-bit variant)");
#endif
}


bool is_name_valid(const std::string& name) {

    return (name == "lookup-8")
        || (name == "lookup-64")
        || (name == "bit-parallel")
        || (name == "bit-parallel-optimized")
        || (name == "harley-seal")
        || (name == "sse-bit-parallel")
        || (name == "sse-lookup")
#if defined(HAVE_AVX2_INSTRUCTIONS)
        || (name == "avx2-lookup")
#endif
#if defined(HAVE_POPCNT_INSTRUCTION)
        || (name == "cpu")
#endif
        ;
}

