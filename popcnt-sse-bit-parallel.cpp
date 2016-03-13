std::uint64_t popcnt_SSE_bit_parallel(const uint8_t* data, const size_t n) {

    size_t i = 0;

    const __m128i pattern_2bit  = _mm_set1_epi8(0x55);
    const __m128i pattern_4bit  = _mm_set1_epi8(0x33);
    const __m128i pattern_16bit = _mm_set1_epi8(0x0f);

    __m128i acc = _mm_setzero_si128();

    while (i + 4*16 < n) {

        __m128i partial = _mm_setzero_si128(); 

#define ITER { \
            const __m128i t1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i)); \
            const __m128i t2 = (t1 & pattern_2bit)  + ((t1 >> shift16(1)) & pattern_2bit); \
            const __m128i t3 = (t2 & pattern_4bit)  + ((t2 >> shift16(2)) & pattern_4bit); \
            const __m128i t4 = (t3 & pattern_16bit) + ((t3 >> shift16(4)) & pattern_16bit); \
            partial = _mm_add_epi8(partial, t4); \
            i += 16; \
        }

        ITER ITER ITER ITER

#undef ITER

        acc = _mm_add_epi64(acc, _mm_sad_epu8(partial, _mm_setzero_si128()));
    }

    size_t result = lower_qword(acc) + higher_qword(acc);

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}
