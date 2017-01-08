std::uint64_t popcnt_parallel_32bit_naive(const uint8_t* data, const size_t n) {

    uint32_t result = 0;

    size_t i = 0;

#define ITER { \
        const uint32_t t1 = *reinterpret_cast<const uint32_t*>(data + i); \
        const uint32_t t2 = (t1 & 0x55555555llu) + ((t1 >>  1) & 0x55555555llu); \
        const uint32_t t3 = (t2 & 0x33333333llu) + ((t2 >>  2) & 0x33333333llu); \
        const uint32_t t4 = (t3 & 0x0f0f0f0fllu) + ((t3 >>  4) & 0x0f0f0f0fllu); \
        const uint32_t t5 = (t4 & 0x00ff00ffllu) + ((t4 >>  8) & 0x00ff00ffllu); \
        const uint32_t t6 = (t5 & 0x0000ffffllu) + ((t5 >> 16) & 0x0000ffffllu); \
        result += t6; \
        i += 4; \
    }

    while (i + 4*4 <= n) {
        ITER ITER ITER ITER
    }

#undef ITER

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}


std::uint64_t popcnt_parallel_32bit_optimized(const uint8_t* data, const size_t n) {

    uint32_t result = 0;

    size_t i = 0;

    while (i + 4*4 <= n) {

        uint32_t partial = 0; // packed_byte

#define ITER { \
            const uint32_t t1 = *reinterpret_cast<const uint32_t*>(data + i); \
            const uint32_t t2 = (t1 & 0x55555555llu) + ((t1 >>  1) & 0x55555555llu); \
            const uint32_t t3 = (t2 & 0x33333333llu) + ((t2 >>  2) & 0x33333333llu); \
            const uint32_t t4 = (t3 & 0x0f0f0f0fllu) + ((t3 >>  4) & 0x0f0f0f0fllu); \
            partial += t4; \
            i += 4; \
        }

        ITER ITER ITER ITER

#undef ITER

        const uint32_t t5 = (partial & 0x00ff00ffllu) + ((partial >>  8) & 0x00ff00ffllu);
        const uint32_t t6 = (t5 & 0x0000ffffllu) + ((t5 >> 16) & 0x0000ffffllu);

        result += t6;
    }

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}


