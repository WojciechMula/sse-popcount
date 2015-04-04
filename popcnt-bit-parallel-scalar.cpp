std::uint64_t popcnt_parallel_64bit_naive(const uint8_t* data, const size_t n) {

    uint64_t result = 0;

    for (size_t i=0; i < n; i += 8) {
        
        const uint64_t t1 = *reinterpret_cast<const uint64_t*>(data + i);

        const uint64_t t2 = (t1 & 0x5555555555555555llu) + ((t1 >>  1) & 0x5555555555555555llu);
        const uint64_t t3 = (t2 & 0x3333333333333333llu) + ((t2 >>  2) & 0x3333333333333333llu);
        const uint64_t t4 = (t3 & 0x0f0f0f0f0f0f0f0fllu) + ((t3 >>  4) & 0x0f0f0f0f0f0f0f0fllu);
        const uint64_t t5 = (t4 & 0x00ff00ff00ff00ffllu) + ((t4 >>  8) & 0x00ff00ff00ff00ffllu);
        const uint64_t t6 = (t5 & 0x0000ffff0000ffffllu) + ((t5 >> 16) & 0x0000ffff0000ffffllu);
        const uint64_t t7 = (t6 & 0x00000000ffffffffllu) + ((t6 >> 32) & 0x00000000ffffffffllu);

        result += t7;
    }

    if (n % 8 != 0) {

        for (size_t i = 8*(n/8); i < n; i++) {
            result += lookup8bit[data[i]];
        }
    }

    return result;
}


std::uint64_t popcnt_parallel_64bit_optimized(const uint8_t* data, const size_t n) {

    uint64_t result = 0;

    size_t i = 0;

    while (i + 8 < n) {

        uint64_t partial = 0; // packed_byte

        for (int k = 0; i + 8 < n && k < 255/8; k++, i += 8) {
            const uint64_t t1 = *reinterpret_cast<const uint64_t*>(data + i);

            const uint64_t t2 = (t1 & 0x5555555555555555llu) + ((t1 >>  1) & 0x5555555555555555llu);
            const uint64_t t3 = (t2 & 0x3333333333333333llu) + ((t2 >>  2) & 0x3333333333333333llu);
            const uint64_t t4 = (t3 & 0x0f0f0f0f0f0f0f0fllu) + ((t3 >>  4) & 0x0f0f0f0f0f0f0f0fllu);

            partial += t4;
        }

        const uint64_t t5 = (partial & 0x00ff00ff00ff00ffllu) + ((partial >>  8) & 0x00ff00ff00ff00ffllu);
        const uint64_t t6 = (t5 & 0x0000ffff0000ffffllu) + ((t5 >> 16) & 0x0000ffff0000ffffllu);
        const uint64_t t7 = (t6 & 0x00000000ffffffffllu) + ((t6 >> 32) & 0x00000000ffffffffllu);

        result += t7;
    }

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}
