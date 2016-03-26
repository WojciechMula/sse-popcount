std::uint64_t popcnt_SSE_and_cpu(const uint8_t* data, const size_t n) {

#define ITER { \
        const __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i)); \
        result += _popcnt64(lower_qword(v)); \
        result += _popcnt64(higher_qword(v)); \
        i += 16; \
    }

    size_t i = 0;
    uint64_t result = 0;

    while (i + 4*16 <= n) {
        ITER ITER ITER ITER
    }

#undef ITER

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}

