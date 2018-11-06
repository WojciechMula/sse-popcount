namespace custom {

    std::uint64_t _mm256_hsum_epi64(__m256i v) {
        return _mm256_extract_epi64(v, 0)
             + _mm256_extract_epi64(v, 1)
             + _mm256_extract_epi64(v, 2)
             + _mm256_extract_epi64(v, 3);
    }

    std::uint64_t _mm512_hsum_epi64(__m512i v) {
        const __m256i t0 = _mm512_extracti64x4_epi64(v, 0);
        const __m256i t1 = _mm512_extracti64x4_epi64(v, 1);

        return _mm256_hsum_epi64(t0)
             + _mm256_hsum_epi64(t1);
    }

} // namespace custom

std::uint64_t popcnt_AVX512BW_lookup_original(const uint8_t* data, const size_t n) {

    size_t i = 0;

    const __m512i lookup = _mm512_setr_epi64(
        0x0302020102010100llu, 0x0403030203020201llu,
        0x0302020102010100llu, 0x0403030203020201llu,
        0x0302020102010100llu, 0x0403030203020201llu,
        0x0302020102010100llu, 0x0403030203020201llu
    );

    const __m512i low_mask = _mm512_set1_epi8(0x0f);

    __m512i acc = _mm512_setzero_si512();

    while (i + 64 < n) {

        __m512i local = _mm512_setzero_si512(); 

        for (int k=0; k < 255/8 && i + 64 < n; k++, i += 64) {
            const __m512i vec = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(data + i));
            const __m512i lo  = _mm512_and_si512(vec, low_mask);
            const __m512i hi  = _mm512_and_si512(_mm512_srli_epi16(vec, 4), low_mask);

            const __m512i popcnt1 = _mm512_shuffle_epi8(lookup, lo);
            const __m512i popcnt2 = _mm512_shuffle_epi8(lookup, hi);

            local = _mm512_add_epi8(local, popcnt1);
            local = _mm512_add_epi8(local, popcnt2);
        }

        acc = _mm512_add_epi64(acc, _mm512_sad_epu8(local, _mm512_setzero_si512()));
    }


    uint64_t result = custom::_mm512_hsum_epi64(acc);

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}
