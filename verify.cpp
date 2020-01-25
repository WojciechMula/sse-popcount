#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

// --------------------------------------------------

#include "config.h"
#include "popcnt-all.cpp"
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
    void run_const_val(const char* name, uint8_t val);
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
    : names(names)
    , failed(false) {}


bool Application::run() {

    run_const_val("all zeros", 0x00);
    run_const_val("all ones", 0xff);
    run_const_val("single bit (0x01)", 0x01);
    run_const_val("single bit (0x02)", 0x02);
    run_const_val("single bit (0x04)", 0x04);
    run_const_val("single bit (0x08)", 0x08);
    run_const_val("single bit (0x10)", 0x10);
    run_const_val("single bit (0x20)", 0x20);
    run_const_val("single bit (0x40)", 0x40);
    run_const_val("single bit (0x80)", 0x80);
    run_ascending();
    run_quasirandom();

    return !failed;
}


void Application::run_const_val(const char* name, uint8_t val) {

    for (size_t i=0; i < size; i++) {
        data[i] = val;
    }

    verify(name);
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
    printf("test '%s':\n", name);

    const size_t reference = popcnt_lookup_8bit(data, size);

    for (auto& item: names.get_functions()) {
        if (item.is_trusted) {
            continue;
        }

        printf("%*s : ", -w, item.name.c_str());
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
            printf("result = %lu, reference = %lu\n", result, reference);
            failed = true;
        }
    }
}


