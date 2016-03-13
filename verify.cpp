#include "config.h"

#include <immintrin.h>
#include <x86intrin.h>

#include <cstdint>
#include <cstdlib>
#include <string>

// --------------------------------------------------

#include "config.h"

// --------------------------------------------------

#include "popcnt-lookup.cpp"
#include "popcnt-bit-parallel-scalar.cpp"

#include "sse_operators.cpp"
#include "popcnt-sse-bit-parallel.cpp"
#include "popcnt-sse-lookup.cpp"
#include "popcnt-cpu.cpp"
#if defined(HAVE_AVX2_INSTRUCTIONS)
#   include "popcnt-avx2-lookup.cpp"
#endif

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
    {false, "sse-lookup",              popcnt_SSE_lookup},
#if defined(HAVE_AVX2_INSTRUCTIONS)
    {false, "avx2-lookup",             popcnt_AVX2_lookup},
#endif
#if defined(HAVE_POPCNT_INSTRUCTION)
    {false, "cpu",                     popcnt_cpu_64bit},
#endif
    {false, nullptr, nullptr}
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

    verify("quasirandom", data, size);

    return EXIT_SUCCESS;
}


#if HAVE_ANSI_CONSOLE
void puts(const char* str, int ansi_color) {
    printf("\033[%dm%s\033[0m\n", ansi_color, str);
}
#else
void puts(const char* str, int) {
    puts(str);
}
#endif // HAVE_ANSI_CONSOLE


static const int RED   = 31;
static const int GREEN = 32;


void verify(const char* name, const std::uint8_t* data, const size_t size) {
 
    puts("");
    printf("test '%s':\n", name);

    const size_t reference = popcnt_lookup_8bit(data, size);

    for (auto& item: functions) {
        if (item.function == nullptr) {
            continue;
        }

        if (item.is_reference) {
            continue;
        }

        printf("%-30s: ", item.name);
        const size_t result = item.function(data, size);

        if (result == reference) {
            puts("OK", GREEN);
        } else {
            puts("ERROR", RED);
        }
    }
}
