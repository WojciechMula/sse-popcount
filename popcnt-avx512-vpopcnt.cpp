std::uint64_t popcnt_AVX512_vpopcnt(const uint8_t *data, const size_t size)
{

    const size_t chunks = size / 64;
    uint8_t *ptr = const_cast<uint8_t *>(data);
    const uint8_t *end = ptr + size;

    // count using AVX512 registers
    __m512i accumulator = _mm512_setzero_si512();
    for (size_t i = 0; i < chunks; i++, ptr += 64)
    {
        const __m512i v = _mm512_loadu_si512((const __m512i *)ptr);
        const __m512i p = _mm512_popcnt_epi64(v);
        accumulator = _mm512_add_epi64(accumulator, p);
    }

    // use masked instrucitons for the tail
    if (ptr < end)
    {
        __mmask8 mask = (__mmask8)_bzhi_u32(0xFFFFFFFF, end - ptr);
        const __m512i v = _mm512_maskz_loadu_epi64(mask, ptr);
        const __m512i p = _mm512_popcnt_epi64(v);
        accumulator = _mm512_add_epi64(accumulator, p);
    }

    return _mm512_reduce_add_epi64(accumulator);
}
