#include "config.h"

#include <immintrin.h>
#include <x86intrin.h>

#include <cstdint>
#include <cstdlib>
#include <cassert>

#include <memory>
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
#include "popcnt-builtin.cpp"

#if defined(HAVE_AVX2_INSTRUCTIONS)
#   include "popcnt-avx2-lookup.cpp"
#endif

#include "function_registry.cpp"

// --------------------------------------------------


class Error final {
    std::string message;

public:
    Error(const std::string& msg) : message(msg) {}

public:
    const char* c_str() const {
        return message.c_str();
    }
};

// --------------------------------------------------

class CommandLine final {

public:
    bool    print_help;
    bool    print_csv;
    size_t  size;
    size_t  iteration_count;
    std::string executable;
    std::set<std::string> functions;

public:
    CommandLine(int argc, char* argv[], const FunctionRegistry& names);
};

// --------------------------------------------------

CommandLine::CommandLine(int argc, char* argv[], const FunctionRegistry& names)
    : print_help(false)
    , print_csv(false)
    , size(0)
    , iteration_count(0) {

    int positional = 0;
    for (int i=1; i < argc; i++) {
        const std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            print_help = true;
            return;
        }

        if (arg == "--csv") {
            print_csv = true;
            continue;
        }

        // positional arguments
        if (positional == 0) {
            int tmp = std::atoi(arg.c_str());
            if (tmp <= 0) {
                throw Error("Size must be greater than 0.");
            }

            if (tmp % 32 != 0) {
                throw Error("Size must be divisible by 32.");
            }

            size = tmp;

        } else if (positional == 1) {

            int tmp = std::atoi(arg.c_str());
            if (tmp <= 0) {
                throw Error("Iteration count must be greater than 0.");
            }

            iteration_count = tmp;
        } else {
            if (names.has(arg)) {
                functions.insert(std::move(arg));
            } else {
                throw Error("'" + arg + "' is not valid function name");
            }
        }

        positional += 1;
    }

    if (positional < 2) {
        print_help = true;
    }
}


// --------------------------------------------------

class Application final {

    const CommandLine& cmd;
    const FunctionRegistry& names;
    std::unique_ptr<uint8_t[]> data;

    uint64_t count;
    double   time;

    struct Result {
        uint64_t count;
        double   time;
    };


public:
    Application(const CommandLine& cmdline, const FunctionRegistry& names);

    int run();

private:
    void print_help();
    void run_procedures();
    void run_procedure(const std::string& name);

    template <typename FN>
    Result run(const std::string& name, FN function, double reference);
};


Application::Application(const CommandLine& cmdline, const FunctionRegistry& names)
    : cmd(cmdline)
    , names(names) {}


int Application::run() {

    if (cmd.print_help) {
        print_help();
    } else {
        run_procedures();
    }

    return 0;
}

void Application::run_procedures() {

    data.reset(new uint8_t[cmd.size]);

    for (size_t i=0; i < cmd.size; i++) {
        data[i] = i;
    }

    count = 0;
    time  = 0;

    if (!cmd.functions.empty()) {
        for (const auto& name: cmd.functions) {
            run_procedure(name);
        }
    } else {
        for (const auto& name: names.get_available()) {
            run_procedure(name);
        }
    }
}

void Application::run_procedure(const std::string& name) {

#define RUN(function_name, function) \
    if (name == function_name) { \
        auto result = run(name, function, time); \
        count += result.count; \
        if (time == 0.0) { \
            time = result.time; \
        } \
    }

    RUN("lookup-8",                 popcnt_lookup_8bit)
    RUN("lookup-64",                popcnt_lookup_64bit);
    RUN("bit-parallel",             popcnt_parallel_64bit_naive);
    RUN("bit-parallel-optimized",   popcnt_parallel_64bit_optimized);
    RUN("harley-seal",              popcnt_harley_seal);
    RUN("sse-bit-parallel",         popcnt_SSE_bit_parallel);
    RUN("sse-lookup",               popcnt_SSE_lookup);

#if defined(HAVE_AVX2_INSTRUCTIONS)
    RUN("avx2-lookup", popcnt_AVX2_lookup);
#endif

#if defined(HAVE_POPCNT_INSTRUCTION)
    RUN("cpu", popcnt_cpu_64bit);
#endif

#define RUN_BUILTIN(function_name, function) \
    { \
        auto wrapper = [](const uint8_t* data, size_t size) { \
            return function(reinterpret_cast<const uint64_t*>(data), size/8); \
        }; \
        RUN(function_name, wrapper); \
    }

    RUN_BUILTIN("builtin-popcnt",                           builtin_popcnt);
    RUN_BUILTIN("builtin-popcnt32",                         builtin_popcnt32);
    RUN_BUILTIN("builtin-popcnt-unrolled",                  builtin_popcnt_unrolled);
    RUN_BUILTIN("builtin-popcnt-unrolled32",                builtin_popcnt_unrolled32);
    RUN_BUILTIN("builtin-popcnt-unrolled-errata",           builtin_popcnt_unrolled_errata);
    RUN_BUILTIN("builtin-popcnt-unrolled-errata-manual",    builtin_popcnt_unrolled_errata_manual);
    RUN_BUILTIN("builtin-popcnt-movdq",                     builtin_popcnt_movdq);
    RUN_BUILTIN("builtin-popcnt-movdq-unrolled",            builtin_popcnt_movdq_unrolled);
    RUN_BUILTIN("builtin-popcnt-movdq-unrolled_manual",     builtin_popcnt_movdq_unrolled_manual);
}


template <typename FN>
Application::Result Application::run(const std::string& name, FN function, double reference) {

    Result result;

    if (cmd.print_csv) {
        printf("%s, %lu, %lu, ", name.c_str(), cmd.size, cmd.iteration_count);
        fflush(stdout);
    } else {
        const auto& dsc = names.get(name);
        printf("%-30s... ", dsc.name.c_str());
        fflush(stdout);
    }

    size_t n = 0;
    size_t k = cmd.iteration_count;

    const auto t1 = std::chrono::high_resolution_clock::now();
    while (k-- > 0) {
        n += function(data.get(), cmd.size);
    }

    const auto t2 = std::chrono::high_resolution_clock::now();

    const std::chrono::duration<double> td = t2-t1;

    if (cmd.print_csv) {
        printf("%0.6f\n", td.count());
    } else {
        //printf("reference result = %lu, time = %0.6f s", n, td.count());
        printf("time = %0.6f s", td.count());

        if (reference > 0.0) {
            const auto speedup = reference/td.count();

            printf(" (speedup: %3.2f)", speedup);
        }

        putchar('\n');
    }

    result.count = n; // to prevent compiler from optimizing out the loop
    result.time  = td.count();

    return result;
}

void Application::print_help() {
    std::printf("usage: %s [--csv] buffer_size iteration_count [function(s)]\n", cmd.executable.c_str());
    std::puts("");
    std::puts("--csv               - print results in CVS format:");
    std::puts("                      function name, buffer_size, iteration_count, time");
    std::puts("");
    std::puts("1. buffer_size      - size of buffer in bytes");
    std::puts("2. iteration_count  - as the name states");

    std::puts("3. one or more functions (if not given all will run):");

    const int w = names.get_widest_name();
    for (const auto& item: names.get_functions()) {

        std::printf("  * %*s - %s\n", -w, item.name.c_str(), item.help.c_str());
    }
}


int main(int argc, char* argv[]) {

    try {
        FunctionRegistry names;
        CommandLine cmd(argc, argv, names);
        Application app(cmd, names);

        return app.run();
    } catch (Error& e) {
        puts(e.c_str());

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

