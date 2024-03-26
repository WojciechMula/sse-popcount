std::uint64_t popcnt_AVX512_vpopcnt(const uint8_t* data, const size_t size) {
    
    const size_t chunks = size / 64;

    uint8_t* ptr = const_cast<uint8_t*>(data);
    const uint8_t* end = ptr + size;

    // count using AVX512 registers
    __m512i accumulator = _mm512_setzero_si512();
    for (size_t i=0; i < chunks; i++, ptr += 64) {
        
        // Note: a short chain of dependencies, likely unrolling will be needed.
        const __m512i v = _mm512_loadu_si512((const __m512i*)ptr);
        const __m512i p = _mm512_popcnt_epi64(v);

        accumulator = _mm512_add_epi64(accumulator, p);
    }

    // horizontal sum of a register
    uint64_t tmp[8] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)tmp, accumulator);

    uint64_t total = 0;
    for (size_t i=0; i < 8; i++) {
        total += tmp[i];
    }

    // popcount the tail
    while (ptr + 8 < end) {
        total += _mm_popcnt_u64(*reinterpret_cast<const uint64_t*>(ptr));
        ptr += 8;
    }

    while (ptr < end) {
        total += lookup8bit[*ptr++];
    }

    return total;
}

