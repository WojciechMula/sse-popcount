#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>


using function_ptr      = std::uint64_t (*)(const uint8_t* data,  const size_t size);
using function_64_ptr   = std::uint64_t (*)(const uint64_t* data, int size);

class Function {
public:
    bool        is_trusted;
    std::string name;
    std::string help;
    function_ptr    function;
    function_64_ptr function_64;
};


class FunctionRegistry final {

private:
    using NameList      = std::vector<std::string>;
    using FunctionList  = std::vector<Function>;

    NameList     available;
    FunctionList functions;

    int widest_name;

public:
    FunctionRegistry() {
        build();

        widest_name = 0;
        for (const auto& name: available) {
            widest_name = std::max(widest_name, static_cast<int>(name.size()));
        }
    }

public:
    bool has(const std::string& name) const;
    const Function& get(const std::string& name) const;

    const NameList& get_available() const {
        return available;
    }

    const FunctionList& get_functions() const {
        return functions;
    }

public:
    int get_widest_name() const {
        return widest_name;
    }

private:
    void build();
    void add(const std::string& name, const std::string& help, function_ptr fn);
    void add(const std::string& name, const std::string& help, function_64_ptr fn);
    void add_trusted(const std::string& name, const std::string& help, function_ptr fn);
};


bool FunctionRegistry::has(const std::string& name) const {

    return std::find(available.begin(), available.end(), name) != available.end();
}


const Function& FunctionRegistry::get(const std::string& name) const {

    auto pred = [&name](const Function& item) {
        return item.name == name;
    };

    const auto& iter = std::find_if(functions.begin(), functions.end(), pred);
    if (iter == functions.end()) {
        fprintf(stderr, "function %s not found", name.c_str());
        assert(false && "function not found");
    }

    return *iter;
}


void FunctionRegistry::build() {
    // definition start

    add_trusted("lookup-8",
                "lookup in std::uint8_t[256] LUT",
                popcnt_lookup_8bit);

    add_trusted("lookup-64",
                "lookup in std::uint64_t[256] LUT",
                popcnt_lookup_64bit);

    add("bit-parallel",
        "naive bit parallel method",
        popcnt_parallel_64bit_naive);

    add("bit-parallel-optimized",
        "a bit better bit parallel",
        popcnt_parallel_64bit_optimized);

    add("harley-seal",
        "Harley-Seal popcount (4th iteration)",
        popcnt_harley_seal);

    add("sse-bit-parallel",
        "SSE implementation of bit-parallel-optimized",
        popcnt_SSE_bit_parallel);

    add("sse-lookup",
        "SSSE3 variant using pshufb instruction",
        popcnt_SSE_lookup);
#if defined(HAVE_AVX2_INSTRUCTIONS)
    add("avx2-lookup",
        "AVX2 variant using pshufb instruction",
        popcnt_AVX2_lookup);
#endif
#if defined(HAVE_POPCNT_INSTRUCTION)
    add("cpu",
        "CPU instruction popcnt (64-bit variant)",
        popcnt_cpu_64bit);

    add("builtin-popcnt",
        "builtin for popcnt",
        builtin_popcnt);

    add("builtin-popcnt32",
        "builtin for popcnt (32-bit variant)",
        builtin_popcnt32);

    add("builtin-popcnt-unrolled",
        "unrolled builtin-popcnt",
        builtin_popcnt_unrolled);

    add("builtin-popcnt-unrolled32",
        "unrolled builtin-popcnt32",
        builtin_popcnt_unrolled32);

    add("builtin-popcnt-unrolled-errata",
        "unrolled builtin-popcnt avoiding false-dependency",
        builtin_popcnt_unrolled_errata);

    add("builtin-popcnt-unrolled-errata-manual",
        "unrolled builtin-popcnt avoiding false-dependency (asembly code)",
        builtin_popcnt_unrolled_errata_manual);

    add("builtin-popcnt-movdq",
        "builtin-popcnt where data is loaded via SSE registers",
        builtin_popcnt_movdq);

    add("builtin-popcnt-movdq-unrolled",
        "builtin-popcnt-movdq unrolled",
        builtin_popcnt_movdq_unrolled);

    add("builtin-popcnt-movdq-unrolled_manual",
        "builtin-popcnt-movdq unrolled (assembly code)",
        builtin_popcnt_movdq_unrolled_manual);
#endif
    // definition end
}


void FunctionRegistry::add(const std::string& name, const std::string& help, function_ptr fn) {

    available.push_back(name);
    functions.push_back({false, name, help, fn, nullptr});
}


void FunctionRegistry::add(const std::string& name, const std::string& help, function_64_ptr fn) {

    available.push_back(name);
    functions.push_back({false, name, help, nullptr, fn});
}


void FunctionRegistry::add_trusted(const std::string& name, const std::string& help, function_ptr fn) {

    available.push_back(name);
    functions.push_back({true, name, help, fn, nullptr});
}


