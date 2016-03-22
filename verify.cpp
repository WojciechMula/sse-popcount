#include "config.h"

#include <immintrin.h>
#include <x86intrin.h>

#include <cstdint>
#include <cstdlib>
#include <cstdio>
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

#include "function_registry.cpp"

// --------------------------------------------------


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


class Application final {

    const FunctionRegistry& names;

    static const size_t size = 1024;
    std::uint8_t data[size] __attribute__((aligned(64)));
    bool failed;

public:
    Application(const FunctionRegistry& names);

    bool run();

private:
    void run_all_zeros();
    void run_all_ones();
    void run_ascending();
    void run_quasirandom();
    void verify(const char* name);
};


int main() {

    FunctionRegistry names;
    Application app(names);

    const bool ok = app.run();

    if (ok) {
        return EXIT_SUCCESS;
    } else {
        puts("There are errors", RED);
        return EXIT_FAILURE;
    }
}


Application::Application(const FunctionRegistry& names)
    : names(names) {}


bool Application::run() {

    run_all_zeros();
    run_all_ones();
    run_ascending();
    run_quasirandom();

    return !failed;
}


void Application::run_all_zeros() {

    for (size_t i=0; i < size; i++) {
        data[i] = 0;
    }

    verify("all zeros");
}


void Application::run_all_ones() {

    for (size_t i=0; i < size; i++) {
        data[i] = 1;
    }

    verify("all ones");
}


void Application::run_ascending() {

    for (size_t i=0; i < size; i++) {
        data[i] = i;
    }

    verify("ascending");
}


void Application::run_quasirandom() {

    for (size_t i=0; i < size; i++) {
        data[i] = i*33 + 12345;
    }

    verify("quasirandom");
}



void Application::verify(const char* name) {

    const int w = names.get_widest_name();

    puts("");
    printf("test '%s' :\n", name);

    const size_t reference = popcnt_lookup_8bit(data, size);

    for (auto& item: names.get_functions()) {
        if (item.is_trusted) {
            continue;
        }

        printf("%*s: ", -w, item.name.c_str());
        size_t result;
        if (item.function) {
            result = item.function(data, size);
        } else {
            result = item.function_64(reinterpret_cast<const uint64_t*>(data), size/8);
        }

        if (result == reference) {
            puts("OK", GREEN);
        } else {
            puts("ERROR", RED);
            failed = true;
        }
    }
}


