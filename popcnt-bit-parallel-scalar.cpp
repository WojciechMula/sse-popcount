std::uint64_t popcnt_parallel_64bit_naive(const uint8_t* data, const size_t n) {

    uint64_t result = 0;

    size_t i = 0;

#define ITER { \
        const uint64_t t1 = *reinterpret_cast<const uint64_t*>(data + i); \
        const uint64_t t2 = (t1 & 0x5555555555555555llu) + ((t1 >>  1) & 0x5555555555555555llu); \
        const uint64_t t3 = (t2 & 0x3333333333333333llu) + ((t2 >>  2) & 0x3333333333333333llu); \
        const uint64_t t4 = (t3 & 0x0f0f0f0f0f0f0f0fllu) + ((t3 >>  4) & 0x0f0f0f0f0f0f0f0fllu); \
        const uint64_t t5 = (t4 & 0x00ff00ff00ff00ffllu) + ((t4 >>  8) & 0x00ff00ff00ff00ffllu); \
        const uint64_t t6 = (t5 & 0x0000ffff0000ffffllu) + ((t5 >> 16) & 0x0000ffff0000ffffllu); \
        const uint64_t t7 = (t6 & 0x00000000ffffffffllu) + ((t6 >> 32) & 0x00000000ffffffffllu); \
        result += t7; \
        i += 8; \
    }

    while (i + 4*8 <= n) {
        ITER ITER ITER ITER
    }

#undef ITER

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}


std::uint64_t popcnt_parallel_64bit_optimized(const uint8_t* data, const size_t n) {

    uint64_t result = 0;

    size_t i = 0;

    while (i + 4*8 <= n) {

        uint64_t partial = 0; // packed_byte

#define ITER { \
            const uint64_t t1 = *reinterpret_cast<const uint64_t*>(data + i); \
            const uint64_t t2 = (t1 & 0x5555555555555555llu) + ((t1 >>  1) & 0x5555555555555555llu); \
            const uint64_t t3 = (t2 & 0x3333333333333333llu) + ((t2 >>  2) & 0x3333333333333333llu); \
            const uint64_t t4 = (t3 & 0x0f0f0f0f0f0f0f0fllu) + ((t3 >>  4) & 0x0f0f0f0f0f0f0f0fllu); \
            partial += t4; \
            i += 8; \
        }

        ITER ITER ITER ITER

#undef ITER

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


// popcnt_mul from popcnt-harley-seal.cpp
std::uint64_t popcnt_parallel_64bit_mul(const uint8_t* data, const size_t n) {
    
    uint64_t result = 0;

    size_t i = 0;

    for (/**/; i < n; i += 8) {

        const uint64_t m1  = UINT64_C(0x5555555555555555);
        const uint64_t m2  = UINT64_C(0x3333333333333333);
        const uint64_t m4  = UINT64_C(0x0F0F0F0F0F0F0F0F);
        const uint64_t h01 = UINT64_C(0x0101010101010101);
        
        uint64_t x = *reinterpret_cast<const uint64_t*>(data + i);
        x -=            (x >> 1)  & m1;
        x = (x & m2) + ((x >> 2)  & m2);
        x = (x +        (x >> 4)) & m4;

        result += (x * h01) >> 56;
    }

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}


std::uint64_t popcnt_parallel_64bit_optimized2(const uint8_t* data, const size_t n) {

    uint64_t result = 0;

    size_t i = 0;

    while (i + 7*8 <= n) {
        const uint64_t in0 = *reinterpret_cast<const uint64_t*>(data + i + 8*0);
        const uint64_t in1 = *reinterpret_cast<const uint64_t*>(data + i + 8*1);
        const uint64_t in2 = *reinterpret_cast<const uint64_t*>(data + i + 8*2);
        const uint64_t in3 = *reinterpret_cast<const uint64_t*>(data + i + 8*3);
        const uint64_t in4 = *reinterpret_cast<const uint64_t*>(data + i + 8*4);
        const uint64_t in5 = *reinterpret_cast<const uint64_t*>(data + i + 8*5);
        const uint64_t in6 = *reinterpret_cast<const uint64_t*>(data + i + 8*6);
        i += 7*8;

        // 2-bit sums (we sum three 1-bit numbers: 3*1 = 2^2 - 1)
        const uint64_t a0 = in0 & 0x5555555555555555llu;
        const uint64_t a1 = (in0 >> 1) & 0x5555555555555555llu;
        const uint64_t a2 = in1 & 0x5555555555555555llu;
        const uint64_t A0 = (a0 + a1 + a2);

        const uint64_t a3 = (in1 >> 1) & 0x5555555555555555llu;
        const uint64_t a4 = in2 & 0x5555555555555555llu;
        const uint64_t a5 = (in2 >> 1) & 0x5555555555555555llu;
        const uint64_t A1 = (a3 + a4 + a5);

        const uint64_t a6 = in3 & 0x5555555555555555llu;
        const uint64_t a7 = (in3 >> 1) & 0x5555555555555555llu;
        const uint64_t a8 = in4 & 0x5555555555555555llu;
        const uint64_t A2 = (a6 + a7 + a8);

        const uint64_t a9 = (in4 >> 1) & 0x5555555555555555llu;
        const uint64_t a10 = in5 & 0x5555555555555555llu;
        const uint64_t a11 = (in5 >> 1) & 0x5555555555555555llu;
        const uint64_t A3 = (a9 + a10 + a11);

        const uint64_t a12 = in6 & 0x5555555555555555llu;
        const uint64_t a13 = (in6 >> 1) & 0x5555555555555555llu;
        const uint64_t A4 = (a12 + a13 + 0);

        // 4-bit sums (we sum five 2-bit numbers: 5*3 = 15 = 2^4 - 1
        const uint64_t b0 = A0 & 0x3333333333333333llu;
        const uint64_t b1 = (A0 >> 2) & 0x3333333333333333llu;
        const uint64_t b2 = A1 & 0x3333333333333333llu;
        const uint64_t b3 = (A1 >> 2) & 0x3333333333333333llu;
        const uint64_t b4 = A2 & 0x3333333333333333llu;
        const uint64_t b5 = (A2 >> 2) & 0x3333333333333333llu;
        const uint64_t b6 = A3 & 0x3333333333333333llu;
        const uint64_t b7 = (A3 >> 2) & 0x3333333333333333llu;
        const uint64_t b8 = A4 & 0x3333333333333333llu;
        const uint64_t b9 = (A4 >> 2) & 0x3333333333333333llu;

        const uint64_t B0 = (b0 + b1 + b2 + b3 + b4);
        const uint64_t B1 = (b5 + b6 + b7 + b8 + b9);

        // horiz sum of 4-bit values
        const uint64_t hsum = 0x0101010101010101llu;

        const uint64_t c0 = B0 & 0x0f0f0f0f0f0f0f0fllu;
        const uint64_t c1 = (B0 >> 4) & 0x0f0f0f0f0f0f0f0fllu;
        const uint64_t c2 = B1 & 0x0f0f0f0f0f0f0f0fllu;
        const uint64_t c3 = (B1 >> 4) & 0x0f0f0f0f0f0f0f0fllu;

        result += ((c0 + c1) * hsum) >> 56;
        result += ((c2 + c3) * hsum) >> 56;
    }

    for (/**/; i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}
