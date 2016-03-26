// define when cpu have popcnt instruction
#define HAVE_POPCNT_INSTRUCTION 1

// does your shell supports ansi color seqences?
#define HAVE_ANSI_CONSOLE 1


#if defined(HAVE_AVX512BW_INSTRUCTIONS)
    // AVX512 implies AVX2
#   undef  HAVE_AVX2_INSTRUCTIONS
#   define HAVE_AVX2_INSTRUCTIONS
#endif
