// includes all available implementations

#include "popcnt-lookup.cpp"
#include "popcnt-bit-parallel-scalar.cpp"
#include "popcnt-bit-parallel-scalar32.cpp"
#include "popcnt-harley-seal.cpp"
#include "popcnt-builtin.cpp"

#if defined(HAVE_SSE_INSTRUCTIONS)
#   include "sse_operators.cpp"
#   include "popcnt-sse-bit-parallel.cpp"
#   include "popcnt-sse-bit-parallel-better.cpp"
#   include "popcnt-sse-lookup.cpp"
#   include "popcnt-sse-harley-seal.cpp"
#   include "popcnt-sse-cpu.cpp"
#endif

#if defined(HAVE_POPCNT_INSTRUCTION)
#   include "popcnt-cpu.cpp"
#endif

#if defined(HAVE_AVX2_INSTRUCTIONS)
#   include "popcnt-avx2-lookup.cpp"
#   include "popcnt-avx2-harley-seal.cpp"
#   include "popcnt-avx2-cpu.cpp"
#endif

#if defined(HAVE_AVX512BW_INSTRUCTIONS)
#   include "popcnt-avx512-harley-seal.cpp"
#endif
