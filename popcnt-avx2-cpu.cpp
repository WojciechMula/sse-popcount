std::uint64_t popcnt_AVX2_and_cpu(const uint8_t* data, const size_t n) {

#define ITER { \
        const __m256i v = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data + i)); \
        result += _popcnt64(_mm256_extract_epi64(v, 0)); \
        result += _popcnt64(_mm256_extract_epi64(v, 1)); \
        result += _popcnt64(_mm256_extract_epi64(v, 2)); \
        result += _popcnt64(_mm256_extract_epi64(v, 3)); \
        i += 32; \
    }

    size_t i = 0;
    uint64_t result = 0;

    while (i + 4*32 <= n) {
        ITER ITER ITER ITER
    }

#undef ITER

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}

