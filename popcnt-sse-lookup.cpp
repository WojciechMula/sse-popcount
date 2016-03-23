std::uint64_t popcnt_SSE_lookup(const uint8_t* data, const size_t n) {

    size_t i = 0;

    const __m128i lookup = _mm_setr_epi8(
        /* 0 */ 0, /* 1 */ 1, /* 2 */ 1, /* 3 */ 2,
        /* 4 */ 1, /* 5 */ 2, /* 6 */ 2, /* 7 */ 3,
        /* 8 */ 1, /* 9 */ 2, /* a */ 2, /* b */ 3,
        /* c */ 2, /* d */ 3, /* e */ 3, /* f */ 4
    );

    const __m128i low_mask = _mm_set1_epi8(0x0f);

    __m128i acc = _mm_setzero_si128();

#define ITER { \
        const __m128i vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i)); \
        const __m128i lo  = vec & low_mask; \
        const __m128i hi  = (vec >> shift16(4)) & low_mask; \
        const __m128i popcnt1 = _mm_shuffle_epi8(lookup, lo); \
        const __m128i popcnt2 = _mm_shuffle_epi8(lookup, hi); \
        local = _mm_add_epi8(local, popcnt1); \
        local = _mm_add_epi8(local, popcnt2); \
        i += 16; \
    }

    while (i + 8*16 <= n) {
        __m128i local = _mm_setzero_si128();
        ITER ITER ITER ITER
        ITER ITER ITER ITER
        acc = _mm_add_epi64(acc, _mm_sad_epu8(local, _mm_setzero_si128()));
    }

    __m128i local = _mm_setzero_si128();

    while (i + 16 <= n) {
        ITER
    }

    acc = _mm_add_epi64(acc, _mm_sad_epu8(local, _mm_setzero_si128()));

#undef ITER

    size_t result = lower_qword(acc) + higher_qword(acc);

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}


std::uint64_t popcnt_SSE_lookup_original(const uint8_t* data, const size_t n) {

    size_t i = 0;

    const __m128i lookup = _mm_setr_epi8(
        /* 0 */ 0, /* 1 */ 1, /* 2 */ 1, /* 3 */ 2,
        /* 4 */ 1, /* 5 */ 2, /* 6 */ 2, /* 7 */ 3,
        /* 8 */ 1, /* 9 */ 2, /* a */ 2, /* b */ 3,
        /* c */ 2, /* d */ 3, /* e */ 3, /* f */ 4
    );

    const __m128i low_mask = _mm_set1_epi8(0x0f);

    __m128i acc = _mm_setzero_si128();

    while (i + 16 < n) {
        __m128i local = _mm_setzero_si128();

        for (int k=0; k < 255/8 && i + 16 < n; k++, i += 16) {
            const __m128i vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i));
            const __m128i lo  = vec & low_mask;
            const __m128i hi  = (vec >> shift16(4)) & low_mask;
            const __m128i popcnt1 = _mm_shuffle_epi8(lookup, lo);
            const __m128i popcnt2 = _mm_shuffle_epi8(lookup, hi);
            local = _mm_add_epi8(local, popcnt1);
            local = _mm_add_epi8(local, popcnt2);
        }
        acc = _mm_add_epi64(acc, _mm_sad_epu8(local, _mm_setzero_si128()));
    }

    size_t result = lower_qword(acc) + higher_qword(acc);

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}
