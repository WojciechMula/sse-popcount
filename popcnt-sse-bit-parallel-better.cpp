/*
    https://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation

    the three first steps from popcount_2:

        int popcount_2(uint64_t x) {
            x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
            x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
            x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
            x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
            x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
            x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
            return x & 0x7f;
        }
*/

std::uint64_t popcnt_SSE_bit_parallel_better(const uint8_t* data, const size_t n) {

    size_t i = 0;

    const __m128i m1  = _mm_set1_epi8(0x55);
    const __m128i m2  = _mm_set1_epi8(0x33);
    const __m128i m4 = _mm_set1_epi8(0x0f);

    __m128i acc = _mm_setzero_si128();

    while (i + 4*16 < n) {

        __m128i partial = _mm_setzero_si128();

#define ITER { \
            const __m128i t1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i)); \
            const __m128i t2 = _mm_sub_epi8(t1, _mm_srli_epi16(t1, 1) & m1); \
            const __m128i t3 = _mm_add_epi8(t2 & m2, _mm_srli_epi16(t2, 2) & m2); \
            const __m128i t4 = _mm_add_epi8(t3, _mm_srli_epi16(t3, 4)) & m4; \
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

