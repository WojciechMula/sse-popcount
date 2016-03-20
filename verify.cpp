#include "config.h"

#include <immintrin.h>
#include <x86intrin.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

// --------------------------------------------------

#include "config.h"

// --------------------------------------------------

#include "popcnt-lookup.cpp"
#include "popcnt-bit-parallel-scalar.cpp"
#include "popcnt-harley-seal.cpp"

#include "sse_operators.cpp"
#include "popcnt-sse-bit-parallel.cpp"
#include "popcnt-sse-lookup.cpp"
#include "popcnt-cpu.cpp"
#include "popcnt-builtin.cpp"
#if defined(HAVE_AVX2_INSTRUCTIONS)
#   include "popcnt-avx2-lookup.cpp"
#endif

// --------------------------------------------------

bool verify(const char* name, const std::uint8_t* data, const size_t size);
size_t widest_name();

struct function_t {
    const bool     is_reference;
    const char*    name;
    std::uint64_t (*function)(const uint8_t* data, const size_t size);
    std::uint64_t (*function_64)(const uint64_t* data, int size);
};

function_t functions[] = {
    {true,  "lookup-8",                popcnt_lookup_8bit, nullptr},
    {true,  "lookup-64",               popcnt_lookup_64bit, nullptr},
    {false, "bit-parallel",            popcnt_parallel_64bit_naive, nullptr},
    {false, "bit-parallel-optimized",  popcnt_parallel_64bit_optimized, nullptr},
    {false, "harley-seal",             popcnt_harley_seal, nullptr},
    {false, "sse-bit-parallel",        popcnt_SSE_bit_parallel, nullptr},
    {false, "sse-lookup",              popcnt_SSE_lookup, nullptr},
#if defined(HAVE_AVX2_INSTRUCTIONS)
    {false, "avx2-lookup",             popcnt_AVX2_lookup, nullptr},
#endif
#if defined(HAVE_POPCNT_INSTRUCTION)
    {false, "cpu",                     popcnt_cpu_64bit, nullptr},
#endif
    // from popcnt-builtin.cpp
    {false, "builtin-popcnt",                           nullptr, builtin_popcnt},
    {false, "builtin-popcnt32",                         nullptr, builtin_popcnt32},
    {false, "builtin-popcnt-unrolled",                  nullptr, builtin_popcnt_unrolled},
    {false, "builtin-popcnt-unrolled32",                nullptr, builtin_popcnt_unrolled32},
    {false, "builtin-popcnt-unrolled-errata",           nullptr, builtin_popcnt_unrolled_errata},
    {false, "builtin-popcnt-unrolled-errata-manual",    nullptr, builtin_popcnt_unrolled_errata_manual},
    {false, "builtin-popcnt-movdq",                     nullptr, builtin_popcnt_movdq},
    {false, "builtin-popcnt-movdq-unrolled",            nullptr, builtin_popcnt_movdq_unrolled},
    {false, "builtin-popcnt-movdq-unrolled_manual",     nullptr, builtin_popcnt_movdq_unrolled_manual},

    // sentinel
    {false, nullptr, nullptr, nullptr}
};


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


int main() {

    static const size_t size = 1024;
    std::uint8_t data[size];
    bool all_ok = true;

    // 1. all zeros
    for (size_t i=0; i < size; i++) {
        data[i] = 0;
    }

    all_ok &= verify("all zeros", data, size);


    // 2. all ones
    for (size_t i=0; i < size; i++) {
        data[i] = 1;
    }

    all_ok &= verify("all ones", data, size);

    // 3. ascending
    for (size_t i=0; i < size; i++) {
        data[i] = i;
    }

    all_ok &= verify("ascending", data, size);

    // 4. quasirandom
    for (size_t i=0; i < size; i++) {
        data[i] = i*33 + 12345;
    }

    all_ok &= verify("quasirandom", data, size);

    if (all_ok) {
        return EXIT_SUCCESS;
    } else {
        puts("There are errors", RED);
        return EXIT_FAILURE;
    }
}


bool verify(const char* name, const std::uint8_t* data, const size_t size) {

    const int w = widest_name();

    puts("");
    printf("test '%s' :\n", name);

    const size_t reference = popcnt_lookup_8bit(data, size);

    bool all_ok = true;

    for (auto& item: functions) {
        if (item.name == nullptr) {
            continue;
        }

        if (item.is_reference) {
            continue;
        }

        printf("%*s: ", -w, item.name);
        size_t result;
        if (item.function)
            result = item.function(data, size);
        else
            result = item.function_64(reinterpret_cast<const uint64_t*>(data), size/8);

        if (result == reference) {
            puts("OK", GREEN);
        } else {
            puts("ERROR", RED);
            all_ok = false;
        }
    }

    return all_ok;
}


size_t widest_name() {

    size_t width = 0;

    for (auto& item: functions) {
        if (item.name != nullptr) {
            width = std::max(width, strlen(item.name));
        }
    }

    return width;
}
