std::uint64_t popcnt_AVX512VBMI_lookup(const uint8_t* data, const size_t n) {

    size_t i = 0;

    const __m512i lookup_lo = _mm512_setr_epi64(0x0302020102010100, 0x0403030203020201,
                                                0x0403030203020201, 0x0504040304030302,
                                                0x0403030203020201, 0x0504040304030302,
                                                0x0504040304030302, 0x0605050405040403);
    const __m512i lookup_hi = _mm512_setr_epi64(0x0403030203020201, 0x0504040304030302,
                                                0x0504040304030302, 0x0605050405040403,
                                                0x0504040304030302, 0x0605050405040403,
                                                0x0605050405040403, 0x0706060506050504);

    const __m512i lsb_mask = _mm512_set1_epi8(0x01);

    __m512i acc = _mm512_setzero_si512();

    while (i + 64 < n) {

        __m512i local = _mm512_setzero_si512(); 

        for (int k=0; k < 255/8 && i + 64 < n; k++, i += 64) {
            const __m512i vec = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(data + i));
            // get popcount from bits 6:0
            const __m512i lo  = _mm512_permutex2var_epi8(lookup_lo, vec, lookup_hi);

            // and move 7th bit onto position 0 -- i.e. (x & 0x80 ? 1 : 0)
            const __m512i hi  = _mm512_and_si512(_mm512_srli_epi32(vec, 7), lsb_mask);

            local = _mm512_add_epi8(local, lo);
            local = _mm512_add_epi8(local, hi);
        }

        acc = _mm512_add_epi64(acc, _mm512_sad_epu8(local, _mm512_setzero_si512()));
    }


    uint64_t result = custom::_mm512_hsum_epi64(acc);

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}

