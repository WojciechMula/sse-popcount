// does your shell supports ansi color seqences?
#define HAVE_ANSI_CONSOLE 1

#if defined(HAVE_AVX512BW_INSTRUCTIONS)
    // AVX512 implies AVX2 & AVX
#   if !defined(HAVE_AVX_INSTRUCTIONS)
#       define HAVE_AVX_INSTRUCTIONS
#   endif
#   if !defined(HAVE_AVX2_INSTRUCTIONS)
#       define HAVE_AVX2_INSTRUCTIONS
#   endif
#endif

#if defined(HAVE_AVX_INSTRUCTIONS) || defined(HAVE_AVX2_INSTRUCTIONS)
#   if !defined(HAVE_SSE_INSTRUCTIONS)
#       define HAVE_SSE_INSTRUCTIONS
#   endif
#endif

#if !defined(HAVE_NEON_INSTRUCTIONS)
#   define HAVE_POPCNT_INSTRUCTION 1
#endif

#if defined(HAVE_NEON_INSTRUCTIONS)
#   include <arm_neon.h>
#else
#   include <immintrin.h>
#   include <x86intrin.h>
#endif

#define FORCE_INLINE inline __attribute__((always_inline))

