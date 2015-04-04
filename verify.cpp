#include <immintrin.h>

#include <cstdint>
#include <cstdlib>

#include <string>

// --------------------------------------------------

#include "popcnt-lookup.cpp"
#include "popcnt-bit-parallel-scalar.cpp"

#include "sse_operators.cpp"
#include "popcnt-sse-bit-parallel.cpp"
#include "popcnt-sse-lookup.cpp"

// --------------------------------------------------

void verify(const char* name, const std::uint8_t* data, const size_t size);

struct function_t {
    const bool     is_reference;
    const char*    name;
    std::uint64_t (*function)(const uint8_t* data, const size_t size);
};

function_t functions[] = {
    {true,  "lookup-8",                popcnt_lookup_8bit},
    {true,  "lookup-64",               popcnt_lookup_64bit},
    {false, "bit-parallel",            popcnt_parallel_64bit_naive},
    {false, "bit-parallel-optimized",  popcnt_parallel_64bit_optimized},
    {false, "sse-bit-parallel",        popcnt_SSE_bit_parallel},
    {false, "sse-lookup",              popcnt_SSE_lookup} 
};


int main() {

    static const size_t size = 1024;
    std::uint8_t data[size];

    // 1. all zeros
    for (size_t i=0; i < size; i++) {
        data[i] = 0;
    }

    verify("all zeros", data, size);


    // 2. all ones
    for (size_t i=0; i < size; i++) {
        data[i] = 0;
    }

    verify("all ones", data, size);

    // 3. ascending
    for (size_t i=0; i < size; i++) {
        data[i] = i;
    }

    verify("asending", data, size);

    // 4. quasirandom
    for (size_t i=0; i < size; i++) {
        data[i] = i*33 + 12345;
    }

    verify("descending", data, size);

    return EXIT_SUCCESS;
}


void verify(const char* name, const std::uint8_t* data, const size_t size) {
 
    puts("");
    printf("test '%s':\n", name);

    const size_t reference = popcnt_lookup_8bit(data, size);

    for (auto& item: functions) {
        if (item.is_reference) {
            continue;
        }

        printf("%-30s: ", item.name);
        const size_t result = item.function(data, size);

        printf("%s\n", (result == reference) ? "OK" : "!!!error!!!");
    }
}
